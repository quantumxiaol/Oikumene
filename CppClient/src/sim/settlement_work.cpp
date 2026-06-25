#include "settlement_work.hpp"

#include <algorithm>

namespace oikumene::settlement_system {

bool IsWorkableLand(const Tile& tile) {
    return !tile.is_ocean && !tile.is_lake && tile.biome != Biome::Mountain && tile.biome != Biome::Snow;
}

bool IsMineral(ResourceKind resource) {
    return resource == ResourceKind::Copper || resource == ResourceKind::Tin || resource == ResourceKind::ShallowIron ||
           resource == ResourceKind::ShallowCoal || resource == ResourceKind::Gold ||
           resource == ResourceKind::Silver || resource == ResourceKind::MeteoricIron;
}

int TileIndex(const World& world, int x, int y) {
    return y * world.Width() + x;
}

Tile& TileByIndex(World& world, int index) {
    return world.Tiles()[static_cast<std::size_t>(index)];
}

const Tile& TileByIndex(const World& world, int index) {
    return world.Tiles()[static_cast<std::size_t>(index)];
}

int WorkRadiusFor(const Settlement& settlement) {
    return settlement.level == SettlementLevel::Village ? 3 : 2;
}

int WorkSlotsFor(const Settlement& settlement, const SimulationParams& params) {
    return std::max(1, settlement.population / std::max(1, params.population_per_work_slot));
}

std::vector<int> NearbyTileIndices(const World& world, const Settlement& settlement, int radius) {
    std::vector<int> indices;
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            const int x = settlement.x + dx;
            const int y = settlement.y + dy;
            if (!world.InBounds(x, y)) {
                continue;
            }
            const auto& tile = world.At(x, y);
            if (IsWorkableLand(tile)) {
                indices.push_back(TileIndex(world, x, y));
            }
        }
    }
    return indices;
}

} // namespace oikumene::settlement_system
