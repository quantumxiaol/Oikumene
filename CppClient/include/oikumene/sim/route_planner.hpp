#pragma once

#include <string>
#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/pathfinding.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/route.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

struct RouteCandidate {
    int from_settlement_id = -1;
    int to_settlement_id = -1;
    int target_x = 0;
    int target_y = 0;
    RoutePurpose purpose = RoutePurpose::Admin;
    RouteKind kind = RouteKind::Trail;
    PathResult path;
    float build_cost_wood = 0.0F;
    float build_cost_wealth = 0.0F;
    float maintenance = 0.0F;
    float route_value = 0.0F;
    float roi = 0.0F;
    std::string reason;
};

[[nodiscard]] float RouteRoiThreshold(RouteKind kind);
[[nodiscard]] std::vector<RouteCandidate> PlanRouteCandidates(const World& world,
                                                              const std::vector<Settlement>& settlements,
                                                              const std::vector<Route>& routes, const Polity& polity);
[[nodiscard]] Route BuildRouteFromCandidate(int id, PolityId polity_id, const RouteCandidate& candidate);

} // namespace oikumene
