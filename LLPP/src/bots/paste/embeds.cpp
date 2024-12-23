#include <asapp/game/window.h>
#include "embeds.h"
#include "../../common/util.h"
#include "../../core/basestation.h"
#include "../../discord/bot.h"
#include <format>

#include "../../config/config.h"
#include "../../discord/helpers.h"

using namespace llpp::config::discord;

namespace llpp::bots::paste
{
    namespace
    {
        const char* PASTE_ICON_URL =
            "https://static.wikia.nocookie.net/arksurvivalevolved_gamepedia/images/0/03/Cementing_Paste.png/revision/latest?cb=20180801020251";
    }

    void send_paste_collected(const core::StationResult& data)
    {
        auto next = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now() + data.station->
                                                    get_completion_interval());

        const int paste_got = data.obtained_items.at("Achatina Paste");
        const std::string obtained = paste_got > 600 ? "N/A" : std::to_string(paste_got);

        auto embed = dpp::embed();
        embed.set_color(dpp::colors::gray).
              set_title(std::format("Paste Station '{}' has been completed.",
                                    data.station->get_name())).set_description(
                  std::format("The station was completed successfully {} times!",
                              data.station->get_times_completed())).
              set_thumbnail(PASTE_ICON_URL).add_field(
                  "Time taken:", std::format("{} seconds", data.time_taken.count()),
                  true).add_field("Paste obtained:", obtained, true).add_field(
                  "Next completion:", std::format("<t:{}:R>", next), true);

        if (obtained == "N/A") {
            embed.set_footer({"Amount of paste collected could not be deduced."});
        }
        discord::set_now_timestamp(embed);
        auto msg = dpp::message(config::discord::channels::info.get(), embed);
        discord::get_bot()->message_create(msg);
    }

    void send_paste_not_deposited(const std::string& station)
    {
        auto embed = dpp::embed();
        embed.set_color(0xf6c330).
              set_title(std::format("Failed to deposit paste at '{}'!", station)).
              set_description(
                  "- The obtained paste will be in a bag.\n"
                  "- Ensure the dedi is set to deposit priority.\n"
                  "- Ensure that the dedi is aligned correctly.").set_thumbnail(
                  "https://static.wikia.nocookie.net/arksurvivalevolved_gamepedia/"
                  "images/f/f6/Radiation.png/revision/latest?cb=20171216191751").
              set_image("attachment://image.png");

        auto fdata = discord::strbuf(asa::window::screenshot());

        dpp::message msg;
        if (channels::error.get_ptr()->empty()) {
            msg.set_channel_id(channels::info.get());
        }
        else { msg.set_channel_id(channels::error.get()); }

        msg.set_content(dpp::utility::role_mention(roles::helper_no_access.get())).
            set_allowed_mentions(false, true, false, false, {}, {}).add_file(
                "image.png", fdata, "image/png ").add_embed(embed);

        if (config::bots::paste::mute_pings.get()) {
            msg.set_content("");
        }
        discord::set_now_timestamp(embed);
        discord::get_bot()->message_create(msg);
    }

    void send_achatina_not_accessible(const std::string& station_name,
                                      const std::string& achatina_name)
    {
        auto embed = dpp::embed();
        embed.set_color(0xf6c330).
              set_title(std::format("Problem with Achatina at '{}'!", station_name)).
              set_description(std::format(
                  "An achatina may be inaccessible, manual check required.",
                  achatina_name)).set_thumbnail(
                  "https://static.wikia.nocookie.net/" "arksurvivalevolved_gamepedia/"
                  "images/f/f6/Radiation.png/revision/latest?cb=20171216191751").
              add_field("Station:", station_name, true).add_field(
                  "Achatina: ", achatina_name, true).set_image("attachment://image.png");

        auto fileData = discord::strbuf(asa::window::screenshot());
        dpp::message message = dpp::message(discord::get_error_channel(),
                                            std::format(
                                                "<@&{}>",
                                                roles::helper_no_access.get())).
            set_allowed_mentions(false, true, false, false, {}, {});
        message.add_file("image.png", fileData, "image/png ").add_embed(embed);
        if (config::bots::paste::mute_pings.get()) {
            message.set_content("");
        }
        discord::get_bot()->message_create(message);
    }
}
