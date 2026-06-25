#pragma once

#include <vector>

#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/simulation_params.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene::settlement_system {

[[nodiscard]] bool IsWorkableLand(const Tile& tile);
[[nodiscard]] bool IsMineral(ResourceKind resource);
[[nodiscard]] int TileIndex(const World& world, int x, int y);
[[nodiscard]] Tile& TileByIndex(World& world, int index);
[[nodiscard]] const Tile& TileByIndex(const World& world, int index);
[[nodiscard]] int WorkRadiusFor(const Settlement& settlement);
[[nodiscard]] int WorkSlotsFor(const Settlement& settlement, const SimulationParams& params);
[[nodiscard]] std::vector<int> NearbyTileIndices(const World& world, const Settlement& settlement, int radius);

} // namespace oikumene::settlement_system
