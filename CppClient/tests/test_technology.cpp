#include <cassert>
#include <iostream>

#include "oikumene/core/simulation.hpp"
#include "oikumene/sim/technology_system.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

oikumene::World MakeWorld(int width = 20, int height = 10) {
    using namespace oikumene;
    World world(width, height, 404);
    for (auto& tile : world.Tiles()) {
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.is_coast = false;
        tile.biome = Biome::Grassland;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.55F;
        tile.soil_quality = 0.55F;
        tile.resource = ResourceKind::None;
    }
    return world;
}

oikumene::Settlement MakeVillage(int id, int x, int y, int population = 260) {
    oikumene::Settlement settlement;
    settlement.id = id;
    settlement.x = x;
    settlement.y = y;
    settlement.level = oikumene::SettlementLevel::Village;
    settlement.population = population;
    settlement.stockpile.food = 160.0F;
    settlement.stockpile.wood = 80.0F;
    settlement.local_food_output_last_turn = 30.0F;
    settlement.food_consumption_last_turn = 18.0F;
    settlement.carrying_capacity = 260.0F;
    settlement.carrying_capacity_ratio = 0.55F;
    return settlement;
}

oikumene::Polity MakePolity(int capital_id = 0) {
    oikumene::Polity polity;
    polity.id = 0;
    polity.name = "Test Polity";
    polity.capital_settlement_id = capital_id;
    polity.member_settlement_ids = {capital_id};
    polity.population = 260;
    polity.stability = 1.0F;
    polity.overextension = 0.0F;
    polity.budget.wealth_surplus = 80.0F;
    return polity;
}

void Assign(oikumene::Settlement& settlement, oikumene::PolityId polity_id, bool capital = false) {
    settlement.polity_id = polity_id;
    settlement.is_capital = capital;
}

void TestPolityAccumulatesKnowledgeAndUnlocksTech() {
    using namespace oikumene;
    World world = MakeWorld();
    std::vector<Settlement> settlements{MakeVillage(0, 5, 5, 420)};
    Assign(settlements[0], 0, true);
    std::vector<Polity> polities{MakePolity(0)};
    EventLog events;

    for (int turn = 0; turn < 40; ++turn) {
        TechnologySystem::UpdateTechnologies(world, turn, settlements, polities, events);
    }

    assert(polities.front().knowledge_income > 0.0F);
    assert(!polities.front().research.unlocked.empty());
    assert(polities.front().research.first_unlock_turn >= 0);
}

void TestTechPrerequisitesAreRespected() {
    using namespace oikumene;
    World world = MakeWorld();
    std::vector<Settlement> settlements{MakeVillage(0, 5, 5)};
    Assign(settlements[0], 0, true);
    std::vector<Polity> polities{MakePolity(0)};

    assert(!CanResearchTech(world, settlements, polities.front(), TechId::Administration));
    polities.front().research.unlocked.push_back(TechId::Pottery);
    polities.front().research.unlocked.push_back(TechId::Roads);
    assert(CanResearchTech(world, settlements, polities.front(), TechId::Administration));
}

void TestBronzeWorkingRequiresMiningAndMetals() {
    using namespace oikumene;
    World world = MakeWorld();
    world.At(6, 5).resource = ResourceKind::Copper;
    world.At(7, 5).resource = ResourceKind::Tin;
    std::vector<Settlement> settlements{MakeVillage(0, 5, 5)};
    Assign(settlements[0], 0, true);
    std::vector<Polity> polities{MakePolity(0)};

    assert(!CanResearchTech(world, settlements, polities.front(), TechId::BronzeWorking));
    polities.front().research.unlocked.push_back(TechId::Mining);
    assert(CanResearchTech(world, settlements, polities.front(), TechId::BronzeWorking));
}

void TestSailingRequiresCoastalAccess() {
    using namespace oikumene;
    World inland = MakeWorld();
    World coast = MakeWorld();
    coast.At(5, 5).is_coast = true;
    coast.At(5, 5).biome = Biome::Coast;
    std::vector<Settlement> settlements{MakeVillage(0, 5, 5)};
    Assign(settlements[0], 0, true);
    std::vector<Polity> polities{MakePolity(0)};

    assert(!CanResearchTech(inland, settlements, polities.front(), TechId::Sailing));
    assert(CanResearchTech(coast, settlements, polities.front(), TechId::Sailing));
}

void TestResearchChoiceRespondsToFoodPressure() {
    using namespace oikumene;
    World world = MakeWorld();
    std::vector<Settlement> settlements{MakeVillage(0, 5, 5)};
    Assign(settlements[0], 0, true);
    settlements[0].stockpile.food = 4.0F;
    std::vector<Polity> polities{MakePolity(0)};
    polities.front().budget.food_surplus = -1.0F;

    assert(ChooseResearchFor(world, settlements, polities.front()) == TechId::Pottery);
}

void TestResearchChoiceRespondsToOverextension() {
    using namespace oikumene;
    World world = MakeWorld();
    std::vector<Settlement> settlements{MakeVillage(0, 5, 5)};
    Assign(settlements[0], 0, true);
    std::vector<Polity> polities{MakePolity(0)};
    polities.front().overextension = 0.6F;
    polities.front().research.unlocked = {TechId::Pottery, TechId::Roads};

    assert(ChooseResearchFor(world, settlements, polities.front()) == TechId::Administration);
}

void TestResearchChoiceRespondsToMinePotential() {
    using namespace oikumene;
    World world = MakeWorld();
    world.At(6, 5).resource = ResourceKind::ShallowIron;
    std::vector<Settlement> settlements{MakeVillage(0, 5, 5)};
    Assign(settlements[0], 0, true);
    std::vector<Polity> polities{MakePolity(0)};

    assert(ChooseResearchFor(world, settlements, polities.front()) == TechId::Mining);
}

void TestTechnologyIsDeterministicForSameSeed() {
    using namespace oikumene;
    Simulation left(WorldGenerator::Generate(WorldGenerationParams{.seed = 42}), SimulationParams{});
    Simulation right(WorldGenerator::Generate(WorldGenerationParams{.seed = 42}), SimulationParams{});
    left.InitializeBands(8);
    right.InitializeBands(8);
    for (int i = 0; i < 240; ++i) {
        left.AdvanceOneTurn();
        right.AdvanceOneTurn();
    }
    assert(left.Polities().size() == right.Polities().size());
    for (std::size_t i = 0; i < left.Polities().size(); ++i) {
        assert(left.Polities()[i].research.unlocked == right.Polities()[i].research.unlocked);
        assert(left.Polities()[i].research.current == right.Polities()[i].research.current);
    }
}

}  // namespace

int main() {
    TestPolityAccumulatesKnowledgeAndUnlocksTech();
    TestTechPrerequisitesAreRespected();
    TestBronzeWorkingRequiresMiningAndMetals();
    TestSailingRequiresCoastalAccess();
    TestResearchChoiceRespondsToFoodPressure();
    TestResearchChoiceRespondsToOverextension();
    TestResearchChoiceRespondsToMinePotential();
    TestTechnologyIsDeterministicForSameSeed();

    std::cout << "oikumene_technology_tests passed\n";
    return 0;
}
