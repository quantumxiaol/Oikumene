#include <cassert>
#include <iostream>

#include "oikumene/core/simulation.hpp"
#include "oikumene/sim/settlement_system.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

oikumene::World MakeWorld(std::uint64_t seed = 42) {
    oikumene::WorldGenerationParams params;
    params.seed = seed;
    return oikumene::WorldGenerator::Generate(params);
}

bool WorkableSettlementTile(const oikumene::Tile& tile) {
    return !tile.is_ocean && !tile.is_lake && tile.biome != oikumene::Biome::Mountain &&
           tile.biome != oikumene::Biome::Snow && tile.soil_quality > 0.0F;
}

oikumene::Settlement MakeSettlementAtBestSoil(const oikumene::World& world) {
    const oikumene::Tile* best = nullptr;
    for (const auto& tile : world.Tiles()) {
        if (!WorkableSettlementTile(tile)) {
            continue;
        }
        if (best == nullptr || tile.soil_quality > best->soil_quality) {
            best = &tile;
        }
    }
    assert(best != nullptr);

    oikumene::Settlement settlement;
    settlement.id = 0;
    settlement.x = best->x;
    settlement.y = best->y;
    settlement.population = 80;
    settlement.turns_since_founded = 20;
    settlement.stockpile.food = 120.0F;
    settlement.stockpile.wood = 80.0F;
    return settlement;
}

int CountImprovement(const oikumene::World& world, oikumene::ImprovementKind kind) {
    int count = 0;
    for (const auto& tile : world.Tiles()) {
        count += tile.improvement == kind ? 1 : 0;
    }
    return count;
}

void RunSettlementUpdate(oikumene::Simulation& sim, int turns) {
    oikumene::SimulationParams params;
    for (int i = 0; i < turns; ++i) {
        oikumene::SettlementSystem::UpdateSettlements(sim.MutableWorld(), params, i, sim.Settlements(), sim.Events());
    }
}

void TestVillageBuildsFarmOnFertileTile() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.Settlements().push_back(MakeSettlementAtBestSoil(sim.GetWorld()));
    sim.Settlements().front().level = SettlementLevel::Village;
    RunSettlementUpdate(sim, 1);
    assert(CountImprovement(sim.GetWorld(), ImprovementKind::Farm) > 0);
}

void TestLumberCampRequiresForestOrWood() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.Settlements().push_back(MakeSettlementAtBestSoil(sim.GetWorld()));
    sim.Settlements().front().level = SettlementLevel::Village;
    sim.Settlements().front().stockpile.wood = 0.0F;
    RunSettlementUpdate(sim, 8);

    for (const auto& tile : sim.GetWorld().Tiles()) {
        if (tile.improvement == ImprovementKind::LumberCamp) {
            assert(tile.forest_cover > 0.0F || tile.resource == ResourceKind::Wood || tile.resource == ResourceKind::Bamboo ||
                   tile.biome == Biome::Forest || tile.biome == Biome::Rainforest);
        }
    }
}

void TestFarmIncreasesFoodOutput() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.Settlements().push_back(MakeSettlementAtBestSoil(sim.GetWorld()));
    RunSettlementUpdate(sim, 2);
    assert(CountImprovement(sim.GetWorld(), ImprovementKind::Farm) > 0);
    assert(sim.Settlements().front().local_food_output_last_turn > 0.0F);
}

void TestCarryingCapacityLimitsPopulationGrowth() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    auto settlement = MakeSettlementAtBestSoil(sim.GetWorld());
    settlement.population = 500;
    settlement.stockpile.food = 1000.0F;
    sim.Settlements().push_back(settlement);
    const int before = sim.Settlements().front().population;
    RunSettlementUpdate(sim, 5);
    assert(sim.Settlements().front().population <= before);
    assert(sim.Settlements().front().carrying_capacity_ratio > 1.0F);
}

void TestNoFarmOnOceanOrMountain() {
    using namespace oikumene;

    Simulation sim(MakeWorld(42), SimulationParams{});
    sim.Settlements().push_back(MakeSettlementAtBestSoil(sim.GetWorld()));
    RunSettlementUpdate(sim, 24);

    for (const auto& tile : sim.GetWorld().Tiles()) {
        if (tile.improvement == ImprovementKind::Farm) {
            assert(!tile.is_ocean);
            assert(tile.biome != Biome::Mountain);
            assert(tile.biome != Biome::Snow);
        }
    }
}

void TestTileImprovementIsDeterministicForSameSeed() {
    using namespace oikumene;

    Simulation left(MakeWorld(42), SimulationParams{});
    Simulation right(MakeWorld(42), SimulationParams{});
    left.Settlements().push_back(MakeSettlementAtBestSoil(left.GetWorld()));
    right.Settlements().push_back(MakeSettlementAtBestSoil(right.GetWorld()));
    RunSettlementUpdate(left, 32);
    RunSettlementUpdate(right, 32);

    assert(CountImprovement(left.GetWorld(), ImprovementKind::Farm) ==
           CountImprovement(right.GetWorld(), ImprovementKind::Farm));
    assert(CountImprovement(left.GetWorld(), ImprovementKind::LumberCamp) ==
           CountImprovement(right.GetWorld(), ImprovementKind::LumberCamp));
    assert(left.Settlements().front().population == right.Settlements().front().population);
}

}  // namespace

int main() {
    TestVillageBuildsFarmOnFertileTile();
    TestLumberCampRequiresForestOrWood();
    TestFarmIncreasesFoodOutput();
    TestCarryingCapacityLimitsPopulationGrowth();
    TestNoFarmOnOceanOrMountain();
    TestTileImprovementIsDeterministicForSameSeed();

    std::cout << "oikumene_settlement_economy_tests passed\n";
    return 0;
}
