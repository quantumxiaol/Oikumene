#pragma once

#include <vector>

#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/route.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

struct RouteNetworkStats {
    int route_count = 0;
    int road_tiles = 0;
    int trail_tiles = 0;
    int connected_settlements = 0;
    int connected_mines = 0;
    int connected_mine_potential = 0;
    int active_connected_mines = 0;
    float connected_ore_income = 0.0F;
    float unconnected_ore_income = 0.0F;
    float route_maintenance = 0.0F;
    float admin_distance_saving = 0.0F;
};

[[nodiscard]] RouteNetworkStats BuildRouteNetworkStats(const World& world, const std::vector<Route>& routes,
                                                       const Polity& polity);
[[nodiscard]] RouteNetworkStats BuildRouteNetworkStats(const World& world, const std::vector<Settlement>& settlements,
                                                       const std::vector<Route>& routes, const Polity& polity);

} // namespace oikumene
