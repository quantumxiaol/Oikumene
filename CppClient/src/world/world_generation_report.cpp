#include "oikumene/world/world_generation_report.hpp"

#include <algorithm>
#include <cmath>

namespace oikumene {
namespace {

[[nodiscard]] float Ratio(int numerator, int denominator) {
    if (denominator <= 0) {
        return 0.0F;
    }
    return static_cast<float>(numerator) / static_cast<float>(denominator);
}

} // namespace

WorldGenerationReport BuildWorldGenerationReport(const World& world) {
    WorldGenerationReport report;
    report.width = world.Width();
    report.height = world.Height();
    report.seed = world.Seed();

    float elevation_sum = 0.0F;
    float temperature_sum = 0.0F;
    float rainfall_sum = 0.0F;
    float fertility_sum = 0.0F;
    float settlement_score_sum = 0.0F;
    int forest_tiles = 0;
    int desert_tiles = 0;
    int high_score_tiles = 0;

    std::vector<const Tile*> candidates;
    candidates.reserve(world.Tiles().size());

    for (const auto& tile : world.Tiles()) {
        if (tile.is_ocean) {
            ++report.ocean_tiles;
        } else {
            ++report.land_tiles;
        }
        if (tile.is_lake) {
            ++report.lake_tiles;
        }
        if (tile.has_river) {
            ++report.river_tiles;
        }
        if (tile.is_coast) {
            ++report.coast_tiles;
        }

        ++report.biome_counts[tile.biome];
        ++report.resource_counts[tile.resource];

        elevation_sum += tile.elevation;
        temperature_sum += tile.temperature;
        rainfall_sum += tile.rainfall;
        fertility_sum += tile.fertility;
        settlement_score_sum += tile.settlement_score;

        if (tile.biome == Biome::Forest || tile.biome == Biome::Rainforest) {
            ++forest_tiles;
        }
        if (tile.biome == Biome::Desert) {
            ++desert_tiles;
        }
        if (tile.settlement_score >= 0.62F) {
            ++high_score_tiles;
        }
        if (tile.settlement_score > 0.0F) {
            candidates.push_back(&tile);
        }
    }

    const int tile_count = static_cast<int>(world.Tiles().size());
    report.mean_elevation = elevation_sum / static_cast<float>(tile_count);
    report.mean_temperature = temperature_sum / static_cast<float>(tile_count);
    report.mean_rainfall = rainfall_sum / static_cast<float>(tile_count);
    report.mean_fertility = fertility_sum / static_cast<float>(tile_count);
    report.mean_settlement_score = settlement_score_sum / static_cast<float>(tile_count);
    report.land_ratio = Ratio(report.land_tiles, tile_count);
    report.river_ratio = Ratio(report.river_tiles, tile_count);
    report.forest_ratio = Ratio(forest_tiles, tile_count);
    report.desert_ratio = Ratio(desert_tiles, tile_count);
    report.high_score_settlement_ratio = Ratio(high_score_tiles, tile_count);

    std::sort(candidates.begin(), candidates.end(),
              [](const Tile* lhs, const Tile* rhs) { return lhs->settlement_score > rhs->settlement_score; });

    const std::size_t candidate_count = std::min<std::size_t>(10, candidates.size());
    report.top_settlement_candidates.reserve(candidate_count);
    for (std::size_t i = 0; i < candidate_count; ++i) {
        const auto& tile = *candidates[i];
        report.top_settlement_score = std::max(report.top_settlement_score, tile.settlement_score);
        report.top_settlement_candidates.push_back(SettlementCandidateReport{
            .x = tile.x,
            .y = tile.y,
            .score = tile.settlement_score,
            .biome = tile.biome,
            .resource = tile.resource,
            .has_river = tile.has_river,
            .is_coast = tile.is_coast,
            .fertility = tile.fertility,
        });
    }

    return report;
}

nlohmann::json ToJson(const WorldGenerationReport& report) {
    nlohmann::json biome_counts = nlohmann::json::object();
    for (const auto& [biome, count] : report.biome_counts) {
        biome_counts[ToString(biome)] = count;
    }

    nlohmann::json resource_counts = nlohmann::json::object();
    for (const auto& [resource, count] : report.resource_counts) {
        resource_counts[ToString(resource)] = count;
    }

    nlohmann::json candidates = nlohmann::json::array();
    for (const auto& candidate : report.top_settlement_candidates) {
        candidates.push_back(nlohmann::json{
            {"x", candidate.x},
            {"y", candidate.y},
            {"score", candidate.score},
            {"biome", ToString(candidate.biome)},
            {"resource", ToString(candidate.resource)},
            {"has_river", candidate.has_river},
            {"is_coast", candidate.is_coast},
            {"fertility", candidate.fertility},
        });
    }

    return nlohmann::json{
        {"width", report.width},
        {"height", report.height},
        {"seed", report.seed},
        {"land_tiles", report.land_tiles},
        {"ocean_tiles", report.ocean_tiles},
        {"lake_tiles", report.lake_tiles},
        {"river_tiles", report.river_tiles},
        {"coast_tiles", report.coast_tiles},
        {"biome_counts", biome_counts},
        {"resource_counts", resource_counts},
        {"mean_elevation", report.mean_elevation},
        {"mean_temperature", report.mean_temperature},
        {"mean_rainfall", report.mean_rainfall},
        {"mean_fertility", report.mean_fertility},
        {"mean_settlement_score", report.mean_settlement_score},
        {"land_ratio", report.land_ratio},
        {"river_ratio", report.river_ratio},
        {"forest_ratio", report.forest_ratio},
        {"desert_ratio", report.desert_ratio},
        {"high_score_settlement_ratio", report.high_score_settlement_ratio},
        {"top_settlement_score", report.top_settlement_score},
        {"top_settlement_candidates", candidates},
    };
}

} // namespace oikumene
