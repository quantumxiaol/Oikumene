#include <cassert>
#include <iostream>
#include <vector>

#include "oikumene/sim/route_audit.hpp"
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

void TestDisableRoutesPreventsRouteConstruction() {
    using namespace oikumene;
    World world = MakeRouteWorld();
    std::vector<Settlement> settlements = MakeSettlements();
    std::vector<Polity> polities{MakePolity(true)};
    std::vector<Route> routes;
    EventLog events;

    RouteSystem::UpdateRoutes(world, 20, settlements, polities, routes, events, false);
    assert(routes.empty());
    assert(events.Size() == 0);
    assert(polities.front().route_ids.empty());
    for (const auto& tile : world.Tiles()) {
        assert(!tile.has_route);
    }
}

void TestRouteAdminSavingUsesActualPathCostDifference() {
    using namespace oikumene;
    World world = MakeRouteWorld();
    std::vector<Settlement> settlements = MakeSettlements();
    std::vector<Polity> polities{MakePolity(true)};
    std::vector<Route> routes;
    EventLog events;

    RouteSystem::UpdateRoutes(world, 20, settlements, polities, routes, events);
    const RouteNetworkStats stats = BuildRouteNetworkStats(world, settlements, routes, polities.front());
    assert(stats.route_count == 1);
    assert(stats.admin_distance_saving > 0.0F);
    assert(stats.admin_distance_saving < 20.0F);
}

void TestConnectedMinePotentialDoesNotEqualActiveOreIncome() {
    using namespace oikumene;
    World world = MakeRouteWorld(8, 5);
    auto& tile = world.At(4, 2);
    tile.resource = ResourceKind::Copper;
    tile.resource_amount = 0.8F;
    tile.improvement = ImprovementKind::ShallowMine;

    std::vector<Settlement> settlements{MakeVillage(0, 2, 2, 0, true)};
    std::vector<Route> routes{Route{
        .id = 0,
        .polity_id = 0,
        .from_settlement_id = 0,
        .target_x = 4,
        .target_y = 2,
        .purpose = RoutePurpose::Resource,
        .kind = RouteKind::Road,
        .path = {TileCoord{.x = 2, .y = 2}, TileCoord{.x = 3, .y = 2}, TileCoord{.x = 4, .y = 2}},
    }};
    for (const auto& coord : routes.front().path) {
        auto& route_tile = world.At(coord.x, coord.y);
        route_tile.has_route = true;
        route_tile.route_kind = RouteKind::Road;
        route_tile.route_quality = RouteQuality(RouteKind::Road);
        route_tile.route_polity_id = 0;
        route_tile.route_id = 0;
    }
    Polity polity = MakePolity(true);
    polity.member_settlement_ids = {0};

    const RouteNetworkStats before_mining = BuildRouteNetworkStats(world, settlements, routes, polity);
    assert(before_mining.connected_mine_potential == 1);
    assert(before_mining.active_connected_mines == 0);
    assert(before_mining.connected_ore_income == 0.0F);

    settlements.front().worked_tile_indices = {2 * world.Width() + 4};
    polity.research.unlocked.push_back(TechId::Mining);
    const RouteNetworkStats after_mining = BuildRouteNetworkStats(world, settlements, routes, polity);
    assert(after_mining.connected_mine_potential == 1);
    assert(after_mining.active_connected_mines == 1);
    assert(after_mining.connected_ore_income > 0.0F);
}

void TestCoastalRouteCanRepresentCoastalSegments() {
    using namespace oikumene;
    World world = MakeRouteWorld(6, 3);
    for (int y = 0; y < world.Height(); ++y) {
        for (int x = 2; x <= 3; ++x) {
            auto& tile = world.At(x, y);
            tile.is_ocean = true;
            tile.biome = Biome::Ocean;
            tile.movement_cost = 1.0F;
        }
    }
    auto& target = world.At(4, 1);
    target.is_coast = true;
    target.biome = Biome::Coast;
    target.resource = ResourceKind::Gold;
    target.resource_amount = 0.7F;

    std::vector<Settlement> settlements{MakeVillage(0, 1, 1, 0, true)};
    std::vector<Polity> polities{MakePolity(false)};
    polities.front().member_settlement_ids = {0};
    polities.front().research.unlocked.push_back(TechId::Sailing);
    std::vector<Route> routes;
    EventLog events;

    RouteSystem::UpdateRoutes(world, 20, settlements, polities, routes, events);
    assert(routes.size() == 1);
    assert(routes.front().kind == RouteKind::CoastalRoute);
    assert(world.At(2, 1).has_route || world.At(3, 1).has_route);
}

} // namespace

int main() {
    TestTrailBuildsBeforeRoadsTech();
    TestRoadBuildsAfterRoadsTech();
    TestRouteNetworkIsDeterministic();
    TestDisableRoutesPreventsRouteConstruction();
    TestRouteAdminSavingUsesActualPathCostDifference();
    TestConnectedMinePotentialDoesNotEqualActiveOreIncome();
    TestCoastalRouteCanRepresentCoastalSegments();

    std::cout << "oikumene_route_network_tests passed\n";
    return 0;
}
