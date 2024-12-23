#include "bed_crate_station.h"
#include "embeds.h"
#include "../../discord/bot.h"
#include <asapp/entities/localplayer.h>
#include <asapp/core/state.h>

namespace llpp::bots::drops
{
    BedCrateStation::BedCrateStation(const std::string& t_name,
                                     CrateManagerConfig* t_config,
                                     asa::structures::CaveLootCrate t_crate,
                                     const bool t_is_first,
                                     const bool t_is_first_in_group)
        : BedStation(t_name, std::chrono::minutes(t_config->interval)),
          CrateStation(t_config, std::move(t_crate), t_is_first, t_is_first_in_group),
          vault_(name_ + "::DROPOFF", 350) {}


    core::StationResult BedCrateStation::complete()
    {
        if (!begin()) {
            return {this, false, get_time_taken<std::chrono::seconds>(), {}};
        }

        asa::entities::local_player->set_pitch(30);
        is_up_ = await_crate_loaded();
        if (!is_up_) {
            set_completed();
            std::cout << "[-] Loot crate completed: " << name_ << " (but nothing looted)" << std::endl;
            return {this, false, get_time_taken<std::chrono::seconds>(), {}};
        }

        cv::Mat loot_image;
        std::vector<LootResult> contents;
        const auto quality = crate_.get_crate_quality();

        loot(loot_image, contents);
        previous_loot_ = contents;
        // only deposit items if we actually got any.
        if (!should_reroll() || any_looted(contents)) { deposit_items(); }

        set_completed();
        const core::StationResult res(this, true, get_time_taken<std::chrono::seconds>(),
                                      {});

        dpp::message msg;
        if (!should_reroll()) {
            msg = get_looted_message(res, loot_image, quality, ++times_looted_, contents);
        }
        else {
            msg = get_reroll_message(res, loot_image, quality,
                                     last_found_up_ + buff_max_wait_, contents);
        }
        discord::get_bot()->message_create(msg);
        std::cout << "[-] Loot crate completed: " << name_ << std::endl;
        return res;
    }

    void BedCrateStation::deposit_items()
    {
        asa::entities::local_player->turn_right();
        // Look down slightly before accessing the vault in case it is a dedi
        asa::entities::local_player->turn_down(30);
        asa::entities::local_player->access(vault_);
        asa::entities::local_player->get_inventory()->transfer_all();
        asa::core::sleep_for(std::chrono::seconds(1));

        vault_.get_current_slots();
        vault_.get_inventory()->close();

        asa::core::sleep_for(std::chrono::seconds(1));
    }
}
