#include "oikumene/world/world_generator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace oikumene {
namespace {

[[nodiscard]] float Clamp01(float value) {
    return std::clamp(value, 0.0F, 1.0F);
}

[[nodiscard]] float Smooth(float value) {
    value = Clamp01(value);
    return value * value * (3.0F - 2.0F * value);
}

[[nodiscard]] float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

[[nodiscard]] std::uint64_t Mix(std::uint64_t value) {
    value ^= value >> 30U;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27U;
    value *= 0x94d049bb133111ebULL;
    value ^= value >> 31U;
    return value;
}

[[nodiscard]] int IndexOf(const World& world, int x, int y) {
    return y * world.Width() + x;
}

[[nodiscard]] std::array<std::pair<int, int>, 8> NeighborOffsets8() {
    return {{{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}}};
}

[[nodiscard]] std::array<std::pair<int, int>, 4> NeighborOffsets4() {
    return {{{0, -1}, {-1, 0}, {1, 0}, {0, 1}}};
}

[[nodiscard]] bool IsLand(const Tile& tile) {
    return !tile.is_ocean && !tile.is_lake;
}

[[nodiscard]] bool IsMineral(ResourceKind resource) {
    return resource == ResourceKind::Copper || resource == ResourceKind::Tin ||
           resource == ResourceKind::ShallowIron || resource == ResourceKind::ShallowCoal ||
           resource == ResourceKind::Gold || resource == ResourceKind::Silver ||
           resource == ResourceKind::MeteoricIron;
}

}  // namespace

World WorldGenerator::Generate(const WorldGenerationParams& params) {
    return WorldGenerator(params).Run();
}

WorldGenerator::WorldGenerator(const WorldGenerationParams& params)
    : params_(params), world_(params.width, params.height, params.seed), rng_(params.seed) {}

World WorldGenerator::Run() {
    GenerateElevation();
    ClassifyOceanAndLand();
    GenerateTemperature();
    GenerateRainfall();
    GenerateRiversAndLakes();
    GenerateBiomes();
    GenerateResources();
    ComputeMovementCost();
    ComputeSettlementScore();
    return world_;
}

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

        const float edge_distance = static_cast<float>(
            std::min({tile.x, tile.y, world_.Width() - 1 - tile.x, world_.Height() - 1 - tile.y}));
        const float edge_mask = Smooth(std::min(1.0F, edge_distance / (min_side * 0.20F)));

        float elevation = 0.42F * continental + 0.27F * medium + 0.13F * detail + 0.26F * ridges;
        elevation = elevation * (0.62F + 0.38F * edge_mask) + 0.08F * continental - 0.12F * (1.0F - edge_mask);
        tile.elevation = Clamp01(elevation);
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

        for (const auto& [dx, dy] : NeighborOffsets8()) {
            const int nx = tile.x + dx;
            const int ny = tile.y + dy;
            if (world_.InBounds(nx, ny) && world_.At(nx, ny).is_ocean) {
                tile.is_coast = true;
                break;
            }
        }
    }
}

void WorldGenerator::GenerateTemperature() {
    const float max_y = static_cast<float>(std::max(1, world_.Height() - 1));

    for (auto& tile : world_.Tiles()) {
        const float latitude = static_cast<float>(tile.y) / max_y;
        const float equator_warmth = 1.0F - std::abs(latitude * 2.0F - 1.0F);
        const float local_variation = (Noise2D(static_cast<float>(tile.x) / world_.Width(),
                                               static_cast<float>(tile.y) / world_.Height(), 59, 3.0F) -
                                       0.5F) *
                                      0.14F;
        tile.temperature = Clamp01(0.10F + 0.84F * equator_warmth - 0.48F * tile.elevation + local_variation);
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
        tile.rainfall = Clamp01(0.16F + 0.42F * climate_noise + ocean_bonus + thermal_lift - rain_shadow);
    }
}

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
            tile.river_flow = 0.35F + tile.rainfall * 1.25F + std::max(0.0F, tile.elevation - params_.sea_level) * 0.25F;
        }
    }

    for (const auto& tile : world_.Tiles()) {
        if (tile.is_ocean) {
            continue;
        }

        float best_score = tile.elevation - params_.lake_basin_threshold;
        int best_index = -1;
        for (const auto& [dx, dy] : NeighborOffsets8()) {
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
                best_index = IndexOf(world_, nx, ny);
            }
        }
        downslope[static_cast<std::size_t>(IndexOf(world_, tile.x, tile.y))] = best_index;
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
                   tile.rainfall > 0.42F && !tile.is_coast &&
                   Random01(tile.x, tile.y, 157) > 0.52F) {
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

        const float threshold = params_.river_flow_threshold + (tile.elevation < params_.sea_level + 0.08F ? 1.4F : 0.0F);
        tile.has_river = tile.river_flow >= threshold;
    }
}

void WorldGenerator::GenerateBiomes() {
    for (auto& tile : world_.Tiles()) {
        const bool water = IsAdjacentToWater(tile.x, tile.y) || tile.has_river || tile.is_lake || tile.is_coast;
        const float rain_fit = 1.0F - std::abs(tile.rainfall - 0.58F) / 0.58F;
        const float temp_fit = 1.0F - std::abs(tile.temperature - 0.58F) / 0.58F;
        tile.fertility = Clamp01(0.52F * rain_fit + 0.36F * temp_fit + (water ? 0.18F : 0.0F));

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
            tile.fertility = Clamp01(tile.fertility + 0.08F);
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
            tile.fertility = Clamp01(tile.fertility + 0.08F);
            continue;
        }
        if (tile.rainfall > 0.78F && water) {
            tile.biome = Biome::Wetland;
            tile.fertility = Clamp01(tile.fertility + 0.05F);
            continue;
        }
        if (tile.rainfall > 0.47F) {
            tile.biome = Biome::Forest;
            tile.fertility = Clamp01(tile.fertility + 0.04F);
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
            tile.fertility = Clamp01(tile.fertility + 0.12F);
        }
    }
}

void WorldGenerator::GenerateResources() {
    for (auto& tile : world_.Tiles()) {
        tile.resource = ResourceKind::None;
        tile.resource_amount = 0.0F;
        if (!IsLand(tile)) {
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
            tile.resource_amount = Clamp01(0.35F + geology * 0.45F + Random01(tile.x, tile.y, 137) * 0.35F);
        }
    }

    for (auto& tile : world_.Tiles()) {
        tile.improvement = ImprovementKind::None;
        tile.worked_by_settlement_id = -1;
        tile.improvement_progress = 0.0F;
        tile.soil_quality = IsLand(tile) ? Clamp01(tile.fertility * 0.78F + tile.rainfall * 0.14F +
                                                   (tile.has_river || tile.is_coast ? 0.10F : 0.0F))
                                         : 0.0F;
        tile.forest_cover = 0.0F;
        if (tile.biome == Biome::Forest) {
            tile.forest_cover = Clamp01(0.62F + tile.rainfall * 0.28F);
        } else if (tile.biome == Biome::Rainforest) {
            tile.forest_cover = Clamp01(0.82F + tile.rainfall * 0.16F);
        } else if (tile.resource == ResourceKind::Wood || tile.resource == ResourceKind::Bamboo) {
            tile.forest_cover = 0.58F;
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
        if (!IsLand(tile) || tile.biome == Biome::Mountain || tile.biome == Biome::Snow) {
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
        if (IsMineral(tile.resource)) {
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

        tile.settlement_score = Clamp01(score);
    }
}

float WorldGenerator::Noise2D(float x, float y, int salt, float frequency) const {
    const float fx = x * frequency;
    const float fy = y * frequency;
    const int x0 = static_cast<int>(std::floor(fx));
    const int y0 = static_cast<int>(std::floor(fy));
    const int x1 = x0 + 1;
    const int y1 = y0 + 1;
    const float tx = Smooth(fx - static_cast<float>(x0));
    const float ty = Smooth(fy - static_cast<float>(y0));

    const float a = Random01(x0, y0, salt);
    const float b = Random01(x1, y0, salt);
    const float c = Random01(x0, y1, salt);
    const float d = Random01(x1, y1, salt);
    return Lerp(Lerp(a, b, tx), Lerp(c, d, tx), ty);
}

float WorldGenerator::Random01(int x, int y, int salt) const {
    std::uint64_t value = params_.seed;
    value ^= static_cast<std::uint64_t>(x + 0x9e3779b9) * 0xbf58476d1ce4e5b9ULL;
    value ^= static_cast<std::uint64_t>(y + 0x85ebca6b) * 0x94d049bb133111ebULL;
    value ^= static_cast<std::uint64_t>(salt + 0xc2b2ae35) * 0x27d4eb2f165667c5ULL;
    value = Mix(value);
    return static_cast<float>((value >> 11U) & 0x1fffffULL) / static_cast<float>(0x1fffffULL);
}

bool WorldGenerator::IsAdjacentToWater(int x, int y) const {
    for (const auto& [dx, dy] : NeighborOffsets8()) {
        const int nx = x + dx;
        const int ny = y + dy;
        if (!world_.InBounds(nx, ny)) {
            continue;
        }
        const auto& neighbor = world_.At(nx, ny);
        if (neighbor.is_ocean || neighbor.is_lake || neighbor.has_river) {
            return true;
        }
    }
    return false;
}

bool WorldGenerator::HasNeighborBiome(int x, int y, Biome biome) const {
    for (const auto& [dx, dy] : NeighborOffsets8()) {
        const int nx = x + dx;
        const int ny = y + dy;
        if (world_.InBounds(nx, ny) && world_.At(nx, ny).biome == biome) {
            return true;
        }
    }
    return false;
}

}  // namespace oikumene
