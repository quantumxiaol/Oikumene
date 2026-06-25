#include "oikumene/sim/settlement_system.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "oikumene/sim/tech_effects.hpp"
#include "settlement_improvement.hpp"
#include "settlement_production.hpp"
#include "settlement_work.hpp"

namespace oikumene {
namespace {

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

void MaybeBuildImprovement(World& world, const SimulationParams& params, Turn turn, Settlement& settlement,
                           const TechEffects& effects, EventLog& event_log) {
    const int existing_improvements =
        settlement_system::CountImprovement(world, settlement, ImprovementKind::Farm) +
        settlement_system::CountImprovement(world, settlement, ImprovementKind::LumberCamp) +
        settlement_system::CountImprovement(world, settlement, ImprovementKind::Pasture) +
        settlement_system::CountImprovement(world, settlement, ImprovementKind::ShallowMine) +
        settlement_system::CountImprovement(world, settlement, ImprovementKind::ForagingGround);
    const int build_interval = settlement.level == SettlementLevel::Village ? params.village_improvement_build_interval
                                                                            : params.camp_improvement_build_interval;
    const bool should_build_now = settlement.turns_since_founded == 1 ||
                                  (build_interval > 0 && settlement.turns_since_founded % build_interval == 0);
    const int improvement_limit =
        settlement_system::WorkSlotsFor(settlement, params) + (settlement.level == SettlementLevel::Village ? 3 : 2);

    if (!should_build_now || existing_improvements >= improvement_limit) {
        return;
    }

    (void)settlement_system::BuildBestImprovement(
        world, settlement, settlement_system::DesiredImprovement(world, settlement, params, effects), turn, event_log);
}

void UpdateSettlementProduction(World& world, const SimulationParams& params, Turn turn, Settlement& settlement,
                                const TechEffects& effects, EventLog& event_log) {
    settlement.local_food_output_last_turn = std::max(0.8F, LocalFoodOutput(world, settlement, params) * 0.16F);
    settlement.local_wood_output_last_turn = 0.0F;
    settlement.ore_output_last_turn = 0.0F;
    settlement.worked_tile_indices = settlement_system::SelectWorkedTiles(world, settlement, params, effects);
    settlement.worked_tile_count = static_cast<int>(settlement.worked_tile_indices.size());

    for (const int index : settlement.worked_tile_indices) {
        settlement_system::ProduceFromWorkedTile(world, settlement, index, effects, params.enable_routes, turn,
                                                 event_log);
    }

    settlement.food_consumption_last_turn =
        static_cast<float>(settlement.population) * params.settlement_food_consumption_per_person;
    settlement.stockpile.food += settlement.local_food_output_last_turn;
    settlement.stockpile.wood += settlement.local_wood_output_last_turn;
    settlement.stockpile.ore += settlement.ore_output_last_turn;
    settlement.stockpile.food -= settlement.food_consumption_last_turn;
    settlement.carrying_capacity = settlement_system::CarryingCapacityFor(world, settlement, effects);
    settlement.carrying_capacity_ratio = settlement.carrying_capacity <= 0.0F
                                             ? 0.0F
                                             : static_cast<float>(settlement.population) / settlement.carrying_capacity;
}

void UpdateSettlementPopulation(const SimulationParams& params, Turn turn, Settlement& settlement,
                                const TechEffects& effects, EventLog& event_log) {
    const float capacity_pressure = std::max(0.0F, 1.0F - settlement.carrying_capacity_ratio);
    const float food_security = std::clamp(
        settlement.stockpile.food / std::max(1.0F, static_cast<float>(settlement.population) * 0.70F), 0.0F, 1.4F);

    if (settlement.stockpile.food > static_cast<float>(settlement.population) * 0.25F && capacity_pressure > 0.025F) {
        const float growth_float = static_cast<float>(settlement.population) * params.settlement_growth_rate *
                                   food_security * capacity_pressure;
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
        return;
    }

    if (settlement.stockpile.food >= 0.0F) {
        return;
    }

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

void MaybeReportCarryingCapacity(Turn turn, const Settlement& settlement, EventLog& event_log) {
    if (settlement.carrying_capacity_ratio < 0.80F || turn % 10 != 0) {
        return;
    }

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

void MaybeUpgradeSettlement(Turn turn, Settlement& settlement, EventLog& event_log) {
    settlement.upgrade_readiness = UpgradeReadiness(settlement);
    if (settlement.level != SettlementLevel::Camp || settlement.population < 70 ||
        settlement.turns_since_founded < 20 || settlement.stockpile.food < 45.0F || settlement.stockpile.wood < 24.0F) {
        return;
    }

    settlement.level = SettlementLevel::Village;
    settlement.work_radius = settlement_system::WorkRadiusFor(settlement);
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

} // namespace

void SettlementSystem::UpdateSettlements(World& world, const SimulationParams& params, Turn turn,
                                         std::vector<Settlement>& settlements, const std::vector<Polity>& polities,
                                         EventLog& event_log) {
    for (auto& settlement : settlements) {
        const TechEffects effects = EffectsForSettlement(settlement, polities);
        ++settlement.turns_since_founded;
        settlement.work_radius = settlement_system::WorkRadiusFor(settlement);

        MaybeBuildImprovement(world, params, turn, settlement, effects, event_log);
        UpdateSettlementProduction(world, params, turn, settlement, effects, event_log);
        UpdateSettlementPopulation(params, turn, settlement, effects, event_log);
        MaybeReportCarryingCapacity(turn, settlement, event_log);
        MaybeUpgradeSettlement(turn, settlement, event_log);
    }
}

void SettlementSystem::UpdateSettlements(World& world, const SimulationParams& params, Turn turn,
                                         std::vector<Settlement>& settlements, EventLog& event_log) {
    const std::vector<Polity> no_polities;
    UpdateSettlements(world, params, turn, settlements, no_polities, event_log);
}

} // namespace oikumene
