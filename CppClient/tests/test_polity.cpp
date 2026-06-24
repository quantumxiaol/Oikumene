#include <cassert>
#include <iostream>

#include "oikumene/sim/polity_system.hpp"

namespace {

oikumene::World MakePolityWorld(int width = 24, int height = 12) {
    using namespace oikumene;
    World world(width, height, 23);
    for (auto& tile : world.Tiles()) {
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.biome = Biome::Grassland;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.55F;
    }
    return world;
}

oikumene::Settlement MakeSettlement(int id, int x, int y, int population, float food) {
    oikumene::Settlement settlement;
    settlement.id = id;
    settlement.x = x;
    settlement.y = y;
    settlement.population = population;
    settlement.level = oikumene::SettlementLevel::Village;
    settlement.stockpile.food = food;
    settlement.stockpile.wood = 40.0F;
    return settlement;
}

void TestVillageCanFoundChiefdom() {
    using namespace oikumene;
    World world = MakePolityWorld();
    std::vector<Settlement> settlements{MakeSettlement(0, 5, 5, 140, 100.0F)};
    std::vector<Polity> polities;
    EventLog events;

    PolitySystem::UpdatePolities(world, 80, settlements, polities, events);
    assert(polities.size() == 1);
    assert(polities.front().level == PolityLevel::Chiefdom);
    assert(polities.front().capital_settlement_id == 0);
    assert(settlements.front().is_capital);
    assert(settlements.front().polity_id == polities.front().id);
}

void TestNearbyVillageJoinsChiefdom() {
    using namespace oikumene;
    World world = MakePolityWorld();
    std::vector<Settlement> settlements{
        MakeSettlement(0, 5, 5, 140, 100.0F),
        MakeSettlement(1, 9, 5, 70, 40.0F),
    };
    std::vector<Polity> polities;
    EventLog events;

    PolitySystem::UpdatePolities(world, 80, settlements, polities, events);
    assert(polities.size() == 1);
    assert(settlements[1].polity_id == polities.front().id);
    assert(polities.front().member_settlement_ids.size() == 2);
}

void TestDistantVillageDoesNotJoinChiefdom() {
    using namespace oikumene;
    World world = MakePolityWorld(80, 20);
    std::vector<Settlement> settlements{
        MakeSettlement(0, 5, 5, 140, 100.0F),
        MakeSettlement(1, 70, 5, 80, 40.0F),
    };
    std::vector<Polity> polities;
    EventLog events;

    PolitySystem::UpdatePolities(world, 80, settlements, polities, events);
    assert(polities.size() == 1);
    assert(settlements[1].polity_id == kInvalidPolityId);
}

void TestRegenerateWorldClearsPolities() {
    using namespace oikumene;
    World world = MakePolityWorld();
    std::vector<Settlement> settlements{MakeSettlement(0, 5, 5, 140, 100.0F)};
    std::vector<Polity> polities;
    EventLog events;
    PolitySystem::UpdatePolities(world, 80, settlements, polities, events);
    assert(!polities.empty());

    PolitySystem::Reset(world, settlements, polities);
    assert(polities.empty());
    assert(settlements.front().polity_id == kInvalidPolityId);
    assert(!settlements.front().is_capital);
    for (const auto& tile : world.Tiles()) {
        assert(tile.controller_polity_id == kInvalidPolityId);
        assert(!tile.is_contested);
    }
}

} // namespace

int main() {
    TestVillageCanFoundChiefdom();
    TestNearbyVillageJoinsChiefdom();
    TestDistantVillageDoesNotJoinChiefdom();
    TestRegenerateWorldClearsPolities();

    std::cout << "oikumene_polity_tests passed\n";
    return 0;
}
