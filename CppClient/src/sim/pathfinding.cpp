#include "oikumene/sim/pathfinding.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace oikumene {
namespace {

struct QueueNode {
    int index = 0;
    float cost = 0.0F;
};

struct QueueNodeGreater {
    bool operator()(const QueueNode& lhs, const QueueNode& rhs) const {
        return lhs.cost > rhs.cost;
    }
};

int TileIndex(const World& world, int x, int y) {
    return y * world.Width() + x;
}

TileCoord CoordForIndex(const World& world, int index) {
    return TileCoord{.x = index % world.Width(), .y = index / world.Width()};
}

} // namespace

float RouteTerrainCost(const Tile& tile, const PathOptions& options) {
    if ((tile.is_ocean || tile.is_lake) && !options.allow_ocean) {
        return std::numeric_limits<float>::infinity();
    }

    float cost = std::max(0.55F, tile.movement_cost);
    switch (tile.biome) {
    case Biome::Ocean:
    case Biome::Lake:
        cost += options.allow_ocean ? 3.0F : 1.0e6F;
        break;
    case Biome::River:
    case Biome::Coast:
    case Biome::Grassland:
        cost -= 0.18F;
        break;
    case Biome::Forest:
        cost += 0.35F;
        break;
    case Biome::Rainforest:
        cost += 0.70F;
        break;
    case Biome::Wetland:
        cost += 0.85F;
        break;
    case Biome::Hill:
        cost += 0.55F;
        break;
    case Biome::Desert:
    case Biome::Tundra:
        cost += 0.95F;
        break;
    case Biome::Snow:
        cost += 1.65F;
        break;
    case Biome::Mountain:
        cost += 3.80F;
        break;
    }

    if (tile.has_river || tile.is_coast) {
        cost -= 0.20F;
    }
    if (options.use_existing_routes && tile.has_route &&
        (options.route_polity_id == kInvalidPolityId || tile.route_polity_id == options.route_polity_id)) {
        cost *= RouteTravelMultiplier(tile.route_kind);
    }
    return std::max(0.30F, cost);
}

PathResult FindPath(const World& world, TileCoord start, TileCoord goal, const PathOptions& options) {
    if (!world.InBounds(start.x, start.y) || !world.InBounds(goal.x, goal.y)) {
        return {};
    }
    const int tile_count = static_cast<int>(world.Tiles().size());
    const int start_index = TileIndex(world, start.x, start.y);
    const int goal_index = TileIndex(world, goal.x, goal.y);

    std::vector<float> distances(static_cast<std::size_t>(tile_count), std::numeric_limits<float>::infinity());
    std::vector<int> previous(static_cast<std::size_t>(tile_count), -1);
    std::priority_queue<QueueNode, std::vector<QueueNode>, QueueNodeGreater> queue;
    distances[static_cast<std::size_t>(start_index)] = 0.0F;
    queue.push(QueueNode{.index = start_index, .cost = 0.0F});

    const int dxs[] = {1, -1, 0, 0};
    const int dys[] = {0, 0, 1, -1};

    while (!queue.empty()) {
        const QueueNode node = queue.top();
        queue.pop();
        if (node.cost > distances[static_cast<std::size_t>(node.index)] || node.cost > options.max_cost) {
            continue;
        }
        if (node.index == goal_index) {
            break;
        }

        const TileCoord coord = CoordForIndex(world, node.index);
        for (int i = 0; i < 4; ++i) {
            const int nx = coord.x + dxs[i];
            const int ny = coord.y + dys[i];
            if (!world.InBounds(nx, ny)) {
                continue;
            }
            const int next_index = TileIndex(world, nx, ny);
            const float step = RouteTerrainCost(world.At(nx, ny), options);
            if (!std::isfinite(step)) {
                continue;
            }
            const float next_cost = node.cost + step;
            if (next_cost < distances[static_cast<std::size_t>(next_index)] && next_cost <= options.max_cost) {
                distances[static_cast<std::size_t>(next_index)] = next_cost;
                previous[static_cast<std::size_t>(next_index)] = node.index;
                queue.push(QueueNode{.index = next_index, .cost = next_cost});
            }
        }
    }

    if (!std::isfinite(distances[static_cast<std::size_t>(goal_index)])) {
        return {};
    }

    std::vector<TileCoord> path;
    for (int cursor = goal_index; cursor >= 0; cursor = previous[static_cast<std::size_t>(cursor)]) {
        path.push_back(CoordForIndex(world, cursor));
        if (cursor == start_index) {
            break;
        }
    }
    std::reverse(path.begin(), path.end());
    return PathResult{.reachable = true, .total_cost = distances[static_cast<std::size_t>(goal_index)], .path = path};
}

int PathTileCount(const PathResult& path) {
    return static_cast<int>(path.path.size());
}

} // namespace oikumene
