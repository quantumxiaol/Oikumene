#include "oikumene/world/world_generator.hpp"

#include <algorithm>
#include <cmath>

#include "world_generator_helpers.hpp"

namespace oikumene {

void WorldGenerator::GenerateElevation() {
    const float width = static_cast<float>(world_.Width());
    const float height = static_cast<float>(world_.Height());
    const float min_side = std::min(width, height);

    for (auto& tile : world_.Tiles()) {
        const float nx = static_cast<float>(tile.x) / std::max(1.0F, width - 1.0F);
        const float ny = static_cast<float>(tile.y) / std::max(1.0F, height - 1.0F);

        const float continental = Noise2D(nx, ny, 11, 1.65F);
        const float medium = Noise2D(nx, ny, 23, 4.25F);
        const float detail = Noise2D(nx, ny, 37, 10.0F);
        const float ridge_source = Noise2D(nx, ny, 43, 5.1F);
        const float ridges = std::pow(std::abs(ridge_source - 0.5F) * 2.0F, 1.7F);

        const float edge_distance =
            static_cast<float>(std::min({tile.x, tile.y, world_.Width() - 1 - tile.x, world_.Height() - 1 - tile.y}));
        const float edge_mask = world_generator::Smooth(std::min(1.0F, edge_distance / (min_side * 0.20F)));

        float elevation = 0.42F * continental + 0.27F * medium + 0.13F * detail + 0.26F * ridges;
        elevation = elevation * (0.62F + 0.38F * edge_mask) + 0.08F * continental - 0.12F * (1.0F - edge_mask);
        tile.elevation = world_generator::Clamp01(elevation);
    }
}

void WorldGenerator::ClassifyOceanAndLand() {
    for (auto& tile : world_.Tiles()) {
        tile.is_ocean = tile.elevation < params_.sea_level;
        tile.is_lake = false;
        tile.has_river = false;
        tile.is_coast = false;
        tile.river_flow = 0.0F;
        tile.biome = tile.is_ocean ? Biome::Ocean : Biome::Grassland;
    }

    for (auto& tile : world_.Tiles()) {
        if (tile.is_ocean) {
            continue;
        }

        for (const auto& [dx, dy] : world_generator::NeighborOffsets8()) {
            const int nx = tile.x + dx;
            const int ny = tile.y + dy;
            if (world_.InBounds(nx, ny) && world_.At(nx, ny).is_ocean) {
                tile.is_coast = true;
                break;
            }
        }
    }
}

} // namespace oikumene
