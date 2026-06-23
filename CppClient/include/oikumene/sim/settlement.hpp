#pragma once

#include <string>
#include <vector>

#include "oikumene/sim/stockpile.hpp"

namespace oikumene {

enum class SettlementLevel {
    Camp,
    Village,
};

[[nodiscard]] std::string ToString(SettlementLevel level);

struct Settlement {
    int id = -1;
    int x = 0;
    int y = 0;
    int founder_band_id = -1;
    int population = 30;
    SettlementLevel level = SettlementLevel::Camp;
    Stockpile stockpile;
    int turns_since_founded = 0;
    float local_food_output_last_turn = 0.0F;
    float local_wood_output_last_turn = 0.0F;
    float food_consumption_last_turn = 0.0F;
    float upgrade_readiness = 0.0F;
    int work_radius = 2;
    std::vector<int> worked_tile_indices;
    float ore_output_last_turn = 0.0F;
    float carrying_capacity = 45.0F;
    float carrying_capacity_ratio = 0.0F;
    int worked_tile_count = 0;
};

}  // namespace oikumene
