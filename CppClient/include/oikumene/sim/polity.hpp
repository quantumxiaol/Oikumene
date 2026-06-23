#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "oikumene/core/types.hpp"

namespace oikumene {

enum class PolityLevel {
    Chiefdom,
    CityState,
    Kingdom,
};

[[nodiscard]] std::string ToString(PolityLevel level);

struct PolityBudget {
    float food_income = 0.0F;
    float wood_income = 0.0F;
    float ore_income = 0.0F;
    float wealth_income = 0.0F;

    float food_maintenance = 0.0F;
    float wood_maintenance = 0.0F;
    float admin_maintenance = 0.0F;
    float control_maintenance = 0.0F;

    float food_surplus = 0.0F;
    float wood_surplus = 0.0F;
    float wealth_surplus = 0.0F;
};

struct Polity {
    PolityId id = kInvalidPolityId;
    std::string name;
    PolityLevel level = PolityLevel::Chiefdom;
    int capital_settlement_id = -1;
    std::vector<int> member_settlement_ids;

    int population = 0;
    float food = 0.0F;
    float wood = 0.0F;
    float ore = 0.0F;
    float wealth = 0.0F;
    float control_power = 0.0F;
    float admin_range = 0.0F;
    float legitimacy = 1.0F;
    float stability = 1.0F;
    float admin_load = 0.0F;
    float admin_capacity = 0.0F;
    float overextension = 0.0F;
    PolityBudget budget;
    std::uint32_t debug_color_index = 0;
    int controlled_tile_count = 0;
    int contested_tile_count = 0;
    Turn founded_turn = 0;
};

}  // namespace oikumene
