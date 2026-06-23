#include <cassert>
#include <iostream>

#include "oikumene/sim/control_field.hpp"
#include "oikumene/sim/polity_system.hpp"
#include "oikumene/sim/settlement_system.hpp"

namespace {

oikumene::World MakePlainWorld(int width = 18, int height = 7) {
    using namespace oikumene;
    World world(width, height, 97);
    for (auto& tile : world.Tiles()) {
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.biome = Biome::Grassland;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.55F;
    }
    return world;
}

oikumene::Settlement MakeVillage(int id, int x, int y, bool capital = false) {
    oikumene::Settlement settlement;
    settlement.id = id;
    settlement.x = x;
    settlement.y = y;
    settlement.level = oikumene::SettlementLevel::Village;
    settlement.population = capital ? 220 : 130;
    settlement.polity_id = 0;
    settlement.is_capital = capital;
    settlement.stockpile.food = 180.0F;
    settlement.stockpile.wood = 80.0F;
    settlement.local_food_output_last_turn = 30.0F;
    settlement.local_wood_output_last_turn = 8.0F;
    settlement.food_consumption_last_turn = 16.0F;
    return settlement;
}

oikumene::Polity MakePolity() {
    oikumene::Polity polity;
    polity.id = 0;
    polity.name = "Effect Test";
    polity.capital_settlement_id = 0;
    polity.member_settlement_ids = {0, 1};
    polity.stability = 1.0F;
    polity.admin_range = 32.0F;
    return polity;
}

void AddRoad(oikumene::World& world, int y = 3) {
    using namespace oikumene;
    for (int x = 2; x <= 13; ++x) {
        auto& tile = world.At(x, y);
        tile.has_route = true;
        tile.route_kind = RouteKind::Road;
        tile.route_quality = RouteQuality(RouteKind::Road);
        tile.route_polity_id = 0;
        tile.route_id = 0;
    }
}

void TestRouteReducesAdminDistanceLoad() {
    using namespace oikumene;
    World plain = MakePlainWorld();
    World roaded = MakePlainWorld();
    AddRoad(roaded);
    std::vector<Settlement> plain_settlements{MakeVillage(0, 2, 3, true), MakeVillage(1, 13, 3)};
    std::vector<Settlement> road_settlements = plain_settlements;
    std::vector<Polity> plain_polities{MakePolity()};
    std::vector<Polity> road_polities{MakePolity()};
    EventLog events;

    PolitySystem::UpdatePolities(plain, 140, plain_settlements, plain_polities, events);
    PolitySystem::UpdatePolities(roaded, 140, road_settlements, road_polities, events);
    assert(road_polities.front().admin_load < plain_polities.front().admin_load);
}

void TestRouteStrengthensControlAlongPath() {
    using namespace oikumene;
    World plain = MakePlainWorld();
    World roaded = MakePlainWorld();
    AddRoad(roaded);
    std::vector<Settlement> settlements{MakeVillage(0, 2, 3, true)};
    std::vector<Polity> plain_polities{MakePolity()};
    plain_polities.front().member_settlement_ids = {0};
    std::vector<Polity> road_polities = plain_polities;

    const auto plain_stats = RecomputeControlField(plain, settlements, plain_polities);
    const auto road_stats = RecomputeControlField(roaded, settlements, road_polities);
    assert(plain_stats.controlled_land_tiles > 0);
    assert(road_stats.controlled_land_tiles > 0);
    assert(roaded.At(13, 3).control_strength > plain.At(13, 3).control_strength);
}

void TestRouteIncreasesMineOreTransfer() {
    using namespace oikumene;
    World plain = MakePlainWorld(7, 5);
    World roaded = MakePlainWorld(7, 5);
    auto configure_mine = [](World& world) {
        auto& tile = world.At(3, 2);
        tile.resource = ResourceKind::Copper;
        tile.resource_amount = 0.9F;
        tile.improvement = ImprovementKind::ShallowMine;
        tile.worked_by_settlement_id = 0;
    };
    configure_mine(plain);
    configure_mine(roaded);
    auto& route_tile = roaded.At(3, 2);
    route_tile.has_route = true;
    route_tile.route_kind = RouteKind::Road;
    route_tile.route_quality = RouteQuality(RouteKind::Road);
    route_tile.route_polity_id = 0;
    route_tile.route_id = 0;

    std::vector<Settlement> plain_settlements{MakeVillage(0, 2, 2, true)};
    std::vector<Settlement> road_settlements = plain_settlements;
    std::vector<Polity> polities{MakePolity()};
    polities.front().member_settlement_ids = {0};
    polities.front().research.unlocked.push_back(TechId::Mining);
    EventLog events;

    SimulationParams params;
    SettlementSystem::UpdateSettlements(plain, params, 80, plain_settlements, polities, events);
    SettlementSystem::UpdateSettlements(roaded, params, 80, road_settlements, polities, events);
    assert(road_settlements.front().ore_output_last_turn > plain_settlements.front().ore_output_last_turn);
}

}  // namespace

int main() {
    TestRouteReducesAdminDistanceLoad();
    TestRouteStrengthensControlAlongPath();
    TestRouteIncreasesMineOreTransfer();

    std::cout << "oikumene_route_effects_tests passed\n";
    return 0;
}
