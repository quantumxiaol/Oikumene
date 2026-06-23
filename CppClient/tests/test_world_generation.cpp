#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

#include "oikumene/world/world_generation_report.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

bool NearWater(const oikumene::Tile& tile) {
    return tile.has_river || tile.is_coast || tile.biome == oikumene::Biome::Coast ||
           tile.biome == oikumene::Biome::Wetland;
}

void TestSameSeedIsStable() {
    using namespace oikumene;

    WorldGenerationParams params;
    params.seed = 12345;

    const auto first = WorldGenerator::Generate(params);
    const auto second = WorldGenerator::Generate(params);
    assert(first.Width() == second.Width());
    assert(first.Height() == second.Height());

    for (int y = 0; y < first.Height(); ++y) {
        for (int x = 0; x < first.Width(); ++x) {
            const auto& a = first.At(x, y);
            const auto& b = second.At(x, y);
            assert(std::abs(a.elevation - b.elevation) < 0.000001F);
            assert(std::abs(a.temperature - b.temperature) < 0.000001F);
            assert(std::abs(a.rainfall - b.rainfall) < 0.000001F);
            assert(std::abs(a.river_flow - b.river_flow) < 0.000001F);
            assert(a.biome == b.biome);
            assert(a.resource == b.resource);
            assert(a.has_river == b.has_river);
        }
    }
}

void TestDifferentSeedChangesWorld() {
    using namespace oikumene;

    WorldGenerationParams first_params;
    first_params.seed = 1;
    WorldGenerationParams second_params;
    second_params.seed = 2;

    const auto first = WorldGenerator::Generate(first_params);
    const auto second = WorldGenerator::Generate(second_params);

    int changed = 0;
    for (int y = 0; y < first.Height(); ++y) {
        for (int x = 0; x < first.Width(); ++x) {
            const auto& a = first.At(x, y);
            const auto& b = second.At(x, y);
            if (std::abs(a.elevation - b.elevation) > 0.02F || a.biome != b.biome) {
                ++changed;
            }
        }
    }
    assert(changed > first.Width() * first.Height() / 4);
}

void TestTileValuesAreInRange() {
    using namespace oikumene;

    const auto world = WorldGenerator::Generate(WorldGenerationParams{});
    for (const auto& tile : world.Tiles()) {
        assert(tile.elevation >= 0.0F && tile.elevation <= 1.0F);
        assert(tile.temperature >= 0.0F && tile.temperature <= 1.0F);
        assert(tile.rainfall >= 0.0F && tile.rainfall <= 1.0F);
        assert(tile.fertility >= 0.0F && tile.fertility <= 1.0F);
        assert(tile.settlement_score >= 0.0F && tile.settlement_score <= 1.0F);
        assert(tile.river_flow >= 0.0F);
        assert(tile.movement_cost >= 1.0F);
    }
}

void TestGeneratedWorldHasLandAndOcean() {
    using namespace oikumene;

    const auto world = WorldGenerator::Generate(WorldGenerationParams{});
    std::size_t land = 0;
    std::size_t ocean = 0;
    for (const auto& tile : world.Tiles()) {
        if (tile.is_ocean) {
            ++ocean;
        } else {
            ++land;
        }
    }
    assert(land > 0);
    assert(ocean > 0);
}

void TestGeneratedWorldHasRiversWithFlow() {
    using namespace oikumene;

    WorldGenerationParams params;
    params.seed = 42;
    const auto world = WorldGenerator::Generate(params);

    std::size_t river_tiles = 0;
    for (const auto& tile : world.Tiles()) {
        if (tile.has_river) {
            ++river_tiles;
            assert(tile.river_flow >= params.river_flow_threshold);
        }
    }
    assert(river_tiles > 0);
}

void TestRiversDoNotReplacePrimaryBiome() {
    using namespace oikumene;

    const auto world = WorldGenerator::Generate(WorldGenerationParams{});
    for (const auto& tile : world.Tiles()) {
        if (tile.has_river) {
            assert(tile.biome != Biome::River);
            assert(!tile.is_ocean);
            assert(!tile.is_lake);
        }
    }
}

void TestResourcesAreGeographicallyPlausible() {
    using namespace oikumene;

    const auto world = WorldGenerator::Generate(WorldGenerationParams{});
    std::size_t resources = 0;
    for (const auto& tile : world.Tiles()) {
        if (tile.resource == ResourceKind::None) {
            continue;
        }
        ++resources;
        if (tile.resource == ResourceKind::Horse) {
            assert(tile.biome == Biome::Grassland);
        }
        if (tile.resource == ResourceKind::Wood) {
            assert(tile.biome == Biome::Forest || tile.biome == Biome::Rainforest);
        }
        if (tile.resource == ResourceKind::Copper || tile.resource == ResourceKind::Tin ||
            tile.resource == ResourceKind::ShallowIron || tile.resource == ResourceKind::Stone) {
            assert(tile.biome == Biome::Hill || tile.biome == Biome::Mountain || tile.biome == Biome::Snow);
        }
    }
    assert(resources > 0);
}

void TestTopSettlementCandidatesArePlausible() {
    using namespace oikumene;

    const auto world = WorldGenerator::Generate(WorldGenerationParams{});
    std::vector<Tile> tiles = world.Tiles();
    std::sort(tiles.begin(), tiles.end(), [](const Tile& lhs, const Tile& rhs) {
        return lhs.settlement_score > rhs.settlement_score;
    });

    std::size_t checked = 0;
    std::size_t plausible = 0;
    for (const auto& tile : tiles) {
        if (tile.settlement_score <= 0.0F) {
            continue;
        }
        ++checked;
        if (NearWater(tile) || tile.fertility > 0.55F) {
            ++plausible;
        }
        if (checked >= 25) {
            break;
        }
    }

    assert(checked >= 10);
    assert(plausible >= checked / 2);
}

void TestReportCountsMatchWorldTiles() {
    using namespace oikumene;

    const auto world = WorldGenerator::Generate(WorldGenerationParams{});
    const auto report = BuildWorldGenerationReport(world);

    int river_tiles = 0;
    int lake_tiles = 0;
    int coast_tiles = 0;
    int land_tiles = 0;
    int ocean_tiles = 0;
    for (const auto& tile : world.Tiles()) {
        river_tiles += tile.has_river ? 1 : 0;
        lake_tiles += tile.is_lake ? 1 : 0;
        coast_tiles += tile.is_coast ? 1 : 0;
        land_tiles += tile.is_ocean ? 0 : 1;
        ocean_tiles += tile.is_ocean ? 1 : 0;
    }

    assert(report.width == world.Width());
    assert(report.height == world.Height());
    assert(report.seed == world.Seed());
    assert(report.river_tiles == river_tiles);
    assert(report.lake_tiles == lake_tiles);
    assert(report.coast_tiles == coast_tiles);
    assert(report.land_tiles == land_tiles);
    assert(report.ocean_tiles == ocean_tiles);
    assert(!report.top_settlement_candidates.empty());
    assert(ToJson(report)["river_tiles"].get<int>() == river_tiles);
}

}  // namespace

int main() {
    TestSameSeedIsStable();
    TestDifferentSeedChangesWorld();
    TestTileValuesAreInRange();
    TestGeneratedWorldHasLandAndOcean();
    TestGeneratedWorldHasRiversWithFlow();
    TestRiversDoNotReplacePrimaryBiome();
    TestResourcesAreGeographicallyPlausible();
    TestTopSettlementCandidatesArePlausible();
    TestReportCountsMatchWorldTiles();

    std::cout << "oikumene_world_generation_tests passed\n";
    return 0;
}
