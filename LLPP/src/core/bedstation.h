#pragma once
#include <asapp/structures/simplebed.h>

#include "basestation.h"

namespace llpp::core
{
    class BedStation : public BaseStation
    {
    public:
        BedStation(std::string t_name, std::chrono::minutes t_interval);

    protected:

        /**
         * @brief Spawns at the bed, checks logs and sets the start time.
         */
        [[nodiscard]] bool begin() override;

        asa::structures::SimpleBed spawn_bed_;
    };
}