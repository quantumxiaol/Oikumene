#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/simulation_params.hpp"
#include "oikumene/sim/tech_effects.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene::settlement_system {

[[nodiscard]] float CarryingCapacityFor(const World& world, const Settlement& settlement, const TechEffects& effects);
[[nodiscard]] std::vector<int> SelectWorkedTiles(const World& world, const Settlement& settlement,
                                                 const SimulationParams& params, const TechEffects& effects);
void ProduceFromWorkedTile(World& world, Settlement& settlement, int index, const TechEffects& effects,
                           bool enable_routes, Turn turn, EventLog& event_log);

} // namespace oikumene::settlement_system
