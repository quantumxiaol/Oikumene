#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/band.hpp"
#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/simulation_params.hpp"
#include "oikumene/world/tile.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

[[nodiscard]] bool IsValidBandTile(const Tile& tile);
[[nodiscard]] bool IsValidSettlementTile(const Tile& tile);
[[nodiscard]] float ForageYield(const Tile& tile);
[[nodiscard]] bool IsSettlementNearby(const std::vector<Settlement>& settlements, int x, int y, int radius);

class BandSystem {
  public:
    static void InitializeBands(const World& world, const SimulationParams& params, int count,
                                std::vector<Band>& bands);

    static void UpdateBands(const World& world, const SimulationParams& params, Turn turn, std::vector<Band>& bands,
                            std::vector<Settlement>& settlements, EventLog& event_log);
};

} // namespace oikumene
