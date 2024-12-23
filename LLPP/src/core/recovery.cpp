#include "recovery.h"
#include "../common/util.h"
#include "../discord/bot.h"
#include <asapp/entities/localplayer.h>
#include <asapp/game/window.h>
#include <asapp/interfaces/console.h>
#include <asapp/interfaces/mainmenu.h>
#include <asapp/interfaces/modeselect.h>
#include <asapp/interfaces/serverselect.h>
#include <asapp/interfaces/spawnmap.h>

#include "../config/config.h"
#include "../discord/helpers.h"

namespace llpp::core
{
    void recover()
    {
        const auto start = std::chrono::system_clock::now();
        asa::core::set_crash_aware(true);

        bool need_restart = false;
        bool need_reconnect = false;

        if (asa::window::has_crashed_popup()) {
            std::cout << "[+] Recovery sequence initiated, diagnosing cause of crash...\n";
            std::cout << "\t[-] The game has crashed...\n";
            need_restart = true;
            need_reconnect = true;
        }
        else if (asa::interfaces::main_menu->is_open() ||
                    asa::interfaces::mode_select->is_open() ||
                    asa::interfaces::server_select->is_open()) {
            std::cout << "[+] Recovery sequence initiated, diagnosing cause of crash...\n";
            std::cout << "\t[-] Kicked to main menu...\n";
            need_reconnect = true;
        }

        // Not needing restart or to reconnect so return here
        if(!need_restart && !need_reconnect) {
            asa::core::set_crash_aware(false);
            return;
        }

        inform_recovery_initiated(need_restart, need_reconnect);
        if (need_restart) { restart_game(); }
        reconnect_to_server();
        inform_recovery_successful(util::get_elapsed<std::chrono::seconds>(start));
        asa::core::set_crash_aware(false);

        if (asa::interfaces::spawn_map->is_open()) {
            asa::interfaces::spawn_map->spawn_at("RESET BED");
            std::this_thread::sleep_for(std::chrono::seconds(15));
        }
    }

    void reconnect_to_server()
    {
        if (asa::interfaces::main_menu->is_open()) {
            asa::interfaces::main_menu->accept_popup();
            util::await([] {return asa::interfaces::main_menu->is_open(); }, 30s);
            asa::interfaces::main_menu->start();
        }
        if(util::await([]{return asa::interfaces::mode_select->is_open();}, std::chrono::seconds(5))) {
            asa::interfaces::mode_select->join_game();
        }

        asa::interfaces::server_select->join_server(config::general::ark::server.get());

        while (true) {
            if (asa::entities::local_player->is_alive() || asa::interfaces::spawn_map->
                is_open()) { break; }

            if (asa::interfaces::main_menu->is_open()) {
                asa::interfaces::mode_select->join_game();
                Sleep(1000);
                std::cerr << "[!] Joining failed, trying again\n";
                return reconnect_to_server();
            }
        }

        Sleep(5000);
        std::cout << "[+] Reconnected successfully.\n";
    }

    void restart_game()
    {
        exit_game();
        Sleep(5000);
        if (system("start steam://rungameid/2399830") == -1) {
            throw std::runtime_error("Failed to restart the game");
        }
        asa::window::get_handle(60, true);

        while (!asa::interfaces::main_menu->is_open()) {
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }

        std::cout << "[+] Game restarted.\n";
        asa::interfaces::console->execute(config::general::bot::commands.get());
    }

    void exit_game()
    {
        DWORD pid;

        GetWindowThreadProcessId(asa::window::hWnd, &pid);
        const HANDLE hprocess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!hprocess) { throw std::runtime_error("Process handle not acquired"); }
        TerminateProcess(hprocess, 0);
        CloseHandle(hprocess);
    }

    void inform_crash_detected(asa::core::ShooterGameError& e)
    {
        auto embed = dpp::embed();
        embed.set_color(0xB82E88).set_title("CRITICAL: The game or server has crashed.").
              set_description(e.what()).set_thumbnail(
                  "https://static.wikia.nocookie.net/" "arksurvivalevolved_gamepedia/"
                  "images/5/53/Mission_Area.png/revision/" "latest?cb=20200314145130").
              add_field("Assistance required:", "False").
              set_image("attachment://image.png").set_footer(
                  dpp::embed_footer("Recovery should follow automatically."));

        const auto file_data = discord::strbuf(asa::window::screenshot());
        dpp::message message = dpp::message(config::discord::channels::info.get(),
                                            std::format(
                                                "<@&{}>",
                                                config::discord::roles::helper_access.
                                                get())).set_allowed_mentions(
            false, true, false, false, {}, {});
        message.add_file("image.png", file_data, "image/png ").add_embed(embed);

        discord::get_bot()->message_create(message);
    }

    void inform_recovery_initiated(const bool restart, const bool reconnect)
    {
        auto embed = dpp::embed();
        embed.set_color(0x7F5D44).set_title("Recovery sequence was initiated!").
              set_description("Recovery attempt imminent. Required steps:").set_thumbnail(
                  "https://static.wikia.nocookie.net/" "arksurvivalevolved_gamepedia/"
                  "images/2/24/Crafting_Light.png/revision/" "latest?cb=20181217123945").
              add_field("Restart required:", restart ? "Yes" : "No", true).add_field(
                  "Reconnect required:", reconnect ? "Yes" : "No", true);

        if (reconnect) {
            embed.add_field("Reconnecting to:", config::general::ark::server.get(), true);
        }

        const auto message = dpp::message(config::discord::channels::info.get(), embed);
        discord::get_bot()->message_create(message);
    }

    void inform_recovery_successful(const std::chrono::seconds time_taken)
    {
        auto embed = dpp::embed();
        embed.set_color(dpp::colors::green).set_title("Recovery sequence was successful!")
             .set_description("Ling Ling++ has successfully recovered itself.").
             set_thumbnail(
                 "https://static.wikia.nocookie.net/" "arksurvivalevolved_gamepedia/"
                 "images/b/b5/Imprinted.png/revision/latest?cb=20181217131908").add_field(
                 "Time taken:", std::format("{} seconds", time_taken.count())).set_image(
                 "attachment://image.png");


        const auto file_data = discord::strbuf(asa::window::screenshot());
        auto message = dpp::message(config::discord::channels::info.get(), embed);
        message.add_file("image.png", file_data, "image/png ");

        discord::get_bot()->message_create(message);
    }
}
