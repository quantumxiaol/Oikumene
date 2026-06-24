#include "oikumene/sim/route_audit.hpp"

#include <algorithm>
#include <cmath>

#include "oikumene/sim/control_field.hpp"
#include "oikumene/sim/tech_effects.hpp"

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

bool ContainsMemberSettlement(const Polity& polity, int settlement_id) {
    return std::find(polity.member_settlement_ids.begin(), polity.member_settlement_ids.end(), settlement_id) !=
           polity.member_settlement_ids.end();
}

bool IsMineral(ResourceKind resource) {
    return resource == ResourceKind::Copper || resource == ResourceKind::Tin ||
           resource == ResourceKind::ShallowIron || resource == ResourceKind::ShallowCoal ||
           resource == ResourceKind::Gold || resource == ResourceKind::Silver ||
           resource == ResourceKind::MeteoricIron;
}

float EstimatedMineOutput(const Tile& tile) {
    return IsMineral(tile.resource) ? 0.45F + tile.resource_amount * 0.65F : 0.0F;
}

float RouteSavingFor(const World& world,
                     const std::vector<Settlement>& settlements,
                     const Route& route,
                     const Polity& polity) {
    const auto* from = SettlementById(settlements, route.from_settlement_id);
    if (from == nullptr) {
        return 0.0F;
    }
    const auto effects = ComputeTechEffects(polity.research);
    const float max_cost = 160.0F;
    const float without_route = TerrainPathCost(world, from->x, from->y, route.target_x, route.target_y, max_cost,
                                                effects.control_path_cost_multiplier,
                                                effects.coastal_control_cost_multiplier, polity.id, false);
    const float with_route = TerrainPathCost(world, from->x, from->y, route.target_x, route.target_y, max_cost,
                                             effects.control_path_cost_multiplier,
                                             effects.coastal_control_cost_multiplier, polity.id, true);
    if (!std::isfinite(without_route) || !std::isfinite(with_route)) {
        return 0.0F;
    }
    return std::max(0.0F, without_route - with_route);
}

void AddMineOutputStats(const World& world,
                        const std::vector<Settlement>& settlements,
                        const Polity& polity,
                        RouteNetworkStats& stats) {
    if (!HasTech(polity.research, TechId::Mining)) {
        return;
    }

    for (const auto& settlement : settlements) {
        if (!ContainsMemberSettlement(polity, settlement.id)) {
            continue;
        }
        for (const int index : settlement.worked_tile_indices) {
            if (index < 0 || index >= static_cast<int>(world.Tiles().size())) {
                continue;
            }
            const auto& tile = world.Tiles()[static_cast<std::size_t>(index)];
            if (tile.improvement != ImprovementKind::ShallowMine || !IsMineral(tile.resource)) {
                continue;
            }
            const float base_output = EstimatedMineOutput(tile);
            const bool connected = tile.has_route && tile.route_polity_id == polity.id;
            if (connected) {
                ++stats.active_connected_mines;
                stats.connected_ore_income += base_output * (1.18F + tile.route_quality * 0.28F);
            } else {
                stats.unconnected_ore_income += base_output;
            }
        }
    }
}

}  // namespace

RouteNetworkStats BuildRouteNetworkStats(const World& world, const std::vector<Route>& routes, const Polity& polity) {
    return BuildRouteNetworkStats(world, {}, routes, polity);
}

RouteNetworkStats BuildRouteNetworkStats(const World& world,
                                         const std::vector<Settlement>& settlements,
                                         const std::vector<Route>& routes,
                                         const Polity& polity) {
    RouteNetworkStats stats;
    for (const auto& route : routes) {
        if (route.polity_id != polity.id) {
            continue;
        }
        ++stats.route_count;
        stats.route_maintenance += route.maintenance;
        stats.admin_distance_saving += settlements.empty() ? 0.0F : RouteSavingFor(world, settlements, route, polity);
        if (route.to_settlement_id >= 0) {
            ++stats.connected_settlements;
        }
        if (route.purpose == RoutePurpose::Resource) {
            const auto& target = world.At(route.target_x, route.target_y);
            if (IsMineral(target.resource) || target.improvement == ImprovementKind::ShallowMine) {
                ++stats.connected_mine_potential;
            }
        }
    }
    stats.connected_mines = stats.connected_mine_potential;
    AddMineOutputStats(world, settlements, polity, stats);

    for (const auto& tile : world.Tiles()) {
        if (!tile.has_route || tile.route_polity_id != polity.id) {
            continue;
        }
        stats.road_tiles += tile.route_kind == RouteKind::Road ? 1 : 0;
        stats.trail_tiles += tile.route_kind == RouteKind::Trail ? 1 : 0;
    }
    return stats;
}

}  // namespace oikumene
