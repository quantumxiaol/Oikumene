#pragma once

#include <string>
#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/pathfinding.hpp"
#include "oikumene/world/route_kind.hpp"

namespace oikumene {

enum class RoutePurpose {
    Admin,
    Resource,
    SettlementLink,
    FutureTrade,
    Strategic,
};

struct RouteSegment {
    int x = 0;
    int y = 0;
    RouteKind kind = RouteKind::Trail;
    float quality = 0.0F;
    float maintenance = 0.0F;
    PolityId polity_id = kInvalidPolityId;
};

struct Route {
    int id = -1;
    PolityId polity_id = kInvalidPolityId;
    int from_settlement_id = -1;
    int to_settlement_id = -1;
    int target_x = 0;
    int target_y = 0;
    RoutePurpose purpose = RoutePurpose::Admin;
    RouteKind kind = RouteKind::Trail;
    std::vector<TileCoord> path;
    float build_cost_wood = 0.0F;
    float build_cost_wealth = 0.0F;
    float maintenance = 0.0F;
    float route_value = 0.0F;
    float roi = 0.0F;
    std::string reason;
};

[[nodiscard]] std::string ToString(RoutePurpose purpose);
[[nodiscard]] float RouteMaintenancePerTile(RouteKind kind);

} // namespace oikumene
