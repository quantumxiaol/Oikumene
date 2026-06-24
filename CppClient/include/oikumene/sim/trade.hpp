#pragma once

#include <string>

#include "oikumene/core/types.hpp"

namespace oikumene {

enum class TradeGood {
    None,
    Food,
    Wood,
    Ore,
    Wealth,
};

[[nodiscard]] std::string ToString(TradeGood good);

struct TradeAgreement {
    int id = -1;
    PolityId polity_a_id = kInvalidPolityId;
    PolityId polity_b_id = kInvalidPolityId;
    Turn opened_turn = 0;
    Turn last_evaluated_turn = 0;
    bool active = true;

    TradeGood export_from_a = TradeGood::None;
    TradeGood export_from_b = TradeGood::None;
    float value_a_to_b = 0.0F;
    float value_b_to_a = 0.0F;

    float complementarity = 0.0F;
    float route_cost = 0.0F;
    float route_cost_without_network = 0.0F;
    float route_saving = 0.0F;
    float route_efficiency = 0.0F;
    float gross_value = 0.0F;
    float transport_cost = 0.0F;
    float expected_profit = 0.0F;
    std::string reason;
};

} // namespace oikumene
