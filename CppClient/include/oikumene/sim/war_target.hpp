#pragma once

#include <string>
#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/pathfinding.hpp"
#include "oikumene/sim/war.hpp"

namespace oikumene {

enum class WarTargetKind {
    Settlement,
    ResourceRegion,
    ContestedBorder,
    TradeRouteNode,
    StrategicPass,
};

[[nodiscard]] std::string ToString(WarTargetKind kind);

struct WarTargetCandidate {
    int id = -1;
    int pressure_id = -1;
    PolityId actor_polity_id = kInvalidPolityId;
    PolityId target_polity_id = kInvalidPolityId;
    WarObjective objective = WarObjective::BorderDispute;
    WarTargetKind kind = WarTargetKind::Settlement;

    int x = 0;
    int y = 0;
    int target_settlement_id = -1;
    int target_trade_id = -1;
    std::vector<TileCoord> path;
    float route_cost = 0.0F;

    float farmland_value = 0.0F;
    float resource_value = 0.0F;
    float settlement_value = 0.0F;
    float trade_value = 0.0F;
    float strategic_value = 0.0F;
    float target_value = 0.0F;

    float mobilization_cost = 0.0F;
    float supply_cost = 0.0F;
    float equipment_cost = 0.0F;
    float terrain_attrition = 0.0F;
    float defender_power = 0.0F;
    float defense_cost = 0.0F;
    float occupation_cost = 0.0F;
    float campaign_cost = 0.0F;

    float roi = 0.0F;
    float action_score = 0.0F;
    bool high_value = false;
    std::string reason;
};

} // namespace oikumene
