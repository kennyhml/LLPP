#pragma once
#include <asapp/entities/localplayer.h>
#include "../../../core/teleportstation.h"


namespace llpp::bots::farm
{
    class MetalStation final : public core::TeleportStation
    {
    public:
        MetalStation(std::string t_name, std::chrono::minutes t_interval,
                     std::shared_ptr<asa::entities::DinoEntity> t_anky);

        core::StationResult complete() override;

    private:
        /**
         * @brief Swings the amount of times previously determined by count_swings.
         */
        void harvest_metal();

        /**
         * @brief Returns whether logs should be checked considering the last time
         * they were checked and the interval to check them at.
         */
        static bool should_check_logs() { return util::timedout(last_logs_, log_cd_); }

        /**
         * @brief Counts how many swings are required to fully farm this station.
         *
         * This is achieved by swinging once, waiting 10 seconds to harvest a resource
         * (to account for lag), waiting for the popup to disappear and repeating this
         * process until we can no longer harvest any resource.
         *
         * @return The amount of times that we successfully hit ANY resource.
         */
        [[nodiscard]] int count_required_swings() const;

        /**
         * @brief Drops the undesired items from the anky.
         */
        void drop_trash() const;

        inline static std::chrono::system_clock::time_point last_logs_;
        inline static std::chrono::seconds log_cd_{60};

        int swing_times_ = 0;
        std::shared_ptr<asa::entities::DinoEntity> anky_;
    };
}
