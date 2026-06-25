#include "oikumene/world/world_generator.hpp"

#include "world_generator_helpers.hpp"

namespace oikumene {

void WorldGenerator::GenerateResources() {
    for (auto& tile : world_.Tiles()) {
        tile.resource = ResourceKind::None;
        tile.resource_amount = 0.0F;
        if (!world_generator::IsLand(tile)) {
            continue;
        }

        const float geology = Noise2D(static_cast<float>(tile.x) / world_.Width(),
                                      static_cast<float>(tile.y) / world_.Height(), 127, 9.0F);
        const float roll = Random01(tile.x, tile.y, 131);
        const bool highland = tile.biome == Biome::Hill || tile.biome == Biome::Mountain || tile.biome == Biome::Snow;
        const bool near_water = IsAdjacentToWater(tile.x, tile.y);

        if (tile.biome == Biome::Forest && roll > 0.42F) {
            tile.resource = ResourceKind::Wood;
        } else if (tile.biome == Biome::Rainforest && roll > 0.34F) {
            tile.resource = tile.temperature > 0.68F && tile.rainfall > 0.72F && geology > 0.44F ? ResourceKind::Bamboo
                                                                                                 : ResourceKind::Wood;
        } else if (tile.biome == Biome::Grassland && roll > 0.90F) {
            tile.resource = ResourceKind::Horse;
        } else if (highland && geology > 0.82F && roll > 0.55F) {
            tile.resource = ResourceKind::Copper;
        } else if (highland && geology > 0.74F && roll > 0.72F) {
            tile.resource = ResourceKind::Tin;
        } else if (highland && geology < 0.18F && roll > 0.52F) {
            tile.resource = ResourceKind::ShallowIron;
        } else if ((tile.biome == Biome::Hill || tile.biome == Biome::Forest) && geology < 0.26F && roll > 0.79F) {
            tile.resource = ResourceKind::ShallowCoal;
        } else if (highland && near_water && roll > 0.95F) {
            tile.resource = ResourceKind::Gold;
        } else if (highland && roll > 0.972F) {
            tile.resource = ResourceKind::Silver;
        } else if (tile.is_coast && tile.rainfall < 0.34F && roll > 0.86F) {
            tile.resource = ResourceKind::Salt;
        } else if (near_water && roll > 0.88F) {
            tile.resource = ResourceKind::Clay;
        } else if (highland && roll > 0.84F) {
            tile.resource = ResourceKind::Stone;
        } else if (roll > 0.9992F) {
            tile.resource = ResourceKind::MeteoricIron;
        }

        if (tile.resource != ResourceKind::None) {
            tile.resource_amount =
                world_generator::Clamp01(0.35F + geology * 0.45F + Random01(tile.x, tile.y, 137) * 0.35F);
        }
    }

    for (auto& tile : world_.Tiles()) {
        tile.improvement = ImprovementKind::None;
        tile.worked_by_settlement_id = -1;
        tile.improvement_progress = 0.0F;
        tile.soil_quality = world_generator::IsLand(tile)
                                ? world_generator::Clamp01(tile.fertility * 0.78F + tile.rainfall * 0.14F +
                                                           (tile.has_river || tile.is_coast ? 0.10F : 0.0F))
                                : 0.0F;
        tile.forest_cover = 0.0F;
        if (tile.biome == Biome::Forest) {
            tile.forest_cover = world_generator::Clamp01(0.62F + tile.rainfall * 0.28F);
        } else if (tile.biome == Biome::Rainforest) {
            tile.forest_cover = world_generator::Clamp01(0.82F + tile.rainfall * 0.16F);
        } else if (tile.resource == ResourceKind::Wood || tile.resource == ResourceKind::Bamboo) {
            tile.forest_cover = 0.58F;
        }
    }
}

} // namespace oikumene
