#include "custom.h"

#include <filesystem>
#include "../../external/imgui/imgui_internal.h"
#include "../config/config.h"

namespace llpp::gui
{
    namespace
    {
        int accent_color[4] = {140, 131, 214, 255};

        ImColor get_accent_color(float a = 1.f)
        {
            return {
                accent_color[0] / 255.f, accent_color[1] / 255.f, accent_color[2] / 255.f,
                a
            };
        }
    }


    bool tab_button(const char* icon, const char* label, const bool selected,
                    const float rounding, const ImDrawFlags flags)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImGuiID id = window->GetID(label);

        const auto font = ImGui::GetIO().Fonts->Fonts[1];
        // get the size of the buttons text and icon
        const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
        const ImVec2 icon_size = font->CalcTextSizeA(font->FontSize - 2, FLT_MAX, 0,
                                                     icon);

        const ImVec2 start = window->DC.CursorPos;
        // Draw from the top left to the full width and 45 down
        const ImRect rect(start, start + ImVec2(ImGui::GetWindowWidth(), 45));
        ImGui::ItemAdd(rect, id);
        ImGui::ItemSize(rect, ImGui::GetStyle().FramePadding.y);

        // add button behavior instead of making it a button and check if its pressed
        bool hovered, held;
        const bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held, NULL);

        static std::unordered_map<ImGuiID, float> saved;

        if (!saved.contains(id)) { saved[id] = 0.f; }
        float& anim = saved[id];

        anim = ImLerp(anim, (selected ? 1.f : 0.f), 0.035f);

        // Create the background colour of the 'button'
        const auto bg_colour = ImColor(1.f, 1.f, 1.f, 0.025f * anim);
        window->DrawList->AddRectFilled(rect.Min, rect.Max, bg_colour, rounding, flags);

        const ImVec2 icon_start(rect.Min.x + 25 - (icon_size.x / 2),
                                rect.GetCenter().y - icon_size.y / 2);
        const auto icon_col = ImGui::GetColorU32(ImGuiCol_Text,
                                                 (anim > 0.3f ? anim : 0.3f));
        window->DrawList->AddText(font, font->FontSize - 2, icon_start, icon_col, icon);

        const auto text_start = ImVec2(rect.Min.x + 50,
                                       rect.GetCenter().y - (label_size.y / 2) + 0.5f);
        const auto text_col = ImGui::GetColorU32(ImGuiCol_Text,
                                                 (anim > 0.3f ? anim : 0.3f));
        window->DrawList->AddText(text_start, text_col, label);

        return pressed;
    }

    bool sub_tab_button(const char* label, const bool selected)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImGuiID id = window->GetID(label);

        const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
        const ImVec2 pos = window->DC.CursorPos;

        const ImRect rect(pos, pos + label_size);
        ImGui::ItemAdd(rect, id);
        ImGui::ItemSize(ImVec4(rect.Min.x, rect.Min.y, rect.Max.x + 15, rect.Max.y), 0);

        bool hovered, held;
        ImRect pressable_rect(rect.Min.x - 7, rect.Min.y - 7, rect.Max.x + 7,
                              rect.Max.y + 7);
        const bool pressed = ImGui::ButtonBehavior(pressable_rect, id, &hovered, &held,
                                                   NULL);

        static std::unordered_map<ImGuiID, float> saved;
        if (!saved.contains(id)) { saved[id] = 0.f; }
        float& anim = saved[id];

        anim = ImLerp(anim, (selected ? 1.f : 0.f), 0.035f);

        window->DrawList->AddText(rect.Min,
                                  ImGui::GetColorU32(ImGuiCol_Text, max(anim, 0.4f)),
                                  label);

        window->DrawList->AddLine(ImVec2(rect.Min.x - 2, rect.Max.y + 10),
                                  ImVec2(rect.Max.x + 2, rect.Max.y + 10),
                                  get_accent_color(anim * ImGui::GetStyle().Alpha), 2);
        return pressed;
    }


    void render_main_tab_area(const std::string& name, const ImVec2 size,
                              const std::function<void()>& render_content)
    {
        auto& [is_hovered, expand, width] = maintabs_data;

        // adjust the with depending on whether we are expanding or collapsing
        width = ImLerp(width, expand ? size.x + 60 : size.x, 0.08f);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::BeginChild(std::string(name).append(".child").c_str(),
                          ImVec2(width, size.y));

        is_hovered = ImGui::IsMouseHoveringRect(ImGui::GetCurrentWindow()->Pos,
                                                ImGui::GetCurrentWindow()->Pos + ImVec2(
                                                    width, size.y));

        ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetCurrentWindow()->Pos,
                                                  ImGui::GetCurrentWindow()->Pos +
                                                  ImGui::GetCurrentWindow()->Size,
                                                  ImColor(28, 30, 36),
                                                  ImGui::GetStyle().WindowRounding,
                                                  ImDrawFlags_RoundCornersTopLeft);

        if (tab_button(
            ICON_FA_MAGNIFYING_GLASS_ARROW_RIGHT, "Expand", expand,
            ImGui::GetStyle().WindowRounding, ImDrawFlags_RoundCornersTopLeft)) {
            expand = !expand;
        }

        render_content();
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    inline char dir_buffer[256];
    inline ImVec4 path_color = ImVec4(1.f, 0.f, 0.f, 1.f);
    inline bool bools[50]{};
    inline int ints[50]{};
    inline float color[4] = {1.f, 1.f, 1.f, 1.f};
    inline std::vector<const char*> items = {};

    void draw_general_ark_tabs()
    {
        begin_child("Game Settings", ImVec2(300, ImGui::GetWindowHeight()));
        {
            // CONFIG FOR THE ARK ROOT DIRECTORY
            ImGui::SetCursorPos(ImVec2(10, 14));
            ImGui::TextColored(path_color, "Directory:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.5f);
            ImGui::SetCursorPosY(10);
            ImGui::InputText("##", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip("The path to the root game directory.");
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.3f);
            ImGui::SetCursorPosY(10);
            if (ImGui::Button("Get")) {
                std::string selected;
                openFolder(selected);
                if (!selected.empty()) { strcpy_s(dir_buffer, selected.c_str()); }
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip("Select the directory.");
            }
            const bool valid = exists(std::filesystem::path(dir_buffer));
            path_color = valid ? ImVec4(0.f, 1.f, 0.f, 1.f) : ImVec4(1.f, 0.f, 0.f, 1.f);
        }
        end_child();

        ImGui::SameLine();
        begin_child("f", ImVec2(280, ImGui::GetWindowHeight()));
        end_child();
    }

    void draw_discord_bot_config()
    {
        begin_child("Bot settings", ImVec2(300, ImGui::GetWindowHeight()));
        {
            ImGui::SetCursorPos({10, 14});
            ImGui::Text("Discord Bot token:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 11});
            ImGui::InputText("##bot_token", config::discord::token.get().data(), 256,
                             ImGuiInputTextFlags_Password |
                             ImGuiInputTextFlags_CallbackEdit,
                             [](ImGuiInputTextCallbackData* data) -> int {
                                 config::discord::token.set(data->Buf);
                                 return 0;
                             });

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "The token for your discord bot, you must create it yourself.\nDO NOT SHARE THIS WITH ANYONE.");
            }

            ImGui::SetCursorPos({10, 45});
            ImGui::Text("Command channels:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 42});
            ImGui::InputText("##command_channels", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "The ids of the channels authorized to issue bot commands, split with ;.\nLeave empty to allow commands anywhere.");
            }

            ImGui::SetCursorPos({10, 76});
            ImGui::Text("Authorized roles:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 73});
            ImGui::InputText("##authorized_roles", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "The ids of the roles authorized to issue bot commands in, split with ;.\nLeave empty to allow any role to use commands (unless users are specified).");
            }
            ImGui::SetCursorPos({10, 107});
            ImGui::Text("Authorized users:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 104});
            ImGui::InputText("##authorized_users", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "The ids of the users authorized to issue bot commands in, split with ;.\nLeave empty to allow any user to issue commands (unless roles are specified).");
            }
        }
        end_child();

        ImGui::SameLine();

        begin_child("Advanced", ImVec2(280, ImGui::GetWindowHeight()));

        ImGui::SetCursorPos({10, 11});
        ImGui::Checkbox("Use ephemeral replies", &bools[0]);
        end_child();
    }

    void draw_discord_info_tabs()
    {
        begin_child("Roles / People", ImVec2(300, ImGui::GetWindowHeight() / 2));
        {
            ImGui::SetCursorPos({10, 14});
            ImGui::Text("Helpers [No Access]:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 11});
            ImGui::InputText("##helpers_no_access", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "This role will be mentioned for any problems ling ling encounters\nthat can be fixed by someone without direct access to ling ling.\n"
                    "Examples:\n-An Achatina is missing or inaccessible.\n-The bed/teleporter of a task is missing.\n-The vault of a task is capped / close to cap.");
            }

            ImGui::SetCursorPos({10, 45});
            ImGui::Text("Helpers [Access]:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 42});
            ImGui::InputText("##helpers_with_access", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "This role will be mentioned for any fatal problems ling ling encounters\n and require direct access to be fixed."
                    "Examples:\n-An unexpected error at any task.\n-Failure to reconnect / restart .\n-Upcoming game update.");
            }
        }
        end_child();

        ImGui::SameLine();
        begin_child("Advanced", ImVec2(280, ImGui::GetWindowHeight())); {}
        end_child();

        ImGui::SetCursorPosY((ImGui::GetWindowHeight() / 2) + 5);
        begin_child("Channels", ImVec2(300, ImGui::GetWindowHeight() / 2));
        {
            ImGui::SetCursorPos({10, 14});
            ImGui::Text("Info Channel:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 11});
            ImGui::InputText("##info_channel", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "[REQUIRED] - General info will be posted here (stations completed, times taken...)");
            }

            ImGui::SetCursorPos({10, 45});
            ImGui::Text("Error Channel:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 42});
            ImGui::InputText("##error_channel", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "[OPTIONAL] - Errors and warnings will be posted here (game crashed, vaults full..)\nIf empty these messages will fall back to the info channel.");
            }
        }
        end_child();
    }

    inline int num = 0;
    inline int num2 = 0;
    inline int num3 = 0;

    void draw_bots_paste_tabs()
    {
        begin_child("Station Configuration", ImVec2(300, ImGui::GetWindowHeight()));
        {
            ImGui::SetCursorPos({10, 14});
            ImGui::Text("Station prefix:");
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 11});
            ImGui::InputText("##paste_prefix", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "The prefix for your paste beds, must be included in your bed name but not be the exact same.\n"
                    "For example your bed may be named COOL SPOT // PASTE00, but your prefix can still be PASTE.");
            }

            ImGui::SetCursorPos({10, 45});
            ImGui::Text("Render prefix:");
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 42});
            ImGui::InputText("##paste_render", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "The prefix for the render bed, for more info please read below.\n\n"
                    "Why is a render bed needed?\n"
                    "When the snail gets loaded before the structure it's on, it visually glitches into it.\n"
                    "Thus you have to render the structure, leave render of the dino and then move back to it.\n"
                    "For this youre going to need a source bed close to the snails named PREFIX::SRC\n"
                    "and a gateway bed out of snail rende named PREFIX::GATEWAY.");
            }

            ImGui::SetCursorPos({10, 76});
            ImGui::Text("Station count:");
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 73});
            ImGui::InputInt("##paste_count", &num, 1, 5);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip("The number of paste stations you have.");
            }
            num = (num <= 0) ? 1 : num;
            if (num > 100) { num = 100; }

            ImGui::SetCursorPos({10, 107});
            ImGui::Text("Interval (minutes):");
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 104});
            ImGui::InputInt("##paste_interval", &num2, 1, 5);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "The interval to complete the station at (in minutes).");
            }
            num2 = (num2 <= 5) ? 5 : num2;
            if (num2 > 100) { num2 = 100; }

            ImGui::SetCursorPos({10, 138});
            ImGui::Text("Load for (seconds):");
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 135});
            ImGui::InputInt("##paste_render", &num3, 1, 5);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "How long to let the stations render at the source render bed (in seconds).\nMore structures -> longer render time required.");
            }
            num3 = (num3 <= 5) ? 5 : num3;
            if (num3 > 100) { num3 = 100; }
        }
        end_child();

        ImGui::SameLine();
        begin_child("Advanced", ImVec2(280, ImGui::GetWindowHeight()));
        {
            ImGui::SetCursorPos({10, 11});
            ImGui::Checkbox("OCR deposited amount", &bools[0]);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "Enable to use OCR to determine how much paste was put into the dedi per station.\n"
                    "This info is sent to the completion embed on discord, it has no other purpose as of now.");
            }
            ImGui::Checkbox("Allow partial completion", &bools[0]);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "Allow the bot to break after a certain station and pick back up after that.\n\n"
                    "Example:\nConsider we have Task A, Task B and Task C where the order is their priority and C is\n"
                    "our PasteManager instance managing your paste stations.\nIf Task A and B are both on cooldown and C is started, A and B will be disregarded\n"
                    "until C completes. With partial completion, C may go to complete station 3, complete task A/B,\n"
                    "then pick C back up at 4.\n\n"
                    "Not recommended if it's going to have to partial complete alot and the rendering takes long.\n");
            }
            end_child();
        }
    }


    int selected_manager = 0;
    inline bool new_name_popup = false;
    inline bool confirmation_popup = false;

    void draw_bots_drops_tab()
    {
        begin_child("Crate Managers", ImVec2(300, ImGui::GetWindowHeight() * 0.33));
        {
            ImGui::SetCursorPos({10, 14});
            ImGui::Text("Selected Manager:");
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 11});
            std::vector<const char*>* data = config::bots::drops::managers.get_ptr();
            ImGui::Combo("##selected_manager", &selected_manager, data->data(),
                         data->size());

            ImGui::SetCursorPos({5, 45});
            if (ImGui::Button("Create manager")) { new_name_popup = true; }
            ImGui::SameLine();
            if (ImGui::Button("Delete selected manager") && !data->empty()) {
                confirmation_popup = true;
            }

            if (new_name_popup) {
                ImGui::OpenPopup("Name your new crate manager");
                if (ImGui::BeginPopupModal("Name your new crate manager", &new_name_popup,
                                           ImGuiWindowFlags_AlwaysAutoResize)) {
                    static char name_buffer[256] = {};
                    if (ImGui::IsWindowAppearing()) {
                        memset(name_buffer, 0, sizeof(name_buffer));
                    }

                    ImGui::InputText("##InputText", name_buffer,
                                     IM_ARRAYSIZE(name_buffer));
                    if (ImGui::Button("OK")) {
                        new_name_popup = false;
                        data->push_back(_strdup(name_buffer));
                        config::bots::drops::managers.save();
                    }
                    ImGui::EndPopup();
                }
            }

            if (confirmation_popup) {
                const auto name = (*data)[selected_manager];
                const std::string title = "Delete '" + std::string(name) + "'";
                ImGui::OpenPopup(title.c_str());

                if (ImGui::BeginPopupModal(title.c_str(), &confirmation_popup,
                                           ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Are you sure about this?\nThis cannot be undone!");
                    if (ImGui::Button("OK", ImVec2(80, 0))) {
                        printf("Action confirmed!\n");
                        confirmation_popup = false;
                        data->erase(data->begin() + selected_manager);
                        config::bots::drops::managers.save();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(80, 0))) {
                        printf("Action canceled!\n");
                        confirmation_popup = false;
                    }
                    ImGui::EndPopup();
                }
            }
        }
        end_child();

        ImGui::SameLine();
        begin_child("Advanced", ImVec2(280, ImGui::GetWindowHeight()));
        {
            ImGui::SetCursorPos({10, 14});
            ImGui::Text("Render (seconds):");
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 11});
            ImGui::InputInt("##crate_render", &num2, 1, 5);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "How long to let the first crate at each group render, more structures nearby -> longer render time.");
            }
            num2 = (num2 < 0) ? 0 : num2;
            if (num2 > 15) { num2 = 15; }

            ImGui::SetCursorPos({10, 45});
            ImGui::Checkbox("Overrule reroll mode", &bools[0]);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "Enable to always loot these crates, even when it is enabled.");
            }
            ImGui::SetCursorPos({10, 76});
            ImGui::Checkbox("Allow partial completion", &bools[0]);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "[ONLY SUPPORTED FOR BED STATIONS]\n\n"
                    "Allow the bot to break after a certain station and pick back up after that.\n\n"
                    "Example:\nConsider we have Task A, Task B and Task C where the order is their priority and C is\n"
                    "our CrateManager instance managing the crates.\nIf Task A and B are both on cooldown and C is started, A and B will be disregarded\n"
                    "until C completes. With partial completion, C may go to complete station 3, complete task A/B,\n"
                    "then pick C back up at 4.");
            }
        }
        end_child();

        ImGui::SetCursorPosY((ImGui::GetWindowHeight() * 0.33) + 5);
        begin_child("Configuration", ImVec2(300, ImGui::GetWindowHeight() * 0.66));
        {
            ImGui::SetCursorPos({10, 14});
            ImGui::Text("Station prefix:");
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 11});
            ImGui::InputText("##crate_prefix", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "The prefix of this crate manager.\n\n"
                    "The following structure instances will be created with the prefix:\n"
                    "-[PREFIX]::ALIGN - The initial bed to spawn on for teleporter mode\n"
                    "-[PREFIX]::DROPXX - The teleporter / bed per drop where XX is it's index\n"
                    "-[PREFIX]::DROPOFF - The vault the items will be stored in for teleporter mode\n");
            }

            ImGui::SetCursorPos({10, 45});
            ImGui::Text("Grouped Crates:");
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 42});
            ImGui::InputText("##crate_groups", dir_buffer, 256);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "An array of arrays where each array represents one group of crates.\n"
                    "Each group corresponds to crates that share the same uptime,\n"
                    "meaning only one of them will be active at any given time (e.g., Swamp Cave big room).\n\n"
                    "Guidelines:\n" " - Enclose each group in curly braces { and }.\n"
                    " - Separate elements within a group with commas.\n"
                    " - For each crate 'j' per crates[i], provide one bed or tp with the corresponding name.\n"
                    " - Define each crate with its color options: BLUE, YELLOW, RED.\n"
                    " - Use a pipe '|' to separate multiple options for a crate (e.g., YELLOW | RED).\n"
                    " - 'ANY' can be used to represent multiple color options for a crate.\n\n"
                    "Example for Island Swamp Cave:\n"
                    "{RED, RED}, {YELLOW, YELLOW, ANY}, {BLUE}");
            }

            ImGui::SetCursorPos({10, 76});
            ImGui::Text("Interval (minutes):");
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45f);
            ImGui::SetCursorPos({150, 73});
            ImGui::InputInt("##crate_interval", &num2, 1, 5);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "The interval to complete the stations at (in minutes).");
            }
            num2 = (num2 <= 5) ? 5 : num2;
            if (num2 > 100) { num2 = 100; }

            ImGui::SetCursorPos({10, 107});
            ImGui::Checkbox("Uses teleporters", &bools[5]);
        }
        end_child();
    }

    void begin_child(const char* name, ImVec2 size)
    {
        const ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImVec2 pos = window->DC.CursorPos;

        ImGui::BeginChild(std::string(name).append(".main").c_str(), size, false,
                          ImGuiWindowFlags_NoScrollbar);

        // draw the name of this tab
        ImGui::GetWindowDrawList()->AddRectFilled(pos, pos + size,
                                                  ImColor(28 / 255.f, 30 / 255.f,
                                                          36 / 255.f, animation), 4);
        ImGui::GetWindowDrawList()->AddText(pos + ImVec2(10, 5),
                                            get_accent_color(animation), name,
                                            ImGui::FindRenderedTextEnd(name));

        ImGui::SetCursorPosY(30);
        ImGui::BeginChild(name, {size.x, size.y - 30});
        ImGui::SetCursorPosX(10);

        ImGui::BeginGroup();

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, animation);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8, 8});
    }

    void end_child()
    {
        ImGui::PopStyleVar(2);
        ImGui::EndGroup();
        ImGui::EndChild();
        ImGui::EndChild();
    }
}