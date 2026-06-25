#include "oikumene/world/world_generator.hpp"

#include <algorithm>
#include <cmath>

#include "world_generator_helpers.hpp"

namespace oikumene {

void WorldGenerator::GenerateBiomes() {
    for (auto& tile : world_.Tiles()) {
        const bool water = IsAdjacentToWater(tile.x, tile.y) || tile.has_river || tile.is_lake || tile.is_coast;
        const float rain_fit = 1.0F - std::abs(tile.rainfall - 0.58F) / 0.58F;
        const float temp_fit = 1.0F - std::abs(tile.temperature - 0.58F) / 0.58F;
        tile.fertility = world_generator::Clamp01(0.52F * rain_fit + 0.36F * temp_fit + (water ? 0.18F : 0.0F));

        if (tile.is_ocean) {
            tile.biome = Biome::Ocean;
            tile.fertility = 0.0F;
            continue;
        }
        if (tile.is_lake) {
            tile.biome = Biome::Lake;
            tile.fertility = 0.25F;
            continue;
        }
        if (tile.is_coast) {
            tile.biome = Biome::Coast;
            tile.fertility = world_generator::Clamp01(tile.fertility + 0.08F);
            continue;
        }
        if (tile.elevation >= params_.mountain_level) {
            tile.biome = tile.temperature < 0.30F ? Biome::Snow : Biome::Mountain;
            tile.fertility *= 0.20F;
            continue;
        }
        if (tile.temperature < 0.15F) {
            tile.biome = Biome::Snow;
            tile.fertility *= 0.15F;
            continue;
        }
        if (tile.temperature < 0.28F) {
            tile.biome = Biome::Tundra;
            tile.fertility *= 0.35F;
            continue;
        }
        if (tile.rainfall < 0.20F) {
            tile.biome = Biome::Desert;
            tile.fertility *= 0.18F;
            continue;
        }
        if (tile.elevation >= params_.hill_level && tile.rainfall < 0.32F) {
            tile.biome = Biome::Hill;
            tile.fertility *= 0.55F;
            continue;
        }
        if (tile.rainfall > 0.76F && tile.temperature > 0.62F) {
            tile.biome = Biome::Rainforest;
            tile.fertility = world_generator::Clamp01(tile.fertility + 0.08F);
            continue;
        }
        if (tile.rainfall > 0.78F && water) {
            tile.biome = Biome::Wetland;
            tile.fertility = world_generator::Clamp01(tile.fertility + 0.05F);
            continue;
        }
        if (tile.rainfall > 0.47F) {
            tile.biome = Biome::Forest;
            tile.fertility = world_generator::Clamp01(tile.fertility + 0.04F);
            if (tile.elevation >= params_.hill_level) {
                tile.fertility *= 0.82F;
            }
            continue;
        }
        if (tile.elevation >= params_.hill_level) {
            tile.biome = Biome::Hill;
            tile.fertility *= 0.65F;
            continue;
        }

        tile.biome = Biome::Grassland;
        if (tile.has_river) {
            tile.fertility = world_generator::Clamp01(tile.fertility + 0.12F);
        }
    }
}

void WorldGenerator::ComputeMovementCost() {
    for (auto& tile : world_.Tiles()) {
        tile.defense_bonus = 0.0F;

        switch (tile.biome) {
        case Biome::Ocean:
        case Biome::Lake:
            tile.movement_cost = 999.0F;
            break;
        case Biome::Coast:
            tile.movement_cost = 1.25F;
            break;
        case Biome::River:
            tile.movement_cost = 1.10F;
            break;
        case Biome::Grassland:
            tile.movement_cost = 1.0F;
            break;
        case Biome::Forest:
        case Biome::Rainforest:
            tile.movement_cost = 2.0F;
            tile.defense_bonus = 0.18F;
            break;
        case Biome::Wetland:
            tile.movement_cost = 2.7F;
            tile.defense_bonus = 0.12F;
            break;
        case Biome::Desert:
            tile.movement_cost = 2.35F;
            break;
        case Biome::Tundra:
            tile.movement_cost = 2.2F;
            break;
        case Biome::Snow:
            tile.movement_cost = 3.2F;
            tile.defense_bonus = 0.24F;
            break;
        case Biome::Hill:
            tile.movement_cost = 2.05F;
            tile.defense_bonus = 0.28F;
            break;
        case Biome::Mountain:
            tile.movement_cost = 5.2F;
            tile.defense_bonus = 0.45F;
            break;
        }
    }
}

void WorldGenerator::ComputeSettlementScore() {
    for (auto& tile : world_.Tiles()) {
        if (!world_generator::IsLand(tile) || tile.biome == Biome::Mountain || tile.biome == Biome::Snow) {
            tile.settlement_score = 0.0F;
            continue;
        }

        float score = tile.fertility * 0.46F;
        if (IsAdjacentToWater(tile.x, tile.y) || tile.has_river || tile.is_coast) {
            score += 0.24F;
        }
        if (tile.biome == Biome::Grassland || tile.biome == Biome::Coast || tile.biome == Biome::River) {
            score += 0.11F;
        }
        if (HasNeighborBiome(tile.x, tile.y, Biome::Forest) || HasNeighborBiome(tile.x, tile.y, Biome::Rainforest)) {
            score += 0.09F;
        }
        if (world_generator::IsMineral(tile.resource)) {
            score += 0.08F;
        }
        if (tile.resource == ResourceKind::Horse || tile.resource == ResourceKind::Wood ||
            tile.resource == ResourceKind::Bamboo) {
            score += 0.05F;
        }
        score += tile.defense_bonus * 0.10F;

        if (tile.biome == Biome::Desert || tile.biome == Biome::Tundra || tile.biome == Biome::Wetland) {
            score -= 0.17F;
        }
        if (tile.movement_cost > 2.5F) {
            score -= 0.08F;
        }

        tile.settlement_score = world_generator::Clamp01(score);
    }
}

} // namespace oikumene
