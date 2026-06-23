#include "oikumene/sim/control_field.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <vector>

namespace oikumene {
namespace {

constexpr float kBlockedCost = 1.0e9F;

int TileIndex(const World& world, int x, int y) {
    return y * world.Width() + x;
}

const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id) {
    for (const auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

bool IsLand(const Tile& tile) {
    return !tile.is_ocean && !tile.is_lake;
}

float SourcePowerFor(const Settlement& settlement, bool capital) {
    if (capital) {
        return 38.0F + static_cast<float>(settlement.population) * 0.045F;
    }
    if (settlement.level == SettlementLevel::Village) {
        return 18.0F + static_cast<float>(settlement.population) * 0.025F;
    }
    return 0.0F;
}

struct QueueNode {
    int index = 0;
    float cost = 0.0F;
};

struct QueueNodeGreater {
    bool operator()(const QueueNode& lhs, const QueueNode& rhs) const {
        return lhs.cost > rhs.cost;
    }
};

void UpdateBestControl(std::vector<float>& best,
                       std::vector<float>& second_best,
                       std::vector<PolityId>& best_polity,
                       int index,
                       PolityId polity_id,
                       float strength) {
    if (strength > best[static_cast<std::size_t>(index)]) {
        if (best_polity[static_cast<std::size_t>(index)] != polity_id) {
            second_best[static_cast<std::size_t>(index)] = best[static_cast<std::size_t>(index)];
        }
        best[static_cast<std::size_t>(index)] = strength;
        best_polity[static_cast<std::size_t>(index)] = polity_id;
    } else if (polity_id != best_polity[static_cast<std::size_t>(index)] &&
               strength > second_best[static_cast<std::size_t>(index)]) {
        second_best[static_cast<std::size_t>(index)] = strength;
    }
}

void DiffuseFromSource(const World& world,
                       const Settlement& source,
                       PolityId polity_id,
                       float source_power,
                       float max_cost,
                       std::vector<float>& best,
                       std::vector<float>& second_best,
                       std::vector<PolityId>& best_polity) {
    if (source_power <= 0.0F) {
        return;
    }

    const int tile_count = static_cast<int>(world.Tiles().size());
    std::vector<float> distances(static_cast<std::size_t>(tile_count), std::numeric_limits<float>::infinity());
    std::priority_queue<QueueNode, std::vector<QueueNode>, QueueNodeGreater> queue;

    const int start = TileIndex(world, source.x, source.y);
    distances[static_cast<std::size_t>(start)] = 0.0F;
    queue.push(QueueNode{.index = start, .cost = 0.0F});

    const int dxs[] = {1, -1, 0, 0};
    const int dys[] = {0, 0, 1, -1};

    while (!queue.empty()) {
        const QueueNode node = queue.top();
        queue.pop();
        if (node.cost > distances[static_cast<std::size_t>(node.index)] || node.cost > max_cost) {
            continue;
        }

        const auto& tile = world.Tiles()[static_cast<std::size_t>(node.index)];
        if (!IsLand(tile)) {
            continue;
        }

        const float strength = source_power - node.cost;
        if (strength > 0.0F) {
            UpdateBestControl(best, second_best, best_polity, node.index, polity_id, strength);
        }

        for (int i = 0; i < 4; ++i) {
            const int nx = tile.x + dxs[i];
            const int ny = tile.y + dys[i];
            if (!world.InBounds(nx, ny)) {
                continue;
            }
            const auto& neighbor = world.At(nx, ny);
            const float step = TerrainControlCost(neighbor);
            if (step >= kBlockedCost) {
                continue;
            }
            const int next_index = TileIndex(world, nx, ny);
            const float next_cost = node.cost + step;
            if (next_cost < distances[static_cast<std::size_t>(next_index)] && next_cost <= max_cost) {
                distances[static_cast<std::size_t>(next_index)] = next_cost;
                queue.push(QueueNode{.index = next_index, .cost = next_cost});
            }
        }
    }
}

}  // namespace

float ControlFieldStats::ControlledLandRatio() const {
    return land_tiles <= 0 ? 0.0F : static_cast<float>(controlled_land_tiles) / static_cast<float>(land_tiles);
}

float TerrainControlCost(const Tile& tile) {
    if (tile.is_ocean || tile.is_lake) {
        return kBlockedCost;
    }

    float cost = std::max(0.65F, tile.movement_cost);
    switch (tile.biome) {
        case Biome::Grassland:
        case Biome::Coast:
        case Biome::River:
            cost -= 0.18F;
            break;
        case Biome::Forest:
            cost += 0.35F;
            break;
        case Biome::Rainforest:
            cost += 0.65F;
            break;
        case Biome::Wetland:
            cost += 0.90F;
            break;
        case Biome::Desert:
        case Biome::Tundra:
            cost += 0.80F;
            break;
        case Biome::Snow:
            cost += 1.60F;
            break;
        case Biome::Hill:
            cost += 0.55F;
            break;
        case Biome::Mountain:
            cost += 3.50F;
            break;
        case Biome::Ocean:
        case Biome::Lake:
            return kBlockedCost;
    }

    if (tile.has_river || tile.is_coast) {
        cost -= 0.22F;
    }
    return std::max(0.35F, cost);
}

float TerrainPathCost(const World& world, int start_x, int start_y, int end_x, int end_y, float max_cost) {
    if (!world.InBounds(start_x, start_y) || !world.InBounds(end_x, end_y)) {
        return std::numeric_limits<float>::infinity();
    }
    if (start_x == end_x && start_y == end_y) {
        return 0.0F;
    }

    const int tile_count = static_cast<int>(world.Tiles().size());
    std::vector<float> distances(static_cast<std::size_t>(tile_count), std::numeric_limits<float>::infinity());
    std::priority_queue<QueueNode, std::vector<QueueNode>, QueueNodeGreater> queue;
    const int start = TileIndex(world, start_x, start_y);
    const int goal = TileIndex(world, end_x, end_y);
    distances[static_cast<std::size_t>(start)] = 0.0F;
    queue.push(QueueNode{.index = start, .cost = 0.0F});

    const int dxs[] = {1, -1, 0, 0};
    const int dys[] = {0, 0, 1, -1};

    while (!queue.empty()) {
        const QueueNode node = queue.top();
        queue.pop();
        if (node.index == goal) {
            return node.cost;
        }
        if (node.cost > distances[static_cast<std::size_t>(node.index)] || node.cost > max_cost) {
            continue;
        }

        const auto& tile = world.Tiles()[static_cast<std::size_t>(node.index)];
        for (int i = 0; i < 4; ++i) {
            const int nx = tile.x + dxs[i];
            const int ny = tile.y + dys[i];
            if (!world.InBounds(nx, ny)) {
                continue;
            }
            const float step = TerrainControlCost(world.At(nx, ny));
            if (step >= kBlockedCost) {
                continue;
            }
            const int next_index = TileIndex(world, nx, ny);
            const float next_cost = node.cost + step;
            if (next_cost < distances[static_cast<std::size_t>(next_index)] && next_cost <= max_cost) {
                distances[static_cast<std::size_t>(next_index)] = next_cost;
                queue.push(QueueNode{.index = next_index, .cost = next_cost});
            }
        }
    }

    return std::numeric_limits<float>::infinity();
}

ControlFieldStats RecomputeControlField(World& world,
                                        const std::vector<Settlement>& settlements,
                                        std::vector<Polity>& polities,
                                        const ControlFieldParams& params) {
    const int tile_count = static_cast<int>(world.Tiles().size());
    std::vector<float> best(static_cast<std::size_t>(tile_count), 0.0F);
    std::vector<float> second_best(static_cast<std::size_t>(tile_count), 0.0F);
    std::vector<PolityId> best_polity(static_cast<std::size_t>(tile_count), kInvalidPolityId);

    for (auto& tile : world.Tiles()) {
        tile.controller_polity_id = kInvalidPolityId;
        tile.control_strength = 0.0F;
        tile.is_contested = false;
    }
    for (auto& polity : polities) {
        polity.controlled_tile_count = 0;
        polity.contested_tile_count = 0;
    }

    for (const auto& polity : polities) {
        if (const auto* capital = SettlementById(settlements, polity.capital_settlement_id)) {
            DiffuseFromSource(world, *capital, polity.id, SourcePowerFor(*capital, true),
                              std::min(params.max_path_cost, polity.admin_range * 1.15F), best, second_best, best_polity);
        }
        for (const int settlement_id : polity.member_settlement_ids) {
            if (settlement_id == polity.capital_settlement_id) {
                continue;
            }
            if (const auto* settlement = SettlementById(settlements, settlement_id)) {
                DiffuseFromSource(world, *settlement, polity.id, SourcePowerFor(*settlement, false),
                                  std::min(params.max_path_cost, polity.admin_range * 0.90F), best, second_best,
                                  best_polity);
            }
        }
    }

    ControlFieldStats stats;
    for (int index = 0; index < tile_count; ++index) {
        auto& tile = world.Tiles()[static_cast<std::size_t>(index)];
        if (!IsLand(tile)) {
            continue;
        }
        ++stats.land_tiles;
        const float top = best[static_cast<std::size_t>(index)];
        const float second = second_best[static_cast<std::size_t>(index)];
        if (top < params.min_control_strength) {
            continue;
        }
        tile.controller_polity_id = best_polity[static_cast<std::size_t>(index)];
        tile.control_strength = top;
        tile.is_contested = second >= params.min_control_strength * 0.70F && top - second <= params.contested_margin;
        ++stats.controlled_land_tiles;
        if (tile.is_contested) {
            ++stats.contested_tiles;
        }
    }

    for (const auto& tile : world.Tiles()) {
        if (tile.controller_polity_id == kInvalidPolityId) {
            continue;
        }
        for (auto& polity : polities) {
            if (polity.id == tile.controller_polity_id) {
                ++polity.controlled_tile_count;
                polity.contested_tile_count += tile.is_contested ? 1 : 0;
                break;
            }
        }
    }

    return stats;
}

}  // namespace oikumene
