#include "oikumene/sim/route_system.hpp"

#include <algorithm>
#include <sstream>

#include "oikumene/sim/route_planner.hpp"

namespace oikumene {
namespace {

const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id) {
    for (const auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

Settlement* MutableSettlementById(std::vector<Settlement>& settlements, int id) {
    for (auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

bool CanAffordRoute(const Polity& polity, const Settlement& capital, const RouteCandidate& candidate) {
    const bool enough_wood = capital.stockpile.wood >= candidate.build_cost_wood * 0.35F || polity.budget.wood_surplus > 0.15F;
    const bool enough_wealth = polity.budget.wealth_surplus >= -0.5F || candidate.build_cost_wealth <= 0.01F;
    return enough_wood && enough_wealth;
}

void ApplyRouteToTiles(World& world, const Route& route) {
    for (const auto& coord : route.path) {
        if (!world.InBounds(coord.x, coord.y)) {
            continue;
        }
        auto& tile = world.At(coord.x, coord.y);
        if (tile.is_ocean || tile.is_lake) {
            continue;
        }
        if (tile.has_route && tile.route_quality > RouteQuality(route.kind)) {
            continue;
        }
        tile.has_route = true;
        tile.route_kind = route.kind;
        tile.route_quality = RouteQuality(route.kind);
        tile.route_polity_id = route.polity_id;
        tile.route_id = route.id;
    }
}

int NextRouteId(const std::vector<Route>& routes) {
    int next = 0;
    for (const auto& route : routes) {
        next = std::max(next, route.id + 1);
    }
    return next;
}

void ChargeRouteBuildCost(std::vector<Settlement>& settlements, const Polity& polity, const Route& route) {
    auto* capital = MutableSettlementById(settlements, polity.capital_settlement_id);
    if (capital == nullptr) {
        return;
    }
    capital->stockpile.wood = std::max(0.0F, capital->stockpile.wood - route.build_cost_wood * 0.35F);
    capital->stockpile.wealth = std::max(0.0F, capital->stockpile.wealth - route.build_cost_wealth * 0.25F);
}

void AddRouteBuiltEvent(Turn turn, const Polity& polity, const Route& route, EventLog& event_log) {
    std::ostringstream summary;
    summary << polity.name << " built " << ToString(route.kind) << " route " << route.id << " to " << route.target_x
            << "," << route.target_y << " purpose " << ToString(route.purpose) << " roi " << route.roi
            << " reason " << route.reason;
    event_log.Add(SimEvent{
        .turn = turn,
        .type = EventType::RouteBuilt,
        .actor_id = route.from_settlement_id,
        .x = route.target_x,
        .y = route.target_y,
        .summary = summary.str(),
    });
}

void RefreshPolityRouteStats(const World& world, const std::vector<Route>& routes, std::vector<Polity>& polities) {
    for (auto& polity : polities) {
        const auto stats = BuildRouteNetworkStats(world, routes, polity);
        polity.route_ids.clear();
        for (const auto& route : routes) {
            if (route.polity_id == polity.id) {
                polity.route_ids.push_back(route.id);
            }
        }
        polity.route_maintenance = stats.route_maintenance;
        polity.connected_settlements = stats.connected_settlements;
        polity.connected_mines = stats.connected_mines;
        polity.admin_distance_saving = stats.admin_distance_saving;
    }
}

}  // namespace

RouteNetworkStats BuildRouteNetworkStats(const World& world, const std::vector<Route>& routes, const Polity& polity) {
    RouteNetworkStats stats;
    for (const auto& route : routes) {
        if (route.polity_id != polity.id) {
            continue;
        }
        ++stats.route_count;
        stats.route_maintenance += route.maintenance;
        stats.admin_distance_saving += route.path.empty() ? 0.0F : route.route_value * 0.08F;
        if (route.to_settlement_id >= 0) {
            ++stats.connected_settlements;
        }
        if (route.purpose == RoutePurpose::Resource) {
            ++stats.connected_mines;
        }
    }
    for (const auto& tile : world.Tiles()) {
        if (!tile.has_route || tile.route_polity_id != polity.id) {
            continue;
        }
        stats.road_tiles += tile.route_kind == RouteKind::Road ? 1 : 0;
        stats.trail_tiles += tile.route_kind == RouteKind::Trail ? 1 : 0;
    }
    return stats;
}

bool HasRouteTech(const Polity& polity) {
    return HasTech(polity.research, TechId::Roads);
}

bool HasSailingTech(const Polity& polity) {
    return HasTech(polity.research, TechId::Sailing);
}

void RouteSystem::Reset(World& world, std::vector<Route>& routes, std::vector<Polity>& polities) {
    routes.clear();
    for (auto& tile : world.Tiles()) {
        tile.has_route = false;
        tile.route_kind = RouteKind::Trail;
        tile.route_quality = 0.0F;
        tile.route_polity_id = kInvalidPolityId;
        tile.route_id = -1;
    }
    for (auto& polity : polities) {
        polity.route_ids.clear();
        polity.route_maintenance = 0.0F;
        polity.connected_settlements = 0;
        polity.connected_mines = 0;
        polity.admin_distance_saving = 0.0F;
    }
}

void RouteSystem::UpdateRoutes(World& world,
                               Turn turn,
                               std::vector<Settlement>& settlements,
                               std::vector<Polity>& polities,
                               std::vector<Route>& routes,
                               EventLog& event_log) {
    if (polities.empty()) {
        return;
    }

    const bool build_turn = turn > 0 && turn % 20 == 0;
    for (auto& polity : polities) {
        if (!build_turn || polity.stability < 0.45F) {
            continue;
        }
        const auto* capital = SettlementById(settlements, polity.capital_settlement_id);
        if (capital == nullptr) {
            continue;
        }

        auto candidates = PlanRouteCandidates(world, settlements, routes, polity);
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                                        [&](const RouteCandidate& candidate) {
                                            return !candidate.path.reachable ||
                                                   candidate.roi < RouteRoiThreshold(candidate.kind) ||
                                                   !CanAffordRoute(polity, *capital, candidate);
                                        }),
                         candidates.end());
        if (candidates.empty()) {
            continue;
        }

        const auto best = std::max_element(candidates.begin(), candidates.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.roi < rhs.roi;
        });
        const int route_id = NextRouteId(routes);
        routes.push_back(BuildRouteFromCandidate(route_id, polity.id, *best));
        auto& route = routes.back();
        ApplyRouteToTiles(world, route);
        ChargeRouteBuildCost(settlements, polity, route);
        AddRouteBuiltEvent(turn, polity, route, event_log);
    }

    RefreshPolityRouteStats(world, routes, polities);
}

}  // namespace oikumene
