#pragma once
#include <string>

namespace llpp::bots::farm
{
    [[nodiscard]] int get_swings(const std::string& for_station);

    void set_swings(const std::string& for_station, int swings);
}
