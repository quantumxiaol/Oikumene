#include "oikumene/world/world_generator.hpp"

#include <algorithm>
#include <cmath>

#include "world_generator_helpers.hpp"

namespace oikumene {

void WorldGenerator::GenerateTemperature() {
    const float max_y = static_cast<float>(std::max(1, world_.Height() - 1));

    for (auto& tile : world_.Tiles()) {
        const float latitude = static_cast<float>(tile.y) / max_y;
        const float equator_warmth = 1.0F - std::abs(latitude * 2.0F - 1.0F);
        const float local_variation = (Noise2D(static_cast<float>(tile.x) / world_.Width(),
                                               static_cast<float>(tile.y) / world_.Height(), 59, 3.0F) -
                                       0.5F) *
                                      0.14F;
        tile.temperature =
            world_generator::Clamp01(0.10F + 0.84F * equator_warmth - 0.48F * tile.elevation + local_variation);
    }
}

void WorldGenerator::GenerateRainfall() {
    for (auto& tile : world_.Tiles()) {
        const float nx = static_cast<float>(tile.x) / static_cast<float>(world_.Width());
        const float ny = static_cast<float>(tile.y) / static_cast<float>(world_.Height());

        float ocean_bonus = tile.is_ocean ? 0.45F : 0.0F;
        for (int distance = 1; distance <= 14; ++distance) {
            const int west = tile.x - distance;
            const int east = tile.x + distance;
            if (world_.InBounds(west, tile.y) && world_.At(west, tile.y).is_ocean) {
                ocean_bonus += 0.30F * (1.0F - static_cast<float>(distance) / 15.0F);
                break;
            }
            if (world_.InBounds(east, tile.y) && world_.At(east, tile.y).is_ocean) {
                ocean_bonus += 0.16F * (1.0F - static_cast<float>(distance) / 15.0F);
                break;
            }
        }

        float rain_shadow = 0.0F;
        for (int distance = 1; distance <= 7; ++distance) {
            const int west = tile.x - distance;
            if (!world_.InBounds(west, tile.y)) {
                continue;
            }
            const auto& upwind = world_.At(west, tile.y);
            if (upwind.elevation > params_.hill_level) {
                rain_shadow += (upwind.elevation - params_.hill_level) * 0.16F;
            }
        }

        const float climate_noise = Noise2D(nx, ny, 71, 5.5F);
        const float thermal_lift = tile.temperature > 0.45F ? 0.08F : -0.05F;
        tile.rainfall =
            world_generator::Clamp01(0.16F + 0.42F * climate_noise + ocean_bonus + thermal_lift - rain_shadow);
    }
}

} // namespace oikumene
