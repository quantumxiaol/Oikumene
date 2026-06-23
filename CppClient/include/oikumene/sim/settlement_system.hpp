#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/simulation_params.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

[[nodiscard]] float LocalFoodOutput(const World& world, const Settlement& settlement, const SimulationParams& params);
[[nodiscard]] float LocalWoodOutput(const World& world, const Settlement& settlement, const SimulationParams& params);

class SettlementSystem {
public:
    static void UpdateSettlements(World& world,
                                  const SimulationParams& params,
                                  Turn turn,
                                  std::vector<Settlement>& settlements,
                                  EventLog& event_log);
};

}  // namespace oikumene
