#include "settlement_improvement.hpp"

#include <algorithm>
#include <sstream>

#include "settlement_work.hpp"

namespace oikumene::settlement_system {
namespace {

float ImprovementValue(const Tile& tile, ImprovementKind kind, const Settlement& settlement) {
    switch (kind) {
    case ImprovementKind::Farm:
        return tile.soil_quality + (tile.has_river ? 0.20F : 0.0F) + (tile.is_coast ? 0.08F : 0.0F);
    case ImprovementKind::LumberCamp:
        return tile.forest_cover +
               (tile.resource == ResourceKind::Wood || tile.resource == ResourceKind::Bamboo ? 0.22F : 0.0F);
    case ImprovementKind::Pasture:
        return (tile.biome == Biome::Grassland ? 0.55F : 0.0F) +
               (tile.biome == Biome::Coast || tile.biome == Biome::Hill || tile.biome == Biome::River ? 0.24F : 0.0F) +
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
        return tile.forest_cover >= 0.42F || tile.resource == ResourceKind::Wood ||
               tile.resource == ResourceKind::Bamboo;
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

} // namespace

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

ImprovementKind DesiredImprovement(const World& world, const Settlement& settlement, const SimulationParams& params,
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
    if (effects.mining_enabled && mines < 1 &&
        HasBuildableImprovement(world, settlement, ImprovementKind::ShallowMine)) {
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

bool BuildBestImprovement(World& world, Settlement& settlement, ImprovementKind desired, Turn turn,
                          EventLog& event_log) {
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
        summary << ToString(settlement.level) << " " << settlement.id << " built " << ToString(kind) << " at " << tile.x
                << "," << tile.y << ": fertility " << tile.fertility << ", soil " << tile.soil_quality << ", forest "
                << tile.forest_cover << ", river " << (tile.has_river ? "yes" : "no");
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
        ImprovementKind::Farm,        ImprovementKind::LumberCamp,     ImprovementKind::Pasture,
        ImprovementKind::ShallowMine, ImprovementKind::ForagingGround,
    };
    for (const auto kind : fallback_order) {
        if (kind != desired && try_build(kind)) {
            return true;
        }
    }
    return false;
}

} // namespace oikumene::settlement_system
