#include <iostream>
#include <asapp/items/items.h>
#include "cropstation.h"
#include <asapp/core/state.h>
#include <asapp/entities/localplayer.h>
#include "embeds.h"

namespace llpp::bots::kitchen
{
    namespace
    {
        void get_crop_and_seed(const CropStation::CropType type,
                               asa::items::Item*& crop_out, asa::items::Item*& seed_out)
        {
            switch (type) {
            case CropStation::CropType::SAVOROOT: crop_out =
                    asa::items::consumables::savoroot;
                seed_out = asa::items::consumables::savoroot_seed;
                break;
            case CropStation::CropType::ROCKARROT: crop_out =
                    asa::items::consumables::rockarrot;
                seed_out = asa::items::consumables::rockarrot_seed;
                break;
            case CropStation::CropType::CITRONAL: crop_out =
                    asa::items::consumables::citronal;
                seed_out = asa::items::consumables::citronal_seed;
                break;
            case CropStation::CropType::LONGRASS: crop_out =
                    asa::items::consumables::longrass;
                seed_out = asa::items::consumables::longrass_seed;
                break;
            }
        }
    }

    CropStation::CropStation(std::string t_name, const CropType crop,
                             const std::chrono::minutes t_interval) :
        BedStation(std::move(t_name), t_interval), fridge_(name_, 80),
        vault_(name_ + " FERTILIZER", 350),
        crop_plots_({
            {"PLOT01"}, {"PLOT02"}, {"PLOT03"}, {"PLOT04"}, {"PLOT05"}, {"PLOT06"}
        }) { get_crop_and_seed(crop, crop_, seed_); };

    core::StationResult CropStation::complete()
    {
        if (!begin()) {
            return {this, false, get_time_taken<std::chrono::seconds>(), {}};
        }

        const int slots_needed = get_slots_to_refill();

        if (!slots_needed) {
            std::cout << "[+] Fridge is completely filled!\n";
            return {this, true, std::chrono::seconds(0), {}};
        }

        grab_fertilizer();
        get_crops(slots_needed + 1);
        int slots_in_fridge = -1;
        put_crops_in_fridge(slots_in_fridge);
        deposit_fertilizer();

        const auto time_taken = get_time_taken<std::chrono::seconds>();
        const core::StationResult res(this, true, time_taken, {});
        set_completed();
        send_crops_collected(res, crop_, slots_in_fridge);

        return res;
    }

    int CropStation::get_slots_to_refill()
    {
        std::cout << "[+] Checking current amount of crops...\n";
        asa::entities::local_player->access(fridge_);
        std::cout << "\t[-] Fridge accessed, restacking crops... ";
        fridge_.get_inventory()->auto_stack();
        std::cout << "Done.\n";

        // if the server is laggy the slots my need some time to load
        asa::core::sleep_for(std::chrono::seconds(2));
        std::cout << "\t[-] Counting current slots of crops... ";
        const int filled_slots = fridge_.get_current_slots();
        std::cout << filled_slots << "\\" << fridge_.get_max_slots() << ".\n";

        fridge_.get_inventory()->close();
        asa::core::sleep_for(std::chrono::seconds(1));
        return fridge_.get_max_slots() - filled_slots;
    }

    void CropStation::turn_to_crop_plots() const
    {
        if (aligned_ == RIGHT) { asa::entities::local_player->turn_right(); }
        else { asa::entities::local_player->turn_left(); }
        asa::core::sleep_for(std::chrono::milliseconds(500));
    }

    void CropStation::grab_fertilizer() const
    {
        asa::entities::local_player->turn_up(180);
        asa::entities::local_player->access(vault_);
        vault_.get_inventory()->transfer_rows(*asa::items::resources::fertilizer, 5);
        vault_.get_inventory()->close();
        asa::core::sleep_for(std::chrono::seconds(1));
        asa::entities::local_player->turn_down(90);
    }

    void CropStation::deposit_fertilizer() const
    {
        asa::entities::local_player->turn_up(180);
        asa::entities::local_player->access(vault_);
        asa::entities::local_player->get_inventory()->transfer_all();
        vault_.get_inventory()->close();
        asa::core::sleep_for(std::chrono::seconds(1));
        asa::entities::local_player->turn_down(90);
    }

    /**
     * @brief Puts the obtained crop of the station into the fridge.
     *
     * Finalizes the station by putting the crops we have taken out of the
     * crop plots into the fridge.
     *
     * TODO: OCR how many were added exactly to inform the user
     *
     * @return None
     */
    void CropStation::put_crops_in_fridge(int& fridge_slots_out)
    {
        aligned_ == LEFT
            ? asa::entities::local_player->turn_right()
            : asa::entities::local_player->turn_left();

        asa::entities::local_player->turn_down(180);
        asa::entities::local_player->turn_up(90);

        asa::entities::local_player->access(fridge_);
        asa::entities::local_player->get_inventory()->transfer_all(*crop_);

        while (asa::entities::local_player->get_inventory()->has(*crop_)) {
            if (fridge_.get_current_slots() == 80) { break; }
        }
        asa::core::sleep_for(std::chrono::milliseconds(500));
        fridge_.get_inventory()->transfer_all(*seed_);
        asa::core::sleep_for(std::chrono::milliseconds(500));
        fridge_slots_out = fridge_.get_current_slots();
        fridge_.get_inventory()->close();
        asa::core::sleep_for(std::chrono::seconds(1));
    }

    void CropStation::empty(const asa::structures::MediumCropPlot& plot,
                            int& current_slots, const bool count) const
    {
        std::cout << "[+] Emptying " << plot.get_name() << "\n";
        asa::entities::local_player->access(plot);
        plot.get_inventory()->transfer_all(*crop_);
        std::cout << "\t[-] Took all " << crop_->get_name() << ". Waiting for server... ";
        while (count && plot.get_inventory()->has(*crop_)) {}
        std::cout << "Done\n";

        if (count) {
            asa::entities::local_player->get_inventory()->count_stacks(
                *crop_, current_slots, true);
            std::cout << "\t[-] Current slots: " << current_slots << ".\n";
        }
        asa::core::sleep_for(std::chrono::seconds(1));

        asa::entities::local_player->get_inventory()->transfer_all();
        asa::entities::local_player->get_inventory()->close();
        asa::core::sleep_for(std::chrono::milliseconds(300));
    }

    void CropStation::get_crops(const int how_many_slots)
    {
        static std::array<int, 6> turns = {-25, 25, 25, -25, 27, 25};
        constexpr auto delay_per_turn = std::chrono::milliseconds(300);

        asa::core::sleep_for(std::chrono::seconds(1));
        turn_to_crop_plots();
        int slots_looted = 0;
        const bool count_required = how_many_slots < 8;
        asa::entities::local_player->crouch();

        for (int i = 0; i < 6 && slots_looted < how_many_slots; i++) {
            if (i == 3) { asa::entities::local_player->stand_up(); }
            asa::entities::local_player->turn_up(turns[i], delay_per_turn);

            if (aligned_ == UNKNOWN) {
                asa::core::sleep_for(std::chrono::seconds(1));
                if (asa::entities::local_player->can_access_bed()) {
                    aligned_ = RIGHT;
                    asa::entities::local_player->turn_right(180);
                }
                else { aligned_ = LEFT; }
            }

            empty(crop_plots_[i], slots_looted, count_required);
        }
    }
}
