#include <cassert>
#include <cmath>
#include <iostream>

#include "oikumene/sim/pathfinding.hpp"

namespace {

oikumene::World MakePlainWorld(int width = 12, int height = 7) {
    using namespace oikumene;
    World world(width, height, 71);
    for (auto& tile : world.Tiles()) {
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.biome = Biome::Grassland;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.5F;
    }
    return world;
}

void TestPathfindingAvoidsOceanWithoutSailing() {
    using namespace oikumene;
    World world = MakePlainWorld();
    for (int y = 0; y < world.Height(); ++y) {
        auto& tile = world.At(5, y);
        tile.is_ocean = true;
        tile.biome = Biome::Ocean;
    }

    PathOptions options;
    options.max_cost = 200.0F;
    const PathResult blocked = FindPath(world, TileCoord{.x = 1, .y = 3}, TileCoord{.x = 10, .y = 3}, options);
    assert(!blocked.reachable);

    options.allow_ocean = true;
    const PathResult coastal = FindPath(world, TileCoord{.x = 1, .y = 3}, TileCoord{.x = 10, .y = 3}, options);
    assert(coastal.reachable);
}

void TestMountainsAreMoreExpensiveThanPlainRoute() {
    using namespace oikumene;
    World plain = MakePlainWorld();
    World mountain = MakePlainWorld();
    for (int x = 2; x < 10; ++x) {
        auto& tile = mountain.At(x, 3);
        tile.biome = Biome::Mountain;
        tile.movement_cost = 5.0F;
    }

    PathOptions options;
    options.max_cost = 200.0F;
    const PathResult plain_path = FindPath(plain, TileCoord{.x = 1, .y = 3}, TileCoord{.x = 10, .y = 3}, options);
    const PathResult mountain_path = FindPath(mountain, TileCoord{.x = 1, .y = 3}, TileCoord{.x = 10, .y = 3}, options);
    assert(plain_path.reachable);
    assert(mountain_path.reachable);
    assert(mountain_path.total_cost > plain_path.total_cost);
}

void TestExistingRoadDiscountsSamePolityPath() {
    using namespace oikumene;
    World world = MakePlainWorld();
    PathOptions with_route;
    with_route.max_cost = 200.0F;
    with_route.route_polity_id = 3;
    PathOptions foreign_route = with_route;
    foreign_route.route_polity_id = 4;

    const PathResult before = FindPath(world, TileCoord{.x = 1, .y = 3}, TileCoord{.x = 10, .y = 3}, with_route);
    for (int x = 1; x <= 10; ++x) {
        auto& tile = world.At(x, 3);
        tile.has_route = true;
        tile.route_kind = RouteKind::Road;
        tile.route_polity_id = 3;
        tile.route_quality = RouteQuality(RouteKind::Road);
    }
    const PathResult after = FindPath(world, TileCoord{.x = 1, .y = 3}, TileCoord{.x = 10, .y = 3}, with_route);
    const PathResult foreign = FindPath(world, TileCoord{.x = 1, .y = 3}, TileCoord{.x = 10, .y = 3}, foreign_route);

    assert(before.reachable);
    assert(after.reachable);
    assert(after.total_cost < before.total_cost);
    assert(std::fabs(foreign.total_cost - before.total_cost) < 0.001F);
}

}  // namespace

int main() {
    TestPathfindingAvoidsOceanWithoutSailing();
    TestMountainsAreMoreExpensiveThanPlainRoute();
    TestExistingRoadDiscountsSamePolityPath();

    std::cout << "oikumene_pathfinding_tests passed\n";
    return 0;
}
