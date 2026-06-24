#include <cassert>
#include <iostream>

#include "oikumene/sim/polity_system.hpp"

namespace {

oikumene::World MakeWorld(int width = 32, int height = 12) {
    using namespace oikumene;
    World world(width, height, 31);
    for (auto& tile : world.Tiles()) {
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.biome = Biome::Grassland;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.60F;
    }
    return world;
}

oikumene::Settlement MakeVillage(int id, int x, int y, int population = 160) {
    oikumene::Settlement settlement;
    settlement.id = id;
    settlement.x = x;
    settlement.y = y;
    settlement.population = population;
    settlement.level = oikumene::SettlementLevel::Village;
    settlement.stockpile.food = 120.0F;
    settlement.stockpile.wood = 60.0F;
    settlement.stockpile.ore = 10.0F;
    settlement.local_food_output_last_turn = 34.0F;
    settlement.local_wood_output_last_turn = 10.0F;
    settlement.ore_output_last_turn = 3.0F;
    settlement.food_consumption_last_turn = 16.0F;
    return settlement;
}

oikumene::Polity MakePolity(oikumene::PolityId id, int capital_id, std::initializer_list<int> members) {
    oikumene::Polity polity;
    polity.id = id;
    polity.name = "Budget " + std::to_string(id);
    polity.capital_settlement_id = capital_id;
    polity.member_settlement_ids.assign(members.begin(), members.end());
    return polity;
}

void Assign(oikumene::Settlement& settlement, oikumene::PolityId polity_id, bool capital = false) {
    settlement.polity_id = polity_id;
    settlement.is_capital = capital;
}

void TestPolityBudgetAggregatesMemberResources() {
    using namespace oikumene;
    World world = MakeWorld();
    std::vector<Settlement> settlements{MakeVillage(0, 5, 5, 180), MakeVillage(1, 8, 5, 90)};
    Assign(settlements[0], 0, true);
    Assign(settlements[1], 0);
    std::vector<Polity> polities{MakePolity(0, 0, {0, 1})};
    EventLog events;

    PolitySystem::UpdatePolities(world, 120, settlements, polities, events);
    assert(polities.size() == 1);
    assert(polities.front().population == 270);
    assert(polities.front().food > 0.0F);
    assert(polities.front().wood > 0.0F);
    assert(polities.front().budget.food_income > 0.0F);
    assert(polities.front().budget.wood_income > 0.0F);
    assert(polities.front().budget.ore_income > 0.0F);
    assert(polities.front().budget.wealth_income > 0.0F);
}

void TestAdminCapacityDependsOnCapitalPopulation() {
    using namespace oikumene;
    World world = MakeWorld();
    std::vector<Settlement> small{MakeVillage(0, 5, 5, 120)};
    std::vector<Settlement> large{MakeVillage(0, 5, 5, 320)};
    Assign(small[0], 0, true);
    Assign(large[0], 0, true);
    std::vector<Polity> small_polities{MakePolity(0, 0, {0})};
    std::vector<Polity> large_polities{MakePolity(0, 0, {0})};
    EventLog events;

    PolitySystem::UpdatePolities(world, 120, small, small_polities, events);
    PolitySystem::UpdatePolities(world, 120, large, large_polities, events);
    assert(large_polities.front().admin_capacity > small_polities.front().admin_capacity);
}

void TestRemoteSettlementIncreasesAdminLoad() {
    using namespace oikumene;
    World near_world = MakeWorld();
    World far_world = MakeWorld();
    std::vector<Settlement> near_settlements{MakeVillage(0, 5, 5, 180), MakeVillage(1, 8, 5, 90)};
    std::vector<Settlement> far_settlements{MakeVillage(0, 5, 5, 180), MakeVillage(1, 25, 5, 90)};
    Assign(near_settlements[0], 0, true);
    Assign(near_settlements[1], 0);
    Assign(far_settlements[0], 0, true);
    Assign(far_settlements[1], 0);
    std::vector<Polity> near_polities{MakePolity(0, 0, {0, 1})};
    std::vector<Polity> far_polities{MakePolity(0, 0, {0, 1})};
    EventLog events;

    PolitySystem::UpdatePolities(near_world, 120, near_settlements, near_polities, events);
    PolitySystem::UpdatePolities(far_world, 120, far_settlements, far_polities, events);
    assert(far_polities.front().admin_load > near_polities.front().admin_load);
}

void TestOverextensionReducesStability() {
    using namespace oikumene;
    World world = MakeWorld();
    std::vector<Settlement> settlements{
        MakeVillage(0, 3, 5, 130), MakeVillage(1, 10, 5, 80), MakeVillage(2, 16, 5, 80),
        MakeVillage(3, 22, 5, 80), MakeVillage(4, 28, 5, 80),
    };
    for (auto& settlement : settlements) {
        Assign(settlement, 0, settlement.id == 0);
    }
    std::vector<Polity> polities{MakePolity(0, 0, {0, 1, 2, 3, 4})};
    EventLog events;

    PolitySystem::UpdatePolities(world, 120, settlements, polities, events);
    assert(polities.front().admin_load > polities.front().admin_capacity);
    assert(polities.front().overextension > 0.0F);
    assert(polities.front().stability < 1.0F);
}

} // namespace

int main() {
    TestPolityBudgetAggregatesMemberResources();
    TestAdminCapacityDependsOnCapitalPopulation();
    TestRemoteSettlementIncreasesAdminLoad();
    TestOverextensionReducesStability();

    std::cout << "oikumene_polity_economy_tests passed\n";
    return 0;
}
