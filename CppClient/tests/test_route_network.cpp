#include <cassert>
#include <iostream>
#include <vector>

#include "oikumene/sim/route_system.hpp"

namespace {

oikumene::World MakeRouteWorld(int width = 18, int height = 7) {
    using namespace oikumene;
    World world(width, height, 83);
    for (auto& tile : world.Tiles()) {
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.biome = Biome::Grassland;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.55F;
    }
    return world;
}

oikumene::Settlement MakeVillage(int id, int x, int y, oikumene::PolityId polity_id, bool capital = false) {
    oikumene::Settlement settlement;
    settlement.id = id;
    settlement.x = x;
    settlement.y = y;
    settlement.level = oikumene::SettlementLevel::Village;
    settlement.population = capital ? 220 : 120;
    settlement.polity_id = polity_id;
    settlement.is_capital = capital;
    settlement.stockpile.food = 160.0F;
    settlement.stockpile.wood = 120.0F;
    settlement.stockpile.wealth = 30.0F;
    return settlement;
}

oikumene::Polity MakePolity(bool roads_unlocked = false) {
    using namespace oikumene;
    Polity polity;
    polity.id = 0;
    polity.name = "Route Test";
    polity.capital_settlement_id = 0;
    polity.member_settlement_ids = {0, 1};
    polity.stability = 1.0F;
    polity.budget.wood_surplus = 4.0F;
    polity.budget.wealth_surplus = 2.0F;
    if (roads_unlocked) {
        polity.research.unlocked.push_back(TechId::Pottery);
        polity.research.unlocked.push_back(TechId::Roads);
    }
    return polity;
}

std::vector<oikumene::Settlement> MakeSettlements() {
    return {
        MakeVillage(0, 2, 3, 0, true),
        MakeVillage(1, 13, 3, 0, false),
    };
}

void TestTrailBuildsBeforeRoadsTech() {
    using namespace oikumene;
    World world = MakeRouteWorld();
    std::vector<Settlement> settlements = MakeSettlements();
    std::vector<Polity> polities{MakePolity(false)};
    std::vector<Route> routes;
    EventLog events;

    RouteSystem::UpdateRoutes(world, 20, settlements, polities, routes, events);
    assert(routes.size() == 1);
    assert(routes.front().kind == RouteKind::Trail);
    assert(routes.front().purpose == RoutePurpose::SettlementLink);
    assert(polities.front().connected_settlements == 1);
    assert(polities.front().route_maintenance > 0.0F);
    assert(events.Size() == 1);
}

void TestRoadBuildsAfterRoadsTech() {
    using namespace oikumene;
    World world = MakeRouteWorld();
    std::vector<Settlement> settlements = MakeSettlements();
    std::vector<Polity> polities{MakePolity(true)};
    std::vector<Route> routes;
    EventLog events;

    RouteSystem::UpdateRoutes(world, 20, settlements, polities, routes, events);
    assert(routes.size() == 1);
    assert(routes.front().kind == RouteKind::Road);
    assert(polities.front().route_ids.size() == 1);
    const RouteNetworkStats stats = BuildRouteNetworkStats(world, routes, polities.front());
    assert(stats.route_count == 1);
    assert(stats.road_tiles > 0);
    assert(stats.trail_tiles == 0);
}

void TestRouteNetworkIsDeterministic() {
    using namespace oikumene;
    World world_a = MakeRouteWorld();
    World world_b = MakeRouteWorld();
    std::vector<Settlement> settlements_a = MakeSettlements();
    std::vector<Settlement> settlements_b = MakeSettlements();
    std::vector<Polity> polities_a{MakePolity(true)};
    std::vector<Polity> polities_b{MakePolity(true)};
    std::vector<Route> routes_a;
    std::vector<Route> routes_b;
    EventLog events_a;
    EventLog events_b;

    RouteSystem::UpdateRoutes(world_a, 20, settlements_a, polities_a, routes_a, events_a);
    RouteSystem::UpdateRoutes(world_b, 20, settlements_b, polities_b, routes_b, events_b);
    assert(routes_a.size() == routes_b.size());
    assert(routes_a.front().kind == routes_b.front().kind);
    assert(routes_a.front().path.size() == routes_b.front().path.size());
    for (std::size_t i = 0; i < routes_a.front().path.size(); ++i) {
        assert(routes_a.front().path[i].x == routes_b.front().path[i].x);
        assert(routes_a.front().path[i].y == routes_b.front().path[i].y);
    }
}

}  // namespace

int main() {
    TestTrailBuildsBeforeRoadsTech();
    TestRoadBuildsAfterRoadsTech();
    TestRouteNetworkIsDeterministic();

    std::cout << "oikumene_route_network_tests passed\n";
    return 0;
}
