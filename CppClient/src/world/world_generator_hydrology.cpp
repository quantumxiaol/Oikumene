#include "oikumene/world/world_generator.hpp"

#include <algorithm>
#include <vector>

#include "world_generator_helpers.hpp"

namespace oikumene {

void WorldGenerator::GenerateRiversAndLakes() {
    const int tile_count = static_cast<int>(world_.Tiles().size());
    std::vector<int> downslope(static_cast<std::size_t>(tile_count), -1);
    std::vector<int> order(static_cast<std::size_t>(tile_count), 0);

    for (int index = 0; index < tile_count; ++index) {
        order[static_cast<std::size_t>(index)] = index;
    }

    for (auto& tile : world_.Tiles()) {
        tile.has_river = false;
        tile.river_flow = 0.0F;
        if (!tile.is_ocean) {
            tile.river_flow =
                0.35F + tile.rainfall * 1.25F + std::max(0.0F, tile.elevation - params_.sea_level) * 0.25F;
        }
    }

    for (const auto& tile : world_.Tiles()) {
        if (tile.is_ocean) {
            continue;
        }

        float best_score = tile.elevation - params_.lake_basin_threshold;
        int best_index = -1;
        for (const auto& [dx, dy] : world_generator::NeighborOffsets8()) {
            const int nx = tile.x + dx;
            const int ny = tile.y + dy;
            if (!world_.InBounds(nx, ny)) {
                continue;
            }

            const auto& neighbor = world_.At(nx, ny);
            float score = neighbor.elevation + Random01(nx, ny, 149) * 0.004F;
            if (neighbor.is_ocean) {
                score -= 0.22F;
            }

            if (score < best_score) {
                best_score = score;
                best_index = world_generator::IndexOf(world_, nx, ny);
            }
        }
        downslope[static_cast<std::size_t>(world_generator::IndexOf(world_, tile.x, tile.y))] = best_index;
    }

    std::sort(order.begin(), order.end(), [this](int lhs, int rhs) {
        return world_.Tiles()[static_cast<std::size_t>(lhs)].elevation >
               world_.Tiles()[static_cast<std::size_t>(rhs)].elevation;
    });

    int lake_tiles = 0;
    for (int index : order) {
        auto& tile = world_.Tiles()[static_cast<std::size_t>(index)];
        if (tile.is_ocean) {
            continue;
        }

        const int target = downslope[static_cast<std::size_t>(index)];
        if (target >= 0) {
            world_.Tiles()[static_cast<std::size_t>(target)].river_flow += tile.river_flow;
        } else if (lake_tiles < params_.max_lake_tiles && tile.elevation < params_.hill_level &&
                   tile.rainfall > 0.42F && !tile.is_coast && Random01(tile.x, tile.y, 157) > 0.52F) {
            tile.is_lake = true;
            tile.has_river = false;
            tile.river_flow = 0.0F;
            ++lake_tiles;
        }
    }

    for (auto& tile : world_.Tiles()) {
        if (tile.is_ocean || tile.is_lake) {
            tile.has_river = false;
            continue;
        }

        const float threshold =
            params_.river_flow_threshold + (tile.elevation < params_.sea_level + 0.08F ? 1.4F : 0.0F);
        tile.has_river = tile.river_flow >= threshold;
    }
}

} // namespace oikumene
