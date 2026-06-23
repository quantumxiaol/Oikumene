#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

struct TileCoord {
    int x = 0;
    int y = 0;
};

struct PathOptions {
    bool allow_ocean = false;
    bool use_existing_routes = true;
    PolityId route_polity_id = kInvalidPolityId;
    float max_cost = 1.0e9F;
};

struct PathResult {
    bool reachable = false;
    float total_cost = 0.0F;
    std::vector<TileCoord> path;
};

[[nodiscard]] float RouteTerrainCost(const Tile& tile, const PathOptions& options);
[[nodiscard]] PathResult FindPath(const World& world, TileCoord start, TileCoord goal, const PathOptions& options = {});
[[nodiscard]] int PathTileCount(const PathResult& path);

}  // namespace oikumene
