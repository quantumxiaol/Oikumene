#include "oikumene/sim/route_planner.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

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

bool IsMineral(ResourceKind resource) {
    return resource == ResourceKind::Copper || resource == ResourceKind::Tin || resource == ResourceKind::ShallowIron ||
           resource == ResourceKind::ShallowCoal || resource == ResourceKind::Gold ||
           resource == ResourceKind::Silver || resource == ResourceKind::MeteoricIron;
}

bool RouteAlreadyTargets(const std::vector<Route>& routes, PolityId polity_id, int x, int y, RoutePurpose purpose) {
    return std::any_of(routes.begin(), routes.end(), [&](const Route& route) {
        return route.polity_id == polity_id && route.target_x == x && route.target_y == y && route.purpose == purpose;
    });
}

std::vector<int> NearbyTileIndices(const World& world, const Settlement& settlement, int radius = 3) {
    std::vector<int> indices;
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            const int x = settlement.x + dx;
            const int y = settlement.y + dy;
            if (world.InBounds(x, y)) {
                indices.push_back(y * world.Width() + x);
            }
        }
    }
    return indices;
}

RouteKind ChooseRouteKind(const Polity& polity, const Tile& target) {
    if (target.is_coast && HasTech(polity.research, TechId::Sailing)) {
        return RouteKind::CoastalRoute;
    }
    if (target.has_river) {
        return RouteKind::RiverRoute;
    }
    return HasTech(polity.research, TechId::Roads) ? RouteKind::Road : RouteKind::Trail;
}

float RouteBuildCostMultiplier(RouteKind kind) {
    switch (kind) {
    case RouteKind::Trail:
        return 0.10F;
    case RouteKind::Road:
        return 0.34F;
    case RouteKind::RiverRoute:
        return 0.18F;
    case RouteKind::CoastalRoute:
        return 0.22F;
    }
    return 0.20F;
}

float ResourceValueFor(const Tile& tile, const Polity& polity) {
    if (!IsMineral(tile.resource)) {
        return 0.0F;
    }
    float value = 12.0F + tile.resource_amount * 18.0F;
    if (HasTech(polity.research, TechId::Mining)) {
        value *= 1.65F;
    }
    return value;
}

float AdminValueFor(const PathResult& path, const Polity& polity, RoutePurpose purpose) {
    const float base = purpose == RoutePurpose::SettlementLink || purpose == RoutePurpose::Admin ? 7.0F : 2.5F;
    const float distance_value = std::min(18.0F, path.total_cost * 0.35F);
    const float overextension_value = polity.overextension > 0.0F ? 8.0F * std::min(1.0F, polity.overextension) : 0.0F;
    return base + distance_value + overextension_value;
}

RouteCandidate MakeCandidate(const World& world, const Polity& polity, const Settlement& from, int target_x,
                             int target_y, int to_settlement_id, RoutePurpose purpose, std::string reason) {
    const auto& target = world.At(target_x, target_y);
    const RouteKind kind = ChooseRouteKind(polity, target);
    PathOptions options;
    options.allow_ocean = kind == RouteKind::CoastalRoute && HasTech(polity.research, TechId::Sailing);
    options.use_existing_routes = true;
    options.route_polity_id = polity.id;
    options.max_cost = 90.0F;

    PathResult path =
        FindPath(world, TileCoord{.x = from.x, .y = from.y}, TileCoord{.x = target_x, .y = target_y}, options);
    if (!path.reachable || path.path.size() < 2) {
        return {};
    }

    RouteCandidate candidate;
    candidate.from_settlement_id = from.id;
    candidate.to_settlement_id = to_settlement_id;
    candidate.target_x = target_x;
    candidate.target_y = target_y;
    candidate.purpose = purpose;
    candidate.kind = kind;
    candidate.path = std::move(path);
    candidate.build_cost_wood = candidate.path.total_cost * RouteBuildCostMultiplier(kind);
    candidate.build_cost_wealth = kind == RouteKind::Trail ? 0.0F : candidate.path.total_cost * 0.045F;
    candidate.maintenance = static_cast<float>(candidate.path.path.size()) * RouteMaintenancePerTile(kind);
    const float admin_value = AdminValueFor(candidate.path, polity, purpose);
    const float resource_value = purpose == RoutePurpose::Resource ? ResourceValueFor(target, polity) : 0.0F;
    const float control_value = static_cast<float>(candidate.path.path.size()) * 0.22F;
    const float future_trade_value = target.is_coast ? 4.0F : 0.0F;
    candidate.route_value = admin_value + resource_value + control_value + future_trade_value;
    candidate.roi = candidate.route_value / std::max(1.0F, candidate.build_cost_wood + candidate.build_cost_wealth +
                                                               candidate.maintenance * 20.0F);
    candidate.reason = std::move(reason);
    return candidate;
}

void AddSettlementLinkCandidates(const World& world, const std::vector<Settlement>& settlements,
                                 const std::vector<Route>& routes, const Polity& polity,
                                 std::vector<RouteCandidate>& candidates) {
    const auto* capital = SettlementById(settlements, polity.capital_settlement_id);
    if (capital == nullptr) {
        return;
    }
    for (const int settlement_id : polity.member_settlement_ids) {
        if (settlement_id == polity.capital_settlement_id) {
            continue;
        }
        const auto* settlement = SettlementById(settlements, settlement_id);
        if (settlement == nullptr ||
            RouteAlreadyTargets(routes, polity.id, settlement->x, settlement->y, RoutePurpose::SettlementLink)) {
            continue;
        }
        candidates.push_back(MakeCandidate(world, polity, *capital, settlement->x, settlement->y, settlement->id,
                                           RoutePurpose::SettlementLink, "capital-member settlement link"));
    }
}

void AddResourceCandidates(const World& world, const std::vector<Settlement>& settlements,
                           const std::vector<Route>& routes, const Polity& polity,
                           std::vector<RouteCandidate>& candidates) {
    for (const int settlement_id : polity.member_settlement_ids) {
        const auto* settlement = SettlementById(settlements, settlement_id);
        if (settlement == nullptr) {
            continue;
        }
        for (const int index : NearbyTileIndices(world, *settlement)) {
            const auto& tile = world.Tiles()[static_cast<std::size_t>(index)];
            if (!IsMineral(tile.resource) && tile.improvement != ImprovementKind::ShallowMine) {
                continue;
            }
            if (RouteAlreadyTargets(routes, polity.id, tile.x, tile.y, RoutePurpose::Resource)) {
                continue;
            }
            candidates.push_back(MakeCandidate(world, polity, *settlement, tile.x, tile.y, -1, RoutePurpose::Resource,
                                               "resource/mine access"));
        }
    }
}

} // namespace

float RouteRoiThreshold(RouteKind kind) {
    return kind == RouteKind::Trail ? 0.55F : 0.72F;
}

std::vector<RouteCandidate> PlanRouteCandidates(const World& world, const std::vector<Settlement>& settlements,
                                                const std::vector<Route>& routes, const Polity& polity) {
    std::vector<RouteCandidate> candidates;
    AddSettlementLinkCandidates(world, settlements, routes, polity, candidates);
    AddResourceCandidates(world, settlements, routes, polity, candidates);
    return candidates;
}

Route BuildRouteFromCandidate(int id, PolityId polity_id, const RouteCandidate& candidate) {
    return Route{
        .id = id,
        .polity_id = polity_id,
        .from_settlement_id = candidate.from_settlement_id,
        .to_settlement_id = candidate.to_settlement_id,
        .target_x = candidate.target_x,
        .target_y = candidate.target_y,
        .purpose = candidate.purpose,
        .kind = candidate.kind,
        .path = candidate.path.path,
        .build_cost_wood = candidate.build_cost_wood,
        .build_cost_wealth = candidate.build_cost_wealth,
        .maintenance = candidate.maintenance,
        .route_value = candidate.route_value,
        .roi = candidate.roi,
        .reason = candidate.reason,
    };
}

} // namespace oikumene
