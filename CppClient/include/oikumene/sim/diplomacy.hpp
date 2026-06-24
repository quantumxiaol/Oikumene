#pragma once

#include <string>

#include "oikumene/core/types.hpp"

namespace oikumene {

enum class DiplomaticPosture {
    Neutral,
    Friendly,
    Competitive,
    Dependent,
    BlockadeRisk,
};

[[nodiscard]] std::string ToString(DiplomaticPosture posture);

struct DiplomacyRelation {
    int id = -1;
    PolityId polity_a_id = kInvalidPolityId;
    PolityId polity_b_id = kInvalidPolityId;

    bool has_active_trade = false;
    int trade_id = -1;
    float trade_profit = 0.0F;
    float trade_complementarity = 0.0F;
    float trade_route_efficiency = 0.0F;
    float trade_route_cost = 0.0F;

    float border_tension = 0.0F;
    float economic_overlap = 0.0F;
    float dependence_a_on_b = 0.0F;
    float dependence_b_on_a = 0.0F;
    PolityId dependent_polity_id = kInvalidPolityId;
    PolityId leverage_polity_id = kInvalidPolityId;

    float friendship = 0.0F;
    float competition = 0.0F;
    float blockade_tendency = 0.0F;
    DiplomaticPosture posture = DiplomaticPosture::Neutral;
    std::string reason;
};

} // namespace oikumene
