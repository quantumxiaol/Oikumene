#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/route.hpp"
#include "oikumene/sim/route_audit.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

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
                             EventLog& event_log,
                             bool enable_routes = true);
};

}  // namespace oikumene
