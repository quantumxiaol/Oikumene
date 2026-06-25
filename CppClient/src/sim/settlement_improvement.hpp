#pragma once

#include "oikumene/core/types.hpp"
#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/simulation_params.hpp"
#include "oikumene/sim/tech_effects.hpp"
#include "oikumene/world/improvement.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene::settlement_system {

[[nodiscard]] int CountImprovement(const World& world, const Settlement& settlement, ImprovementKind kind);
[[nodiscard]] ImprovementKind DesiredImprovement(const World& world, const Settlement& settlement,
                                                 const SimulationParams& params, const TechEffects& effects);
[[nodiscard]] bool BuildBestImprovement(World& world, Settlement& settlement, ImprovementKind desired, Turn turn,
                                        EventLog& event_log);

} // namespace oikumene::settlement_system
