#pragma once

#include <string>

#include "oikumene/core/types.hpp"

namespace oikumene {

enum class WarObjective {
    BorderDispute,
    TradeCoercion,
    Blockade,
    DependencyBreakout,
};

[[nodiscard]] std::string ToString(WarObjective objective);

struct WarPressure {
    int id = -1;
    int relation_id = -1;
    PolityId actor_polity_id = kInvalidPolityId;
    PolityId target_polity_id = kInvalidPolityId;
    WarObjective objective = WarObjective::BorderDispute;

    float military_ratio = 0.0F;
    float military_advantage = 0.0F;
    float border_pressure = 0.0F;
    float trade_conflict_weight = 0.0F;
    float dependency_pressure = 0.0F;
    float blockade_pressure = 0.0F;
    float friendly_penalty = 0.0F;
    float grievance_pressure = 0.0F;
    float restraint_pressure = 0.0F;
    float vassalage_pressure = 0.0F;

    float target_value = 0.0F;
    float campaign_cost = 0.0F;
    float base_roi = 0.0F;
    float diplomatic_modifier = 0.0F;
    float war_roi = 0.0F;
    float declaration_pressure = 0.0F;
    bool high_pressure = false;
    std::string reason;
};

} // namespace oikumene
