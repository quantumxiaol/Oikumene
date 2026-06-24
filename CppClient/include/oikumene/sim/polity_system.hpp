#pragma once

#include <vector>

#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/simulation_params.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

class PolitySystem {
public:
    static void Reset(World& world, std::vector<Settlement>& settlements, std::vector<Polity>& polities);
    static void UpdatePolities(World& world,
                               Turn turn,
                               std::vector<Settlement>& settlements,
                               std::vector<Polity>& polities,
                               EventLog& event_log);
    static void UpdatePolities(World& world,
                               const SimulationParams& params,
                               Turn turn,
                               std::vector<Settlement>& settlements,
                               std::vector<Polity>& polities,
                               EventLog& event_log);
};

}  // namespace oikumene
