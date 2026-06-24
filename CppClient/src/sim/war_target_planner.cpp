#include "oikumene/sim/war_target_planner.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "oikumene/sim/pathfinding.hpp"
#include "oikumene/sim/technology.hpp"

namespace oikumene {
namespace {

constexpr float kMaxCampaignPathCost = 220.0F;
constexpr int kMaxResourceTargetsPerPressure = 8;
constexpr int kMaxBorderTargetsPerPressure = 8;
constexpr int kMaxStrategicTargetsPerPressure = 6;
constexpr int kMaxCandidates = 512;

struct RawTarget {
    WarTargetKind kind = WarTargetKind::Settlement;
    int x = 0;
    int y = 0;
    int settlement_id = -1;
    int trade_id = -1;
    float sort_score = 0.0F;
};

float Clamp01(float value) {
    return std::clamp(value, 0.0F, 1.0F);
}

float SafeRatio(float numerator, float denominator) {
    return numerator / std::max(1.0F, denominator);
}

const Polity* PolityById(const std::vector<Polity>& polities, PolityId id) {
    for (const auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id) {
    for (const auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

const Settlement* CapitalFor(const std::vector<Settlement>& settlements, const Polity& polity) {
    return SettlementById(settlements, polity.capital_settlement_id);
}

bool SamePair(const TradeAgreement& trade, PolityId a, PolityId b) {
    return (trade.polity_a_id == a && trade.polity_b_id == b) || (trade.polity_a_id == b && trade.polity_b_id == a);
}

float ResourceStrategicValue(ResourceKind resource) {
    switch (resource) {
    case ResourceKind::MeteoricIron:
        return 1.00F;
    case ResourceKind::Copper:
    case ResourceKind::Tin:
    case ResourceKind::ShallowIron:
        return 0.86F;
    case ResourceKind::ShallowCoal:
        return 0.72F;
    case ResourceKind::Horse:
        return 0.68F;
    case ResourceKind::Gold:
    case ResourceKind::Silver:
        return 0.64F;
    case ResourceKind::Salt:
        return 0.56F;
    case ResourceKind::Stone:
    case ResourceKind::Clay:
        return 0.36F;
    case ResourceKind::Wood:
    case ResourceKind::Bamboo:
        return 0.24F;
    case ResourceKind::None:
        return 0.0F;
    }
    return 0.0F;
}

float TerrainAttritionFor(const Tile& tile, float route_cost) {
    float attrition = route_cost * 0.008F;
    if (tile.biome == Biome::Mountain) {
        attrition += 0.34F;
    } else if (tile.biome == Biome::Desert || tile.biome == Biome::Snow) {
        attrition += 0.22F;
    } else if (tile.biome == Biome::Wetland || tile.biome == Biome::Rainforest) {
        attrition += 0.18F;
    } else if (tile.biome == Biome::Hill || tile.biome == Biome::Forest) {
        attrition += 0.10F;
    }
    return attrition;
}

float KindObjectiveFit(WarTargetKind kind, WarObjective objective) {
    if (kind == WarTargetKind::TradeRouteNode &&
        (objective == WarObjective::Blockade || objective == WarObjective::TradeCoercion ||
         objective == WarObjective::DependencyBreakout)) {
        return 0.30F;
    }
    if (kind == WarTargetKind::ContestedBorder && objective == WarObjective::BorderDispute) {
        return 0.24F;
    }
    if (kind == WarTargetKind::ResourceRegion && objective == WarObjective::TradeCoercion) {
        return 0.20F;
    }
    if (kind == WarTargetKind::Settlement &&
        (objective == WarObjective::BorderDispute || objective == WarObjective::DependencyBreakout)) {
        return 0.16F;
    }
    if (kind == WarTargetKind::StrategicPass && objective == WarObjective::Blockade) {
        return 0.16F;
    }
    return 0.0F;
}

float TileResourceValue(const Tile& tile) {
    return ResourceStrategicValue(tile.resource) * std::clamp(tile.resource_amount, 0.0F, 1.3F);
}

float TileFarmValue(const Tile& tile) {
    float value = tile.fertility * 0.46F + tile.soil_quality * 0.24F;
    if (tile.has_river) {
        value += 0.14F;
    }
    if (tile.improvement == ImprovementKind::Farm) {
        value += 0.22F;
    }
    if (tile.improvement == ImprovementKind::Pasture) {
        value += 0.12F;
    }
    return Clamp01(value);
}

void AddTopTiles(std::vector<RawTarget>& raw_targets, std::vector<RawTarget> candidates, int max_count) {
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.sort_score > rhs.sort_score; });
    const int count = std::min(max_count, static_cast<int>(candidates.size()));
    for (int i = 0; i < count; ++i) {
        raw_targets.push_back(candidates[static_cast<std::size_t>(i)]);
    }
}

std::vector<RawTarget> BuildRawTargets(const World& world, const std::vector<Settlement>& settlements,
                                       const std::vector<TradeAgreement>& trades, const WarPressure& pressure) {
    std::vector<RawTarget> raw_targets;

    for (const auto& settlement : settlements) {
        if (settlement.polity_id != pressure.target_polity_id) {
            continue;
        }
        const float population_value = std::min(1.0F, static_cast<float>(settlement.population) / 520.0F);
        raw_targets.push_back(RawTarget{
            .kind = WarTargetKind::Settlement,
            .x = settlement.x,
            .y = settlement.y,
            .settlement_id = settlement.id,
            .trade_id = -1,
            .sort_score = population_value + (settlement.is_capital ? 0.55F : 0.0F),
        });
    }

    std::vector<RawTarget> resource_targets;
    std::vector<RawTarget> border_targets;
    std::vector<RawTarget> strategic_targets;
    for (const auto& tile : world.Tiles()) {
        if (tile.is_ocean || tile.is_lake) {
            continue;
        }
        if (tile.controller_polity_id == pressure.target_polity_id && tile.resource != ResourceKind::None) {
            const float score = TileResourceValue(tile) + TileFarmValue(tile) * 0.22F + tile.defense_bonus * 0.12F;
            if (score >= 0.12F) {
                resource_targets.push_back(RawTarget{
                    .kind = WarTargetKind::ResourceRegion,
                    .x = tile.x,
                    .y = tile.y,
                    .settlement_id = -1,
                    .trade_id = -1,
                    .sort_score = score,
                });
            }
        }
        if (tile.is_contested && tile.controller_polity_id == pressure.target_polity_id) {
            border_targets.push_back(RawTarget{
                .kind = WarTargetKind::ContestedBorder,
                .x = tile.x,
                .y = tile.y,
                .settlement_id = -1,
                .trade_id = -1,
                .sort_score = pressure.border_pressure + tile.defense_bonus * 0.30F + TileFarmValue(tile) * 0.18F,
            });
        }
        const bool is_pass_like = tile.has_route && (tile.biome == Biome::Hill || tile.biome == Biome::Mountain ||
                                                     tile.has_river || tile.is_coast);
        if (tile.controller_polity_id == pressure.target_polity_id && is_pass_like) {
            strategic_targets.push_back(RawTarget{
                .kind = WarTargetKind::StrategicPass,
                .x = tile.x,
                .y = tile.y,
                .settlement_id = -1,
                .trade_id = -1,
                .sort_score =
                    tile.defense_bonus * 0.42F + tile.route_quality * 0.36F + pressure.border_pressure * 0.20F,
            });
        }
    }
    AddTopTiles(raw_targets, resource_targets, kMaxResourceTargetsPerPressure);
    AddTopTiles(raw_targets, border_targets, kMaxBorderTargetsPerPressure);
    AddTopTiles(raw_targets, strategic_targets, kMaxStrategicTargetsPerPressure);

    for (const auto& trade : trades) {
        if (!trade.active || !SamePair(trade, pressure.actor_polity_id, pressure.target_polity_id) ||
            trade.path.empty()) {
            continue;
        }
        const auto midpoint = trade.path[trade.path.size() / 2U];
        raw_targets.push_back(RawTarget{
            .kind = WarTargetKind::TradeRouteNode,
            .x = midpoint.x,
            .y = midpoint.y,
            .settlement_id = -1,
            .trade_id = trade.id,
            .sort_score = pressure.trade_conflict_weight + trade.expected_profit * 0.18F,
        });
    }

    return raw_targets;
}

std::string BuildReason(const WarTargetCandidate& candidate) {
    std::ostringstream stream;
    stream << ToString(candidate.kind) << " for " << ToString(candidate.objective) << ": value "
           << candidate.target_value << " cost " << candidate.campaign_cost << " roi " << candidate.roi << " route "
           << candidate.route_cost;
    return stream.str();
}

WarTargetCandidate BuildCandidate(int id, const World& world, const std::vector<Settlement>& settlements,
                                  const std::vector<TradeAgreement>& trades, const Polity& actor, const Polity& target,
                                  const WarPressure& pressure, const RawTarget& raw) {
    WarTargetCandidate candidate;
    candidate.id = id;
    candidate.pressure_id = pressure.id;
    candidate.actor_polity_id = actor.id;
    candidate.target_polity_id = target.id;
    candidate.objective = pressure.objective;
    candidate.kind = raw.kind;
    candidate.x = raw.x;
    candidate.y = raw.y;
    candidate.target_settlement_id = raw.settlement_id;
    candidate.target_trade_id = raw.trade_id;

    const auto* capital = CapitalFor(settlements, actor);
    if (capital == nullptr || !world.InBounds(raw.x, raw.y)) {
        candidate.reason = "missing capital or target out of bounds";
        return candidate;
    }

    PathOptions options;
    options.allow_ocean = HasTech(actor.research, TechId::Sailing);
    options.use_existing_routes = true;
    options.route_polity_id = actor.id;
    options.max_cost = kMaxCampaignPathCost;
    const auto path =
        FindPath(world, TileCoord{.x = capital->x, .y = capital->y}, TileCoord{.x = raw.x, .y = raw.y}, options);
    if (!path.reachable) {
        candidate.reason = "unreachable campaign target";
        return candidate;
    }
    candidate.route_cost = path.total_cost;
    candidate.path = path.path;

    const Tile& tile = world.At(raw.x, raw.y);
    const auto* target_settlement = SettlementById(settlements, raw.settlement_id);
    candidate.farmland_value = TileFarmValue(tile);
    candidate.resource_value = TileResourceValue(tile);
    if (target_settlement != nullptr) {
        candidate.settlement_value = std::min(1.15F, static_cast<float>(target_settlement->population) / 380.0F);
        if (target_settlement->is_capital) {
            candidate.settlement_value += 0.36F;
        }
        candidate.farmland_value =
            std::max(candidate.farmland_value, target_settlement->local_food_output_last_turn * 0.10F);
        candidate.resource_value = std::max(candidate.resource_value, target_settlement->ore_output_last_turn * 0.42F);
    }
    if (raw.trade_id >= 0) {
        const auto trade_it = std::find_if(trades.begin(), trades.end(), [&](const TradeAgreement& trade) {
            return trade.id == raw.trade_id && trade.active;
        });
        if (trade_it != trades.end()) {
            candidate.trade_value = Clamp01(pressure.trade_conflict_weight * 0.62F + trade_it->expected_profit * 0.16F +
                                            (1.0F - trade_it->route_efficiency) * 0.24F);
        }
    }
    candidate.strategic_value =
        Clamp01(tile.defense_bonus * 0.30F + (tile.has_route ? 0.16F : 0.0F) + (tile.has_river ? 0.14F : 0.0F) +
                (tile.is_contested ? 0.24F : 0.0F) + KindObjectiveFit(candidate.kind, candidate.objective));
    candidate.target_value = candidate.farmland_value * 0.24F + candidate.resource_value * 0.34F +
                             candidate.settlement_value * 0.38F + candidate.trade_value * 0.42F +
                             candidate.strategic_value * 0.28F + pressure.target_value * 0.18F;

    const float actor_population = static_cast<float>(std::max(1, actor.population));
    const float target_population = static_cast<float>(std::max(1, target.population));
    candidate.mobilization_cost = 0.16F +
                                  std::min(0.42F, target_population / (actor_population + target_population + 120.0F)) +
                                  pressure.declaration_pressure * 0.08F;
    candidate.supply_cost = candidate.route_cost * 0.011F + static_cast<float>(candidate.path.size()) * 0.0025F;
    candidate.equipment_cost =
        0.09F + SafeRatio(target.military_potential, actor.tool_efficiency * 140.0F + 30.0F) * 0.34F;
    candidate.terrain_attrition = TerrainAttritionFor(tile, candidate.route_cost);
    candidate.defender_power = Clamp01(
        SafeRatio(target.military_potential + 8.0F, actor.military_potential + target.military_potential + 16.0F));
    candidate.defense_cost = tile.defense_bonus * 0.38F + candidate.defender_power * 0.56F;
    if (target_settlement != nullptr) {
        candidate.defense_cost += std::min(0.28F, static_cast<float>(target_settlement->population) / 720.0F);
    }
    if (HasTech(target.research, TechId::Fortification)) {
        candidate.defense_cost += 0.18F;
    }
    candidate.occupation_cost = candidate.route_cost * 0.006F + target.stability * 0.18F +
                                pressure.friendly_penalty * 0.42F + std::max(0.0F, actor.overextension) * 0.18F +
                                (target_settlement != nullptr ? 0.14F : 0.04F);
    candidate.campaign_cost = candidate.mobilization_cost + candidate.supply_cost + candidate.equipment_cost +
                              candidate.terrain_attrition + candidate.defense_cost + candidate.occupation_cost;

    const float pressure_multiplier =
        0.72F + pressure.declaration_pressure * 0.36F + pressure.military_advantage * 0.24F;
    candidate.roi = candidate.target_value * pressure_multiplier / std::max(0.18F, candidate.campaign_cost);
    candidate.action_score = Clamp01(candidate.roi / 0.92F);
    candidate.high_value = candidate.action_score >= 0.64F;
    candidate.reason = BuildReason(candidate);
    return candidate;
}

} // namespace

std::vector<WarTargetCandidate> BuildWarTargetCandidates(const World& world, const std::vector<Settlement>& settlements,
                                                         const std::vector<Polity>& polities,
                                                         const std::vector<TradeAgreement>& trades,
                                                         const std::vector<WarPressure>& pressures) {
    std::vector<WarTargetCandidate> candidates;
    int next_id = 0;
    for (const auto& pressure : pressures) {
        const auto* actor = PolityById(polities, pressure.actor_polity_id);
        const auto* target = PolityById(polities, pressure.target_polity_id);
        if (actor == nullptr || target == nullptr || actor->id == target->id) {
            continue;
        }
        const auto raw_targets = BuildRawTargets(world, settlements, trades, pressure);
        for (const auto& raw : raw_targets) {
            auto candidate = BuildCandidate(next_id, world, settlements, trades, *actor, *target, pressure, raw);
            if (candidate.path.empty() || candidate.target_value <= 0.04F) {
                continue;
            }
            candidates.push_back(candidate);
            ++next_id;
        }
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.action_score > rhs.action_score; });
    if (static_cast<int>(candidates.size()) > kMaxCandidates) {
        candidates.resize(kMaxCandidates);
    }
    return candidates;
}

} // namespace oikumene
