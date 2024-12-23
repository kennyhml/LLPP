#include "suicideembed.h"
#include "../../core/basestation.h"
#include "../../discord/bot.h"
#include <format>

#include "../../config/config.h"


namespace llpp::bots::suicide
{
    void send_suicided_embed(const core::StationResult& data, const std::string& at_bed,
                             const std::string& respawning_at)
    {
        auto embed = dpp::embed();
        embed.set_color(dpp::colors::light_blue).
              set_title(std::format("Successfully suicided at '{}'.", at_bed)).
              set_description(std::format("Suicided a total of {} times!",
                                          data.station->get_times_completed())).
              set_thumbnail(
                  "https://static.wikia.nocookie.net/"
                  "arksurvivalevolved_gamepedia/images/5/55/"
                  "Specimen_Implant.png/revision/latest?cb=20180731184153").add_field(
                  "Time taken:", std::format("{} seconds", data.time_taken.count()),
                  true).add_field("Respawning at:", respawning_at, true);

        auto message = dpp::message(config::discord::channels::info.get(), embed);
        discord::get_bot()->message_create(message);
    }
}
