#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "oikumene/world/world_generation_report.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

int CountResource(const oikumene::WorldGenerationReport& report, oikumene::ResourceKind resource) {
    const auto it = report.resource_counts.find(resource);
    return it == report.resource_counts.end() ? 0 : it->second;
}

int BaseMineralCount(const oikumene::WorldGenerationReport& report) {
    return CountResource(report, oikumene::ResourceKind::Copper) + CountResource(report, oikumene::ResourceKind::Tin) +
           CountResource(report, oikumene::ResourceKind::ShallowIron) +
           CountResource(report, oikumene::ResourceKind::ShallowCoal);
}

void TestWorldGenerationBalanceAcrossFixedSeeds() {
    using namespace oikumene;

    const std::vector<std::uint64_t> seeds = {0,  1,  2,  3,  4,  5,  7,   11,   13,    17,
                                              23, 29, 31, 37, 42, 99, 123, 2026, 12345, 20260623};

    int enough_rivers = 0;
    int enough_wood = 0;
    int has_base_minerals = 0;
    float land_ratio_sum = 0.0F;
    float forest_ratio_sum = 0.0F;
    float desert_ratio_sum = 0.0F;

    for (const auto seed : seeds) {
        WorldGenerationParams params;
        params.seed = seed;
        const auto world = WorldGenerator::Generate(params);
        const auto report = BuildWorldGenerationReport(world);

        assert(report.land_tiles > 0);
        assert(report.ocean_tiles > 0);
        assert(report.top_settlement_score >= 0.50F);

        enough_rivers += report.river_tiles >= 20 ? 1 : 0;
        enough_wood += CountResource(report, ResourceKind::Wood) >= 20 ? 1 : 0;
        has_base_minerals += BaseMineralCount(report) >= 1 ? 1 : 0;
        land_ratio_sum += report.land_ratio;
        forest_ratio_sum += report.forest_ratio;
        desert_ratio_sum += report.desert_ratio;
    }

    const float count = static_cast<float>(seeds.size());
    assert(enough_rivers >= 16);
    assert(enough_wood >= 16);
    assert(has_base_minerals >= 16);
    assert(land_ratio_sum / count >= 0.25F);
    assert(land_ratio_sum / count <= 0.60F);
    assert(forest_ratio_sum / count > 0.02F);
    assert(desert_ratio_sum / count > 0.002F);
}

} // namespace

int main() {
    TestWorldGenerationBalanceAcrossFixedSeeds();
    std::cout << "oikumene_world_generation_balance_tests passed\n";
    return 0;
}
