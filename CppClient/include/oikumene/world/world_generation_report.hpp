#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include <nlohmann/json.hpp>

#include "oikumene/world/biome.hpp"
#include "oikumene/world/resource.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

struct SettlementCandidateReport {
    int x = 0;
    int y = 0;
    float score = 0.0F;
    Biome biome = Biome::Ocean;
    ResourceKind resource = ResourceKind::None;
    bool has_river = false;
    bool is_coast = false;
    float fertility = 0.0F;
};

struct WorldGenerationReport {
    int width = 0;
    int height = 0;
    std::uint64_t seed = 0;

    int land_tiles = 0;
    int ocean_tiles = 0;
    int lake_tiles = 0;
    int river_tiles = 0;
    int coast_tiles = 0;

    std::map<Biome, int> biome_counts;
    std::map<ResourceKind, int> resource_counts;

    float mean_elevation = 0.0F;
    float mean_temperature = 0.0F;
    float mean_rainfall = 0.0F;
    float mean_fertility = 0.0F;
    float mean_settlement_score = 0.0F;

    float land_ratio = 0.0F;
    float river_ratio = 0.0F;
    float forest_ratio = 0.0F;
    float desert_ratio = 0.0F;
    float high_score_settlement_ratio = 0.0F;
    float top_settlement_score = 0.0F;

    std::vector<SettlementCandidateReport> top_settlement_candidates;
};

[[nodiscard]] WorldGenerationReport BuildWorldGenerationReport(const World& world);
[[nodiscard]] nlohmann::json ToJson(const WorldGenerationReport& report);

}  // namespace oikumene
