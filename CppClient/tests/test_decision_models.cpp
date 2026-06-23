#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

#include "oikumene/ai/heuristic_decision_provider.hpp"
#include "oikumene/io/json_protocol.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

void TestDecisionModels() {
    using namespace oikumene;

    CandidateAction action{
        .action_id = "research_mining",
        .type = ActionType::ResearchTech,
        .target = "mining",
        .cost = 40.0,
        .estimated_benefit = 0.8,
        .risk = 0.1,
    };

    DecisionRequest request;
    request.request_id = "turn_10_polity_1";
    request.turn = 10;
    request.strategic_turn = 2;
    request.polity_id = 1;
    request.report = Json{{"name", "Test Polity"}};
    request.candidate_actions.push_back(action);

    const auto serialized = SerializeDecisionRequest(request);
    const auto parsed = serialized.get<DecisionRequest>();
    assert(parsed.request_id == request.request_id);
    assert(parsed.candidate_actions.size() == 1);
    assert(parsed.candidate_actions.front().type == ActionType::ResearchTech);

    DecisionBatchRequest batch_request;
    batch_request.requests.push_back(request);

    HeuristicDecisionProvider provider;
    const auto response = provider.DecideBatch(batch_request);
    assert(response.responses.size() == 1);
    assert(response.responses.front().selected_actions.size() == 1);
    assert(response.responses.front().selected_actions.front().action_id == "research_mining");
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
            assert(a.biome == b.biome);
            assert(a.resource == b.resource);
        }
    }
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

void TestGeneratedWorldHasSomeRiversForCommonSeed() {
    using namespace oikumene;

    WorldGenerationParams params;
    params.seed = 42;
    const auto world = WorldGenerator::Generate(params);

    std::size_t river_tiles = 0;
    for (const auto& tile : world.Tiles()) {
        if (tile.has_river) {
            ++river_tiles;
        }
    }
    assert(river_tiles > 0);
}

void TestResourcesAreNotAllNone() {
    using namespace oikumene;

    const auto world = WorldGenerator::Generate(WorldGenerationParams{});
    std::size_t resources = 0;
    for (const auto& tile : world.Tiles()) {
        if (tile.resource != ResourceKind::None) {
            ++resources;
        }
    }
    assert(resources > 0);
}

void TestHighSettlementScorePrefersWaterOrFertility() {
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
        const bool near_water = tile.has_river || tile.is_coast || tile.biome == Biome::River ||
                                tile.biome == Biome::Coast || tile.biome == Biome::Wetland;
        if (near_water || tile.fertility > 0.55F) {
            ++plausible;
        }
        if (checked >= 25) {
            break;
        }
    }

    assert(checked >= 10);
    assert(plausible >= checked / 2);
}

}  // namespace

int main() {
    TestDecisionModels();
    TestSameSeedIsStable();
    TestTileValuesAreInRange();
    TestGeneratedWorldHasLandAndOcean();
    TestGeneratedWorldHasSomeRiversForCommonSeed();
    TestResourcesAreNotAllNone();
    TestHighSettlementScorePrefersWaterOrFertility();

    std::cout << "oikumene_tests passed\n";
    return 0;
}
