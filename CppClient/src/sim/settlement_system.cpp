#include "oikumene/sim/settlement_system.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "oikumene/sim/tech_effects.hpp"

namespace oikumene {
namespace {

bool IsWorkableLand(const Tile& tile) {
    return !tile.is_ocean && !tile.is_lake && tile.biome != Biome::Mountain && tile.biome != Biome::Snow;
}

bool IsMineral(ResourceKind resource) {
    return resource == ResourceKind::Copper || resource == ResourceKind::Tin ||
           resource == ResourceKind::ShallowIron || resource == ResourceKind::ShallowCoal ||
           resource == ResourceKind::Gold || resource == ResourceKind::Silver ||
           resource == ResourceKind::MeteoricIron;
}

int TileIndex(const World& world, int x, int y) {
    return y * world.Width() + x;
}

Tile& TileByIndex(World& world, int index) {
    return world.Tiles()[static_cast<std::size_t>(index)];
}

const Tile& TileByIndex(const World& world, int index) {
    return world.Tiles()[static_cast<std::size_t>(index)];
}

const Polity* PolityById(const std::vector<Polity>& polities, PolityId id) {
    for (const auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

TechEffects EffectsForSettlement(const Settlement& settlement, const std::vector<Polity>& polities) {
    const auto* polity = PolityById(polities, settlement.polity_id);
    return polity == nullptr ? TechEffects{} : ComputeTechEffects(polity->research);
}

int WorkRadiusFor(const Settlement& settlement) {
    return settlement.level == SettlementLevel::Village ? 3 : 2;
}

int WorkSlotsFor(const Settlement& settlement, const SimulationParams& params) {
    return std::max(1, settlement.population / std::max(1, params.population_per_work_slot));
}

float UpgradeReadiness(const Settlement& settlement) {
    if (settlement.level == SettlementLevel::Village) {
        return 1.0F;
    }

    const float population = std::min(1.0F, static_cast<float>(settlement.population) / 70.0F);
    const float age = std::min(1.0F, static_cast<float>(settlement.turns_since_founded) / 20.0F);
    const float food = std::min(1.0F, settlement.stockpile.food / 45.0F);
    const float wood = std::min(1.0F, settlement.stockpile.wood / 24.0F);
    return std::min(std::min(population, age), std::min(food, wood));
}

float ImprovementValue(const Tile& tile, ImprovementKind kind, const Settlement& settlement) {
    switch (kind) {
        case ImprovementKind::Farm:
            return tile.soil_quality + (tile.has_river ? 0.20F : 0.0F) + (tile.is_coast ? 0.08F : 0.0F);
        case ImprovementKind::LumberCamp:
            return tile.forest_cover + (tile.resource == ResourceKind::Wood || tile.resource == ResourceKind::Bamboo ? 0.22F : 0.0F);
        case ImprovementKind::Pasture:
            return (tile.biome == Biome::Grassland ? 0.55F : 0.0F) +
                   (tile.biome == Biome::Coast || tile.biome == Biome::Hill || tile.biome == Biome::River ? 0.24F
                                                                                                           : 0.0F) +
                   (tile.resource == ResourceKind::Horse ? 0.55F : 0.0F) + tile.fertility * 0.36F +
                   tile.soil_quality * 0.12F - tile.forest_cover * 0.20F;
        case ImprovementKind::ShallowMine:
            return IsMineral(tile.resource) ? 0.70F + tile.resource_amount * 0.35F : -1.0F;
        case ImprovementKind::ForagingGround:
            return tile.fertility + (tile.biome == Biome::Forest || tile.biome == Biome::Rainforest ? 0.20F : 0.0F);
        case ImprovementKind::None:
        case ImprovementKind::Road:
            break;
    }
    (void)settlement;
    return -1.0F;
}

bool CanBuildImprovement(const Tile& tile, ImprovementKind kind, int settlement_id) {
    if (!IsWorkableLand(tile) || tile.improvement != ImprovementKind::None) {
        return false;
    }
    if (tile.worked_by_settlement_id != -1 && tile.worked_by_settlement_id != settlement_id) {
        return false;
    }

    switch (kind) {
        case ImprovementKind::Farm:
            return tile.soil_quality >= 0.42F && tile.biome != Biome::Desert && tile.biome != Biome::Tundra &&
                   tile.biome != Biome::Wetland;
        case ImprovementKind::LumberCamp:
            return tile.forest_cover >= 0.42F || tile.resource == ResourceKind::Wood || tile.resource == ResourceKind::Bamboo;
        case ImprovementKind::Pasture:
            return tile.resource == ResourceKind::Horse || tile.biome == Biome::Grassland ||
                   ((tile.biome == Biome::Coast || tile.biome == Biome::Hill || tile.biome == Biome::River) &&
                    tile.fertility >= 0.38F && tile.forest_cover < 0.38F) ||
                   (tile.soil_quality >= 0.50F && tile.rainfall < 0.72F && tile.forest_cover < 0.28F &&
                    tile.biome != Biome::Forest && tile.biome != Biome::Rainforest && tile.biome != Biome::Desert &&
                    tile.biome != Biome::Tundra && tile.biome != Biome::Wetland);
        case ImprovementKind::ShallowMine:
            return IsMineral(tile.resource);
        case ImprovementKind::ForagingGround:
            return tile.fertility >= 0.30F || tile.forest_cover >= 0.25F;
        case ImprovementKind::None:
        case ImprovementKind::Road:
            break;
    }
    return false;
}

std::vector<int> NearbyTileIndices(const World& world, const Settlement& settlement, int radius) {
    std::vector<int> indices;
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            const int x = settlement.x + dx;
            const int y = settlement.y + dy;
            if (!world.InBounds(x, y)) {
                continue;
            }
            const auto& tile = world.At(x, y);
            if (IsWorkableLand(tile)) {
                indices.push_back(TileIndex(world, x, y));
            }
        }
    }
    return indices;
}

int CountImprovement(const World& world, const Settlement& settlement, ImprovementKind kind) {
    int count = 0;
    for (const int index : NearbyTileIndices(world, settlement, WorkRadiusFor(settlement))) {
        const auto& tile = TileByIndex(world, index);
        if (tile.worked_by_settlement_id == settlement.id && tile.improvement == kind) {
            ++count;
        }
    }
    return count;
}

bool HasBuildableImprovement(const World& world, const Settlement& settlement, ImprovementKind kind) {
    for (const int index : NearbyTileIndices(world, settlement, WorkRadiusFor(settlement))) {
        if (CanBuildImprovement(TileByIndex(world, index), kind, settlement.id)) {
            return true;
        }
    }
    return false;
}

int DesiredFarmCount(const Settlement& settlement) {
    const int cap = settlement.level == SettlementLevel::Village ? 4 : 2;
    return std::clamp(1 + settlement.population / 130, 1, cap);
}

ImprovementKind DesiredImprovement(const World& world,
                                   const Settlement& settlement,
                                   const SimulationParams& params,
                                   const TechEffects& effects) {
    const int farms = CountImprovement(world, settlement, ImprovementKind::Farm);
    const int lumber_camps = CountImprovement(world, settlement, ImprovementKind::LumberCamp);
    const int pastures = CountImprovement(world, settlement, ImprovementKind::Pasture);
    const int mines = CountImprovement(world, settlement, ImprovementKind::ShallowMine);
    const int desired_farms = DesiredFarmCount(settlement);
    const int work_slots = WorkSlotsFor(settlement, params);

    if (farms < 1) {
        return ImprovementKind::Farm;
    }
    if (lumber_camps < 1) {
        return ImprovementKind::LumberCamp;
    }
    if (effects.mining_enabled && mines < 1 && HasBuildableImprovement(world, settlement, ImprovementKind::ShallowMine)) {
        return ImprovementKind::ShallowMine;
    }
    if (pastures < 1 && HasBuildableImprovement(world, settlement, ImprovementKind::Pasture)) {
        return ImprovementKind::Pasture;
    }
    if (settlement.stockpile.food < static_cast<float>(settlement.population) * 0.45F && farms < desired_farms) {
        return ImprovementKind::Farm;
    }
    if (settlement.stockpile.wood < 28.0F && lumber_camps < 2) {
        return ImprovementKind::LumberCamp;
    }
    if (mines < 1 && HasBuildableImprovement(world, settlement, ImprovementKind::ShallowMine) &&
        (settlement.level == SettlementLevel::Village || settlement.population >= 85)) {
        return ImprovementKind::ShallowMine;
    }
    if (settlement.level == SettlementLevel::Village && pastures < 2 &&
        HasBuildableImprovement(world, settlement, ImprovementKind::Pasture)) {
        return ImprovementKind::Pasture;
    }
    if (farms < std::min(desired_farms, work_slots + 1) &&
        HasBuildableImprovement(world, settlement, ImprovementKind::Farm)) {
        return ImprovementKind::Farm;
    }
    return ImprovementKind::ForagingGround;
}

EventType EventForImprovement(ImprovementKind kind) {
    switch (kind) {
        case ImprovementKind::Farm:
            return EventType::FarmBuilt;
        case ImprovementKind::LumberCamp:
            return EventType::LumberCampBuilt;
        case ImprovementKind::Pasture:
            return EventType::PastureBuilt;
        case ImprovementKind::ShallowMine:
            return EventType::ShallowMineBuilt;
        case ImprovementKind::ForagingGround:
        case ImprovementKind::Road:
        case ImprovementKind::None:
            break;
    }
    return EventType::TileImproved;
}

bool BuildBestImprovement(World& world, Settlement& settlement, ImprovementKind desired, Turn turn, EventLog& event_log) {
    const int radius = WorkRadiusFor(settlement);
    const auto candidates = NearbyTileIndices(world, settlement, radius);

    auto try_build = [&](ImprovementKind kind) {
        float best_value = -1.0F;
        int best_index = -1;
        for (const int index : candidates) {
            const auto& tile = TileByIndex(world, index);
            if (!CanBuildImprovement(tile, kind, settlement.id)) {
                continue;
            }
            const float value = ImprovementValue(tile, kind, settlement);
            if (value > best_value) {
                best_value = value;
                best_index = index;
            }
        }

        if (best_index < 0) {
            return false;
        }

        auto& tile = TileByIndex(world, best_index);
        tile.improvement = kind;
        tile.worked_by_settlement_id = settlement.id;
        tile.improvement_progress = 1.0F;

        std::ostringstream summary;
        summary << ToString(settlement.level) << " " << settlement.id << " built " << ToString(kind) << " at "
                << tile.x << "," << tile.y << ": fertility " << tile.fertility << ", soil " << tile.soil_quality
                << ", forest " << tile.forest_cover << ", river " << (tile.has_river ? "yes" : "no");
        event_log.Add(SimEvent{
            .turn = turn,
            .type = EventForImprovement(kind),
            .actor_id = settlement.id,
            .x = tile.x,
            .y = tile.y,
            .summary = summary.str(),
        });
        return true;
    };

    if (try_build(desired)) {
        return true;
    }

    const ImprovementKind fallback_order[] = {
        ImprovementKind::Farm,
        ImprovementKind::LumberCamp,
        ImprovementKind::Pasture,
        ImprovementKind::ShallowMine,
        ImprovementKind::ForagingGround,
    };
    for (const auto kind : fallback_order) {
        if (kind != desired && try_build(kind)) {
            return true;
        }
    }
    return false;
}

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
                capacity +=
                    (16.0F + tile.soil_quality * 12.0F + (tile.has_river ? 4.0F : 0.0F)) *
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

std::vector<int> SelectWorkedTiles(const World& world,
                                   const Settlement& settlement,
                                   const SimulationParams& params,
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
            return std::any_of(selected.begin(), selected.end(), [&](int index) {
                return TileByIndex(world, index).improvement == kind;
            });
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

void ProduceFromWorkedTile(World& world,
                           Settlement& settlement,
                           int index,
                           const TechEffects& effects,
                           bool enable_routes,
                           Turn turn,
                           EventLog& event_log) {
    auto& tile = TileByIndex(world, index);
    switch (tile.improvement) {
        case ImprovementKind::Farm:
            settlement.local_food_output_last_turn +=
                (2.8F + tile.soil_quality * 3.8F + (tile.has_river ? 0.8F : 0.0F)) *
                effects.farm_output_multiplier * (tile.has_river ? effects.river_farm_output_multiplier : 1.0F);
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
                const bool route_connected =
                    enable_routes && tile.has_route && settlement.polity_id != kInvalidPolityId &&
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

}  // namespace

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

void SettlementSystem::UpdateSettlements(World& world,
                                         const SimulationParams& params,
                                         Turn turn,
                                         std::vector<Settlement>& settlements,
                                         const std::vector<Polity>& polities,
                                         EventLog& event_log) {
    for (auto& settlement : settlements) {
        const TechEffects effects = EffectsForSettlement(settlement, polities);
        ++settlement.turns_since_founded;
        settlement.work_radius = WorkRadiusFor(settlement);
        const int existing_improvements = CountImprovement(world, settlement, ImprovementKind::Farm) +
                                          CountImprovement(world, settlement, ImprovementKind::LumberCamp) +
                                          CountImprovement(world, settlement, ImprovementKind::Pasture) +
                                          CountImprovement(world, settlement, ImprovementKind::ShallowMine) +
                                          CountImprovement(world, settlement, ImprovementKind::ForagingGround);
        const int build_interval = settlement.level == SettlementLevel::Village ? params.village_improvement_build_interval
                                                                                : params.camp_improvement_build_interval;
        const bool should_build_now =
            settlement.turns_since_founded == 1 ||
            (build_interval > 0 && settlement.turns_since_founded % build_interval == 0);
        const int improvement_limit =
            WorkSlotsFor(settlement, params) + (settlement.level == SettlementLevel::Village ? 3 : 2);
        if (should_build_now && existing_improvements < improvement_limit) {
            BuildBestImprovement(world, settlement, DesiredImprovement(world, settlement, params, effects), turn, event_log);
        }

        settlement.local_food_output_last_turn =
            std::max(0.8F, LocalFoodOutput(world, settlement, params) * 0.16F);
        settlement.local_wood_output_last_turn = 0.0F;
        settlement.ore_output_last_turn = 0.0F;
        settlement.worked_tile_indices = SelectWorkedTiles(world, settlement, params, effects);
        settlement.worked_tile_count = static_cast<int>(settlement.worked_tile_indices.size());
        for (const int index : settlement.worked_tile_indices) {
            ProduceFromWorkedTile(world, settlement, index, effects, params.enable_routes, turn, event_log);
        }

        settlement.food_consumption_last_turn =
            static_cast<float>(settlement.population) * params.settlement_food_consumption_per_person;
        settlement.stockpile.food += settlement.local_food_output_last_turn;
        settlement.stockpile.wood += settlement.local_wood_output_last_turn;
        settlement.stockpile.ore += settlement.ore_output_last_turn;
        settlement.stockpile.food -= settlement.food_consumption_last_turn;
        settlement.carrying_capacity = CarryingCapacityFor(world, settlement, effects);
        settlement.carrying_capacity_ratio =
            settlement.carrying_capacity <= 0.0F ? 0.0F : static_cast<float>(settlement.population) / settlement.carrying_capacity;

        const float capacity_pressure = std::max(0.0F, 1.0F - settlement.carrying_capacity_ratio);
        const float food_security =
            std::clamp(settlement.stockpile.food / std::max(1.0F, static_cast<float>(settlement.population) * 0.70F),
                       0.0F, 1.4F);
        if (settlement.stockpile.food > static_cast<float>(settlement.population) * 0.25F &&
            capacity_pressure > 0.025F) {
            const float growth_float =
                static_cast<float>(settlement.population) * params.settlement_growth_rate * food_security * capacity_pressure;
            const int growth = std::max(1, static_cast<int>(std::floor(growth_float)));
            settlement.population += growth;
            settlement.stockpile.food -= static_cast<float>(growth) * 0.7F;
            settlement.carrying_capacity_ratio =
                static_cast<float>(settlement.population) / std::max(1.0F, settlement.carrying_capacity);
            std::ostringstream summary;
            summary << "Camp " << settlement.id << " population grew to " << settlement.population << ", food surplus "
                    << settlement.stockpile.food << ", food output " << settlement.local_food_output_last_turn
                    << ", wood output " << settlement.local_wood_output_last_turn << ", cap "
                    << settlement.carrying_capacity;
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::PopulationGrowth,
                .actor_id = settlement.id,
                .x = settlement.x,
                .y = settlement.y,
                .summary = summary.str(),
            });
        } else if (settlement.stockpile.food < 0.0F) {
            const float deficit = settlement.stockpile.food;
            const int loss = std::max(
                1, static_cast<int>(static_cast<float>(settlement.population) * 0.025F * effects.famine_severity_multiplier));
            settlement.population = std::max(1, settlement.population - loss);
            settlement.stockpile.food = 0.0F;
            std::ostringstream summary;
            summary << "Camp " << settlement.id << " famine, food deficit " << deficit << ", population "
                    << settlement.population;
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::Famine,
                .actor_id = settlement.id,
                .x = settlement.x,
                .y = settlement.y,
                .summary = summary.str(),
            });
        }

        if (settlement.carrying_capacity_ratio >= 0.80F && turn % 10 == 0) {
            std::ostringstream summary;
            summary << ToString(settlement.level) << " " << settlement.id << " nearing carrying capacity: pop "
                    << settlement.population << " / cap " << settlement.carrying_capacity;
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::CarryingCapacityReached,
                .actor_id = settlement.id,
                .x = settlement.x,
                .y = settlement.y,
                .summary = summary.str(),
            });
        }

        settlement.upgrade_readiness = UpgradeReadiness(settlement);
        if (settlement.level == SettlementLevel::Camp && settlement.population >= 70 &&
            settlement.turns_since_founded >= 20 && settlement.stockpile.food >= 45.0F &&
            settlement.stockpile.wood >= 24.0F) {
            settlement.level = SettlementLevel::Village;
            settlement.work_radius = WorkRadiusFor(settlement);
            settlement.upgrade_readiness = 1.0F;
            settlement.stockpile.food -= 35.0F;
            settlement.stockpile.wood -= 18.0F;
            std::ostringstream summary;
            summary << "Camp " << settlement.id << " upgraded to Village, pop " << settlement.population << ", food "
                    << settlement.stockpile.food << ", wood " << settlement.stockpile.wood;
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::SettlementUpgraded,
                .actor_id = settlement.id,
                .x = settlement.x,
                .y = settlement.y,
                .summary = summary.str(),
            });
        }
    }
}

void SettlementSystem::UpdateSettlements(World& world,
                                         const SimulationParams& params,
                                         Turn turn,
                                         std::vector<Settlement>& settlements,
                                         EventLog& event_log) {
    const std::vector<Polity> no_polities;
    UpdateSettlements(world, params, turn, settlements, no_polities, event_log);
}

}  // namespace oikumene
