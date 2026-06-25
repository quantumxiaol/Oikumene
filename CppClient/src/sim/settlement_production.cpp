#include "settlement_production.hpp"

#include <algorithm>
#include <sstream>

#include "settlement_work.hpp"

namespace oikumene::settlement_system {
namespace {

float WorkedTileScore(const Tile& tile, const Settlement& settlement, const TechEffects& effects) {
    switch (tile.improvement) {
    case ImprovementKind::Farm:
        return 4.0F + tile.soil_quality * 5.0F;
    case ImprovementKind::LumberCamp:
        if (settlement.stockpile.food < static_cast<float>(settlement.population) * 0.35F) {
            return 2.4F + tile.forest_cover * 0.5F;
        }
        return settlement.stockpile.wood < 45.0F ? 7.0F : 5.0F + tile.forest_cover * 0.8F;
    case ImprovementKind::Pasture:
        return 2.6F + tile.fertility + (tile.resource == ResourceKind::Horse ? 2.5F : 0.0F);
    case ImprovementKind::ShallowMine:
        return effects.mining_enabled ? 8.0F + tile.resource_amount * 3.0F : 1.2F + tile.resource_amount;
    case ImprovementKind::ForagingGround:
        return 1.5F + tile.fertility * 2.0F;
    case ImprovementKind::None:
    case ImprovementKind::Road:
        break;
    }
    return 0.0F;
}

} // namespace

float CarryingCapacityFor(const World& world, const Settlement& settlement, const TechEffects& effects) {
    float capacity = settlement.level == SettlementLevel::Village ? 65.0F : 42.0F;
    const auto& center = world.At(settlement.x, settlement.y);
    if (center.has_river || center.is_coast) {
        capacity += 18.0F;
    }

    for (const int index : NearbyTileIndices(world, settlement, WorkRadiusFor(settlement))) {
        const auto& tile = TileByIndex(world, index);
        if (tile.worked_by_settlement_id != settlement.id) {
            continue;
        }
        switch (tile.improvement) {
        case ImprovementKind::Farm:
            capacity += (16.0F + tile.soil_quality * 12.0F + (tile.has_river ? 4.0F : 0.0F)) *
                        (tile.has_river ? effects.river_farm_output_multiplier : 1.0F);
            break;
        case ImprovementKind::Pasture:
            capacity += (12.0F + tile.fertility * 6.0F +
                         (tile.resource == ResourceKind::Horse ? 5.0F * effects.horse_value_multiplier : 0.0F)) *
                        effects.pasture_output_multiplier;
            break;
        case ImprovementKind::ForagingGround:
            capacity += 6.0F + tile.fertility * 5.0F;
            break;
        case ImprovementKind::LumberCamp:
            capacity += 3.0F;
            break;
        case ImprovementKind::ShallowMine:
            capacity += 2.0F;
            break;
        case ImprovementKind::None:
        case ImprovementKind::Road:
            break;
        }
    }
    return capacity * effects.carrying_capacity_multiplier;
}

std::vector<int> SelectWorkedTiles(const World& world, const Settlement& settlement, const SimulationParams& params,
                                   const TechEffects& effects) {
    std::vector<int> candidates;
    for (const int index : NearbyTileIndices(world, settlement, WorkRadiusFor(settlement))) {
        const auto& tile = TileByIndex(world, index);
        if (tile.worked_by_settlement_id == settlement.id && tile.improvement != ImprovementKind::None) {
            candidates.push_back(index);
        }
    }
    std::sort(candidates.begin(), candidates.end(), [&](int lhs, int rhs) {
        return WorkedTileScore(TileByIndex(world, lhs), settlement, effects) >
               WorkedTileScore(TileByIndex(world, rhs), settlement, effects);
    });
    const int slots = WorkSlotsFor(settlement, params);
    if (static_cast<int>(candidates.size()) > slots) {
        std::vector<int> selected(candidates.begin(), candidates.begin() + slots);
        const bool food_secure = settlement.stockpile.food >= static_cast<float>(settlement.population) * 0.35F;
        auto contains_kind = [&](ImprovementKind kind) {
            return std::any_of(selected.begin(), selected.end(),
                               [&](int index) { return TileByIndex(world, index).improvement == kind; });
        };
        auto append_best_kind = [&](ImprovementKind kind) {
            for (const int index : candidates) {
                const auto already_selected = std::find(selected.begin(), selected.end(), index) != selected.end();
                if (!already_selected && TileByIndex(world, index).improvement == kind) {
                    selected.back() = index;
                    std::sort(selected.begin(), selected.end(), [&](int lhs, int rhs) {
                        return WorkedTileScore(TileByIndex(world, lhs), settlement, effects) >
                               WorkedTileScore(TileByIndex(world, rhs), settlement, effects);
                    });
                    return;
                }
            }
        };

        if (slots > 1 && food_secure && !contains_kind(ImprovementKind::LumberCamp)) {
            append_best_kind(ImprovementKind::LumberCamp);
        }
        candidates = selected;
    }
    return candidates;
}

void ProduceFromWorkedTile(World& world, Settlement& settlement, int index, const TechEffects& effects,
                           bool enable_routes, Turn turn, EventLog& event_log) {
    auto& tile = TileByIndex(world, index);
    switch (tile.improvement) {
    case ImprovementKind::Farm:
        settlement.local_food_output_last_turn += (2.8F + tile.soil_quality * 3.8F + (tile.has_river ? 0.8F : 0.0F)) *
                                                  effects.farm_output_multiplier *
                                                  (tile.has_river ? effects.river_farm_output_multiplier : 1.0F);
        break;
    case ImprovementKind::LumberCamp: {
        const float before = tile.forest_cover;
        settlement.local_wood_output_last_turn += 0.7F + tile.forest_cover * 2.0F;
        tile.forest_cover = std::max(0.0F, tile.forest_cover - 0.010F);
        if (before >= 0.18F && tile.forest_cover < 0.18F) {
            std::ostringstream summary;
            summary << "LumberCamp at " << tile.x << "," << tile.y << " cleared most forest cover";
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::ForestCleared,
                .actor_id = settlement.id,
                .x = tile.x,
                .y = tile.y,
                .summary = summary.str(),
            });
        }
        break;
    }
    case ImprovementKind::Pasture:
        settlement.local_food_output_last_turn +=
            (1.8F + tile.fertility * 1.2F +
             (tile.resource == ResourceKind::Horse ? 1.0F * effects.horse_value_multiplier : 0.0F)) *
            effects.pasture_output_multiplier;
        break;
    case ImprovementKind::ShallowMine:
        if (effects.mining_enabled) {
            const bool route_connected = enable_routes && tile.has_route && settlement.polity_id != kInvalidPolityId &&
                                         tile.route_polity_id == settlement.polity_id;
            const float route_multiplier = route_connected ? (1.18F + tile.route_quality * 0.28F) : 1.0F;
            settlement.ore_output_last_turn +=
                IsMineral(tile.resource) ? (0.45F + tile.resource_amount * 0.65F) * route_multiplier : 0.0F;
        }
        break;
    case ImprovementKind::ForagingGround:
        settlement.local_food_output_last_turn += 1.0F + tile.fertility * 1.4F + tile.forest_cover * 0.5F;
        break;
    case ImprovementKind::None:
    case ImprovementKind::Road:
        break;
    }
}

} // namespace oikumene::settlement_system

namespace oikumene {

float LocalFoodOutput(const World& world, const Settlement& settlement, const SimulationParams& params) {
    float output = 0.0F;
    int samples = 0;

    for (int dy = -params.settlement_work_radius; dy <= params.settlement_work_radius; ++dy) {
        for (int dx = -params.settlement_work_radius; dx <= params.settlement_work_radius; ++dx) {
            const int x = settlement.x + dx;
            const int y = settlement.y + dy;
            if (!world.InBounds(x, y)) {
                continue;
            }
            const auto& tile = world.At(x, y);
            if (tile.is_ocean || tile.is_lake) {
                continue;
            }
            output += tile.fertility;
            if (tile.has_river || tile.is_coast) {
                output += 0.24F;
            }
            if (tile.biome == Biome::Grassland || tile.biome == Biome::Forest || tile.biome == Biome::Rainforest) {
                output += 0.10F;
            }
            ++samples;
        }
    }

    if (samples <= 0) {
        return 0.0F;
    }
    return output / static_cast<float>(samples) * static_cast<float>(settlement.population) * 0.13F;
}

float LocalWoodOutput(const World& world, const Settlement& settlement, const SimulationParams& params) {
    float output = 0.0F;
    for (int dy = -params.settlement_work_radius; dy <= params.settlement_work_radius; ++dy) {
        for (int dx = -params.settlement_work_radius; dx <= params.settlement_work_radius; ++dx) {
            const int x = settlement.x + dx;
            const int y = settlement.y + dy;
            if (!world.InBounds(x, y)) {
                continue;
            }
            const auto& tile = world.At(x, y);
            if (tile.biome == Biome::Forest || tile.biome == Biome::Rainforest || tile.resource == ResourceKind::Wood ||
                tile.resource == ResourceKind::Bamboo) {
                output += 0.45F;
            }
        }
    }
    return output;
}

} // namespace oikumene
