#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
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
    std::sort(sorted.begin(), sorted.end(),
              [](const Tile* lhs, const Tile* rhs) { return lhs->settlement_score < rhs->settlement_score; });
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
        SettlementSystem::UpdateSettlements(sim.MutableWorld(), params, i, sim.Settlements(), sim.Events());
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

void TestBandDecisionReasonIsRecordedAfterUpdate() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);
    sim.AdvanceOneTurn();

    bool found_reason = false;
    for (const auto& band : sim.Bands()) {
        if (!band.last_decision_reason.empty() && band.forage_yield_last_turn > 0.0F &&
            band.current_tile_score >= 0.0F) {
            found_reason = true;
        }
    }
    assert(found_reason);
}

void TestSettlementFoundingEventContainsUsefulSummary() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);
    for (int i = 0; i < 80; ++i) {
        sim.AdvanceOneTurn();
    }

    bool found_summary = false;
    for (const auto& event : sim.Events().Events()) {
        if (event.type == EventType::SettlementFounded && event.summary.find("score") != std::string::npos &&
            event.summary.find("fertility") != std::string::npos) {
            found_summary = true;
        }
    }
    assert(found_summary);
}

void TestSimulationResetCreatesFreshState() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);
    for (int i = 0; i < 80; ++i) {
        sim.AdvanceOneTurn();
    }
    assert(sim.CurrentTurn() == 80);
    assert(!sim.Settlements().empty());
    assert(sim.Events().Size() > 0);

    sim = Simulation(MakeWorld(43), SimulationParams{});
    sim.InitializeBands(5);
    assert(sim.CurrentTurn() == 0);
    assert(sim.Bands().size() == 5);
    assert(sim.Settlements().empty());
    assert(sim.Events().Size() == 0);
    assert(sim.GetWorld().Seed() == 43);
}

void TestRegenerateWorldChangesSeed() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);
    assert(sim.GetWorld().Seed() == 42);

    sim = Simulation(MakeWorld(43), SimulationParams{});
    sim.InitializeBands(8);
    assert(sim.GetWorld().Seed() == 43);
}

void TestResetBandsClearsSettlementsAndEvents() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);
    for (int i = 0; i < 80; ++i) {
        sim.AdvanceOneTurn();
    }
    assert(!sim.Settlements().empty());
    assert(sim.Events().Size() > 0);

    sim.InitializeBands(8);
    assert(sim.CurrentTurn() == 0);
    assert(sim.Settlements().empty());
    assert(sim.Events().Size() == 0);
    assert(sim.Bands().size() == 8);
}

void TestResetBandsDoesNotChangeSeed() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);
    for (int i = 0; i < 20; ++i) {
        sim.AdvanceOneTurn();
    }
    sim.InitializeBands(8);
    assert(sim.GetWorld().Seed() == 42);
}

void TestSimulationDeterministicForSameSeed() {
    using namespace oikumene;

    Simulation left(MakeWorld(42), SimulationParams{});
    Simulation right(MakeWorld(42), SimulationParams{});
    left.InitializeBands(8);
    right.InitializeBands(8);

    for (int i = 0; i < 120; ++i) {
        left.AdvanceOneTurn();
        right.AdvanceOneTurn();
    }

    assert(left.CurrentTurn() == right.CurrentTurn());
    assert(left.Bands().size() == right.Bands().size());
    assert(left.Settlements().size() == right.Settlements().size());
    assert(left.Events().Size() == right.Events().Size());

    for (std::size_t i = 0; i < left.Bands().size(); ++i) {
        assert(left.Bands()[i].x == right.Bands()[i].x);
        assert(left.Bands()[i].y == right.Bands()[i].y);
        assert(left.Bands()[i].population == right.Bands()[i].population);
        assert(left.Bands()[i].active == right.Bands()[i].active);
        assert(left.Bands()[i].state == right.Bands()[i].state);
    }

    for (std::size_t i = 0; i < left.Settlements().size(); ++i) {
        assert(left.Settlements()[i].x == right.Settlements()[i].x);
        assert(left.Settlements()[i].y == right.Settlements()[i].y);
        assert(left.Settlements()[i].population == right.Settlements()[i].population);
        assert(left.Settlements()[i].level == right.Settlements()[i].level);
    }
}

void TestEventLogIsChronological() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.InitializeBands(8);
    for (int i = 0; i < 120; ++i) {
        sim.AdvanceOneTurn();
    }

    Turn previous = 0;
    for (const auto& event : sim.Events().Events()) {
        assert(event.turn >= previous);
        previous = event.turn;
    }
}

void TestCampCanUpgradeToVillage() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    SimulationParams params;
    Settlement settlement;
    settlement.id = 0;
    settlement.x = 20;
    settlement.y = 20;
    settlement.population = 80;
    settlement.turns_since_founded = 20;
    settlement.stockpile.food = 200.0F;
    settlement.stockpile.wood = 100.0F;
    sim.Settlements().push_back(settlement);

    SettlementSystem::UpdateSettlements(sim.MutableWorld(), params, 20, sim.Settlements(), sim.Events());
    assert(sim.Settlements().front().level == SettlementLevel::Village);
    assert(sim.Settlements().front().local_food_output_last_turn > 0.0F);
    assert(sim.Settlements().front().local_wood_output_last_turn >= 0.0F);
    assert(sim.Settlements().front().food_consumption_last_turn > 0.0F);
    assert(sim.Settlements().front().upgrade_readiness == 1.0F);

    bool found_upgrade_event = false;
    for (const auto& event : sim.Events().Events()) {
        if (event.type == EventType::SettlementUpgraded) {
            found_upgrade_event = true;
        }
    }
    assert(found_upgrade_event);
}

} // namespace

int main() {
    TestBandsSpawnOnValidLand();
    TestForageYieldDependsOnBiomeAndWater();
    TestBandMigratesTowardBetterSettlementScore();
    TestBandEventuallySettlesOnCommonSeed();
    TestSettlementsAreFoundedOnValidTiles();
    TestSettlementsPreferHighScoreTiles();
    TestSettlementPopulationChangesWithFood();
    TestSettlementFoundingEventsAreLogged();
    TestBandDecisionReasonIsRecordedAfterUpdate();
    TestSettlementFoundingEventContainsUsefulSummary();
    TestSimulationResetCreatesFreshState();
    TestRegenerateWorldChangesSeed();
    TestResetBandsClearsSettlementsAndEvents();
    TestResetBandsDoesNotChangeSeed();
    TestSimulationDeterministicForSameSeed();
    TestEventLogIsChronological();
    TestCampCanUpgradeToVillage();

    std::cout << "oikumene_band_settlement_tests passed\n";
    return 0;
}
