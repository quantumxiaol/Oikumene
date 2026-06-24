#include "oikumene/sim/polity_system.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "oikumene/sim/control_field.hpp"
#include "oikumene/sim/tech_effects.hpp"

namespace oikumene {
namespace {

constexpr float kFoundingFoodThreshold = 70.0F;
constexpr int kFoundingPopulationThreshold = 120;
constexpr float kCapitalSeparationCost = 22.0F;
constexpr float kTributeRate = 0.15F;

PolityId NextPolityId(const std::vector<Polity>& polities) {
    PolityId next = 0;
    for (const auto& polity : polities) {
        next = std::max(next, static_cast<PolityId>(polity.id + 1));
    }
    return next;
}

Settlement* SettlementById(std::vector<Settlement>& settlements, int id) {
    for (auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
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

bool ContainsSettlement(const Polity& polity, int settlement_id) {
    return std::find(polity.member_settlement_ids.begin(), polity.member_settlement_ids.end(), settlement_id) !=
           polity.member_settlement_ids.end();
}

bool IsEligibleCapital(const Settlement& settlement) {
    return settlement.level == SettlementLevel::Village && settlement.population >= kFoundingPopulationThreshold &&
           settlement.stockpile.food >= kFoundingFoodThreshold;
}

bool HasNearbyCapital(const World& world, const std::vector<Settlement>& settlements,
                      const std::vector<Polity>& polities, const Settlement& candidate, bool use_routes) {
    for (const auto& polity : polities) {
        const auto* capital = SettlementById(settlements, polity.capital_settlement_id);
        if (capital == nullptr) {
            continue;
        }
        const auto effects = ComputeTechEffects(polity.research);
        const float cost = TerrainPathCost(world, candidate.x, candidate.y, capital->x, capital->y,
                                           kCapitalSeparationCost, effects.control_path_cost_multiplier,
                                           effects.coastal_control_cost_multiplier, polity.id, use_routes);
        if (cost <= kCapitalSeparationCost) {
            return true;
        }
    }
    return false;
}

Polity* PolityById(std::vector<Polity>& polities, PolityId id) {
    for (auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

void RecalculatePolityAggregates(std::vector<Settlement>& settlements, std::vector<Polity>& polities) {
    for (auto& polity : polities) {
        polity.population = 0;
        polity.food = 0.0F;
        polity.wood = 0.0F;
        polity.ore = 0.0F;
        polity.wealth = 0.0F;

        for (const int settlement_id : polity.member_settlement_ids) {
            const auto* settlement = SettlementById(settlements, settlement_id);
            if (settlement == nullptr) {
                continue;
            }
            polity.population += settlement->population;
            polity.food += settlement->stockpile.food;
            polity.wood += settlement->stockpile.wood;
            polity.ore += settlement->stockpile.ore;
            polity.wealth += settlement->stockpile.wealth;
        }

        const float member_bonus = static_cast<float>(polity.member_settlement_ids.size()) * 1.2F;
        polity.admin_range =
            16.0F + std::sqrt(static_cast<float>(std::max(0, polity.population))) * 0.22F + member_bonus;
        polity.control_power = 38.0F + static_cast<float>(polity.population) * 0.045F;
    }
}

float PolityLevelAdminBonus(PolityLevel level) {
    switch (level) {
    case PolityLevel::Chiefdom:
        return 0.0F;
    case PolityLevel::CityState:
        return 24.0F;
    case PolityLevel::Kingdom:
        return 58.0F;
    }
    return 0.0F;
}

float AverageCapitalPathCost(const World& world, const std::vector<Settlement>& settlements, const Polity& polity,
                             const Settlement& capital, bool use_routes) {
    if (polity.member_settlement_ids.size() <= 1) {
        return 0.0F;
    }

    float total = 0.0F;
    int measured = 0;
    for (const int settlement_id : polity.member_settlement_ids) {
        if (settlement_id == polity.capital_settlement_id) {
            continue;
        }
        const auto* settlement = SettlementById(settlements, settlement_id);
        if (settlement == nullptr) {
            continue;
        }
        const auto effects = ComputeTechEffects(polity.research);
        float cost = TerrainPathCost(world, capital.x, capital.y, settlement->x, settlement->y, 120.0F,
                                     effects.control_path_cost_multiplier, effects.coastal_control_cost_multiplier,
                                     polity.id, use_routes);
        if (!std::isfinite(cost)) {
            cost = 120.0F;
        }
        total += cost;
        ++measured;
    }
    return measured <= 0 ? 0.0F : total / static_cast<float>(measured);
}

void RecalculatePolityBudgetsAndAdministration(const World& world, const std::vector<Settlement>& settlements,
                                               std::vector<Polity>& polities, bool use_routes) {
    for (auto& polity : polities) {
        polity.budget = PolityBudget{};
        const auto* capital = SettlementById(settlements, polity.capital_settlement_id);
        if (capital == nullptr) {
            polity.admin_load = 0.0F;
            polity.admin_capacity = 0.0F;
            polity.overextension = 0.0F;
            polity.stability = 0.0F;
            polity.legitimacy = 0.0F;
            continue;
        }

        for (const int settlement_id : polity.member_settlement_ids) {
            const auto* settlement = SettlementById(settlements, settlement_id);
            if (settlement == nullptr) {
                continue;
            }
            const float food_surplus =
                std::max(0.0F, settlement->local_food_output_last_turn - settlement->food_consumption_last_turn);
            polity.budget.food_income += food_surplus * kTributeRate;
            polity.budget.wood_income += std::max(0.0F, settlement->local_wood_output_last_turn) * kTributeRate;
            polity.budget.ore_income += std::max(0.0F, settlement->ore_output_last_turn) * kTributeRate;
            polity.budget.wealth_income += static_cast<float>(settlement->population) * 0.018F;
        }

        const auto effects = ComputeTechEffects(polity.research);
        polity.budget.wealth_income += polity.trade_income;
        const float average_distance = AverageCapitalPathCost(world, settlements, polity, *capital, use_routes);
        const float member_count = static_cast<float>(polity.member_settlement_ids.size());
        polity.admin_load = member_count * 8.0F + static_cast<float>(polity.controlled_tile_count) * 0.08F +
                            static_cast<float>(polity.contested_tile_count) * 0.25F +
                            average_distance * 1.2F * effects.distance_admin_load_multiplier;
        polity.admin_capacity =
            (40.0F + static_cast<float>(capital->population) * 0.08F + PolityLevelAdminBonus(polity.level)) *
            effects.admin_capacity_multiplier;
        polity.overextension = polity.admin_capacity <= 0.0F
                                   ? 1.0F
                                   : std::max(0.0F, polity.admin_load / polity.admin_capacity - 1.0F) *
                                         effects.overextension_penalty_multiplier;

        polity.budget.food_maintenance =
            member_count * 1.5F + static_cast<float>(polity.controlled_tile_count) * 0.006F;
        polity.budget.wood_maintenance = member_count * 0.32F;
        polity.budget.admin_maintenance = polity.admin_load * 0.055F;
        polity.budget.control_maintenance = static_cast<float>(polity.controlled_tile_count) * 0.012F +
                                            static_cast<float>(polity.contested_tile_count) * 0.055F +
                                            polity.route_maintenance;
        polity.budget.food_surplus = polity.budget.food_income - polity.budget.food_maintenance;
        polity.budget.wood_surplus = polity.budget.wood_income - polity.budget.wood_maintenance;
        polity.budget.wealth_surplus =
            polity.budget.wealth_income - polity.budget.admin_maintenance - polity.budget.control_maintenance;

        const float contested_pressure =
            polity.controlled_tile_count <= 0
                ? 0.0F
                : static_cast<float>(polity.contested_tile_count) / static_cast<float>(polity.controlled_tile_count);
        const float deficit_pressure =
            polity.budget.wealth_surplus < 0.0F ? std::min(0.35F, -polity.budget.wealth_surplus * 0.025F) : 0.0F;
        polity.stability =
            std::clamp(1.0F - polity.overextension * 0.45F -
                           contested_pressure * 0.35F * effects.contested_stability_loss_multiplier - deficit_pressure,
                       0.10F, 1.0F);
        polity.legitimacy = std::clamp(1.0F - polity.overextension * 0.25F - deficit_pressure * 0.20F, 0.15F, 1.0F);
        polity.control_power =
            (38.0F + static_cast<float>(polity.population) * 0.045F) * (0.65F + polity.stability * 0.35F);
    }
}

void FoundEligiblePolities(World& world, const SimulationParams& params, Turn turn,
                           std::vector<Settlement>& settlements, std::vector<Polity>& polities, EventLog& event_log) {
    for (auto& settlement : settlements) {
        if (settlement.polity_id != kInvalidPolityId || !IsEligibleCapital(settlement)) {
            continue;
        }
        if (HasNearbyCapital(world, settlements, polities, settlement, params.enable_routes)) {
            continue;
        }

        const PolityId id = NextPolityId(polities);
        Polity polity;
        polity.id = id;
        polity.name = "Chiefdom " + std::to_string(id);
        polity.level = PolityLevel::Chiefdom;
        polity.capital_settlement_id = settlement.id;
        polity.member_settlement_ids.push_back(settlement.id);
        polity.debug_color_index = static_cast<std::uint32_t>(id);
        polity.founded_turn = turn;
        settlement.polity_id = id;
        settlement.is_capital = true;
        polities.push_back(polity);

        std::ostringstream summary;
        summary << polity.name << " founded at settlement " << settlement.id << " (" << settlement.x << ","
                << settlement.y << "), pop " << settlement.population;
        event_log.Add(SimEvent{
            .turn = turn,
            .type = EventType::PolityFounded,
            .actor_id = settlement.id,
            .x = settlement.x,
            .y = settlement.y,
            .summary = summary.str(),
        });
    }
}

void JoinNearbySettlements(World& world, const SimulationParams& params, Turn turn,
                           std::vector<Settlement>& settlements, std::vector<Polity>& polities, EventLog& event_log) {
    for (auto& settlement : settlements) {
        if (settlement.polity_id != kInvalidPolityId) {
            continue;
        }

        Polity* best_polity = nullptr;
        float best_cost = 1.0e9F;
        for (auto& polity : polities) {
            const auto* capital = SettlementById(settlements, polity.capital_settlement_id);
            if (capital == nullptr) {
                continue;
            }
            const auto effects = ComputeTechEffects(polity.research);
            const float cost =
                TerrainPathCost(world, settlement.x, settlement.y, capital->x, capital->y, polity.admin_range,
                                effects.control_path_cost_multiplier, effects.coastal_control_cost_multiplier,
                                polity.id, params.enable_routes);
            if (cost <= polity.admin_range && cost < best_cost) {
                best_cost = cost;
                best_polity = &polity;
            }
        }

        if (best_polity == nullptr) {
            continue;
        }

        settlement.polity_id = best_polity->id;
        settlement.is_capital = false;
        if (!ContainsSettlement(*best_polity, settlement.id)) {
            best_polity->member_settlement_ids.push_back(settlement.id);
        }

        std::ostringstream summary;
        summary << "Settlement " << settlement.id << " joined " << best_polity->name << ", path cost " << best_cost;
        event_log.Add(SimEvent{
            .turn = turn,
            .type = EventType::SettlementJoinedPolity,
            .actor_id = settlement.id,
            .x = settlement.x,
            .y = settlement.y,
            .summary = summary.str(),
        });
    }
}

} // namespace

void PolitySystem::Reset(World& world, std::vector<Settlement>& settlements, std::vector<Polity>& polities) {
    polities.clear();
    for (auto& settlement : settlements) {
        settlement.polity_id = kInvalidPolityId;
        settlement.is_capital = false;
    }
    for (auto& tile : world.Tiles()) {
        tile.owner_polity_id = kInvalidPolityId;
        tile.controller_polity_id = kInvalidPolityId;
        tile.control_strength = 0.0F;
        tile.is_contested = false;
    }
}

void PolitySystem::UpdatePolities(World& world, Turn turn, std::vector<Settlement>& settlements,
                                  std::vector<Polity>& polities, EventLog& event_log) {
    UpdatePolities(world, SimulationParams{}, turn, settlements, polities, event_log);
}

void PolitySystem::UpdatePolities(World& world, const SimulationParams& params, Turn turn,
                                  std::vector<Settlement>& settlements, std::vector<Polity>& polities,
                                  EventLog& event_log) {
    FoundEligiblePolities(world, params, turn, settlements, polities, event_log);
    RecalculatePolityAggregates(settlements, polities);
    JoinNearbySettlements(world, params, turn, settlements, polities, event_log);
    RecalculatePolityAggregates(settlements, polities);
    RecalculatePolityBudgetsAndAdministration(world, settlements, polities, params.enable_routes);

    std::vector<int> previous_counts;
    previous_counts.reserve(polities.size());
    for (const auto& polity : polities) {
        previous_counts.push_back(polity.controlled_tile_count);
    }

    ControlFieldParams control_params;
    control_params.use_routes = params.enable_routes;
    const ControlFieldStats stats = RecomputeControlField(world, settlements, polities, control_params);
    RecalculatePolityBudgetsAndAdministration(world, settlements, polities, params.enable_routes);
    if (turn > 0 && turn % 25 == 0) {
        for (std::size_t i = 0; i < polities.size(); ++i) {
            if (polities[i].controlled_tile_count > previous_counts[i]) {
                const auto* capital = SettlementById(settlements, polities[i].capital_settlement_id);
                event_log.Add(SimEvent{
                    .turn = turn,
                    .type = EventType::ControlFieldExpanded,
                    .actor_id = polities[i].capital_settlement_id,
                    .x = capital == nullptr ? 0 : capital->x,
                    .y = capital == nullptr ? 0 : capital->y,
                    .summary = polities[i].name + " controls " + std::to_string(polities[i].controlled_tile_count) +
                               " land tiles",
                });
            }
        }
        if (stats.contested_tiles > 0) {
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::ContestedRegionFormed,
                .actor_id = -1,
                .x = 0,
                .y = 0,
                .summary = std::to_string(stats.contested_tiles) + " contested tiles detected",
            });
        }
    }
}

} // namespace oikumene
