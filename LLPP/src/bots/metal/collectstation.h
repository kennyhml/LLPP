#pragma once
#include <asapp/structures/dedicatedstorage.h>
#include "../../core/bedstation.h"

namespace llpp::bots::metal
{
    class CollectStation final : public core::BedStation
    {
    public:
        explicit CollectStation(const std::string& t_name);

        /**
         * @brief Completes the station by spawning there and collecting the metal.
         *
         * @return The result of the station completion.
         */
        core::StationResult complete() override;

    private:
        std::vector<asa::structures::DedicatedStorage> dedis_;
    };
}