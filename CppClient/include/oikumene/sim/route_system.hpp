#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/event_log.hpp"
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
    float route_maintenance = 0.0F;
    float admin_distance_saving = 0.0F;
};

[[nodiscard]] RouteNetworkStats BuildRouteNetworkStats(const World& world,
                                                       const std::vector<Route>& routes,
                                                       const Polity& polity);
[[nodiscard]] bool HasRouteTech(const Polity& polity);
[[nodiscard]] bool HasSailingTech(const Polity& polity);

class RouteSystem {
public:
    static void Reset(World& world, std::vector<Route>& routes, std::vector<Polity>& polities);
    static void UpdateRoutes(World& world,
                             Turn turn,
                             std::vector<Settlement>& settlements,
                             std::vector<Polity>& polities,
                             std::vector<Route>& routes,
                             EventLog& event_log);
};

}  // namespace oikumene
