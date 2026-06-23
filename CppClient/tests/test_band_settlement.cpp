#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "oikumene/core/simulation.hpp"
#include "oikumene/sim/band_system.hpp"
#include "oikumene/sim/settlement_system.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

oikumene::World MakeWorld(std::uint64_t seed = 42) {
    oikumene::WorldGenerationParams params;
    params.seed = seed;
    return oikumene::WorldGenerator::Generate(params);
}

void TestBandsSpawnOnValidLand() {
    using namespace oikumene;

    Simulation sim(MakeWorld(), SimulationParams{});
    sim.InitializeBands(8);
    assert(sim.Bands().size() == 8);
    for (const auto& band : sim.Bands()) {
        assert(sim.GetWorld().InBounds(band.x, band.y));
        assert(IsValidBandTile(sim.GetWorld().At(band.x, band.y)));
    }
}

void TestForageYieldDependsOnBiomeAndWater() {
    using namespace oikumene;

    Tile forest;
    forest.biome = Biome::Forest;
    forest.fertility = 0.60F;

    Tile river_grass;
    river_grass.biome = Biome::Grassland;
    river_grass.fertility = 0.60F;
    river_grass.has_river = true;

    Tile desert;
    desert.biome = Biome::Desert;
    desert.fertility = 0.08F;

    Tile mountain;
    mountain.biome = Biome::Mountain;
    mountain.fertility = 0.20F;

    assert(ForageYield(forest) > ForageYield(desert));
    assert(ForageYield(river_grass) > ForageYield(forest));
    assert(ForageYield(mountain) == 0.0F);
}

void TestBandMigratesTowardBetterSettlementScore() {
    using namespace oikumene;

    auto world = MakeWorld(42);
    std::vector<const Tile*> sorted;
    for (const auto& tile : world.Tiles()) {
        if (IsValidBandTile(tile)) {
            sorted.push_back(&tile);
        }
    }
    std::sort(sorted.begin(), sorted.end(), [](const Tile* lhs, const Tile* rhs) {
        return lhs->settlement_score < rhs->settlement_score;
    });
    assert(sorted.size() > 2);

    const auto* low = sorted.front();
    const auto* high = sorted.back();
    SimulationParams params;
    params.settlement_threshold = 0.99F;

    Simulation sim(std::move(world), params);
    Band band;
    band.id = 0;
    band.x = low->x;
    band.y = low->y;
    band.food = 100.0F;
    band.population = 30;
    sim.Bands().push_back(band);

    const int before = std::abs(band.x - high->x) + std::abs(band.y - high->y);
    for (int i = 0; i < 12; ++i) {
        sim.AdvanceOneTurn();
    }
    const auto& moved = sim.Bands().front();
    const int after = std::abs(moved.x - high->x) + std::abs(moved.y - high->y);
    assert(after < before);
}

void TestBandEventuallySettlesOnCommonSeed() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);

    for (int i = 0; i < 80; ++i) {
        sim.AdvanceOneTurn();
    }
    assert(!sim.Settlements().empty());
}

void TestSettlementsAreFoundedOnValidTiles() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);
    for (int i = 0; i < 80; ++i) {
        sim.AdvanceOneTurn();
    }

    assert(!sim.Settlements().empty());
    for (const auto& settlement : sim.Settlements()) {
        const auto& tile = sim.GetWorld().At(settlement.x, settlement.y);
        assert(IsValidSettlementTile(tile));
        assert(tile.settlement_score >= 0.50F);
        assert(tile.has_river || tile.is_coast || tile.fertility > 0.55F);
    }
}

void TestSettlementsPreferHighScoreTiles() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);
    for (int i = 0; i < 80; ++i) {
        sim.AdvanceOneTurn();
    }

    for (const auto& settlement : sim.Settlements()) {
        const auto& tile = sim.GetWorld().At(settlement.x, settlement.y);
        assert(tile.settlement_score >= 0.50F);
    }
}

void TestSettlementPopulationChangesWithFood() {
    using namespace oikumene;

    auto world = MakeWorld(42);
    SimulationParams params;
    Simulation sim(std::move(world), params);
    Settlement settlement;
    settlement.id = 0;
    settlement.x = 20;
    settlement.y = 20;
    settlement.population = 40;
    settlement.stockpile.food = 100.0F;
    settlement.stockpile.wood = 100.0F;
    sim.Settlements().push_back(settlement);

    const int before = sim.Settlements().front().population;
    for (int i = 0; i < 20; ++i) {
        SettlementSystem::UpdateSettlements(sim.GetWorld(), params, i, sim.Settlements(), sim.Events());
    }
    assert(sim.Settlements().front().population > before);
}

void TestSettlementFoundingEventsAreLogged() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);
    for (int i = 0; i < 80; ++i) {
        sim.AdvanceOneTurn();
    }

    bool found_settlement_event = false;
    for (const auto& event : sim.Events().Events()) {
        if (event.type == EventType::SettlementFounded) {
            found_settlement_event = true;
        }
    }
    assert(found_settlement_event);
}

}  // namespace

int main() {
    TestBandsSpawnOnValidLand();
    TestForageYieldDependsOnBiomeAndWater();
    TestBandMigratesTowardBetterSettlementScore();
    TestBandEventuallySettlesOnCommonSeed();
    TestSettlementsAreFoundedOnValidTiles();
    TestSettlementsPreferHighScoreTiles();
    TestSettlementPopulationChangesWithFood();
    TestSettlementFoundingEventsAreLogged();

    std::cout << "oikumene_band_settlement_tests passed\n";
    return 0;
}
