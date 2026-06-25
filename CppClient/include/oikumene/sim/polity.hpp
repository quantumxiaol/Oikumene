#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/technology.hpp"

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
    ResearchState research;
    float knowledge_income = 0.0F;
    float military_potential = 0.0F;
    float tool_efficiency = 1.0F;
    std::vector<int> route_ids;
    float route_maintenance = 0.0F;
    int connected_settlements = 0;
    int connected_mines = 0;
    int connected_mine_potential = 0;
    int active_connected_mines = 0;
    float connected_ore_income = 0.0F;
    float unconnected_ore_income = 0.0F;
    float admin_distance_saving = 0.0F;
    std::vector<int> trade_ids;
    int active_trade_count = 0;
    float trade_income = 0.0F;
    float trade_expense = 0.0F;
    float trade_profit = 0.0F;
    float trade_complementarity = 0.0F;
    float trade_route_cost = 0.0F;
    float trade_route_efficiency = 0.0F;
    float occupation_load = 0.0F;
    float occupation_unrest = 0.0F;
    int occupied_settlements = 0;
    int vassal_count = 0;
    std::uint32_t debug_color_index = 0;
    int controlled_tile_count = 0;
    int contested_tile_count = 0;
    Turn founded_turn = 0;
};

} // namespace oikumene
