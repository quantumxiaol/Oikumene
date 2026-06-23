#include "oikumene/sim/settlement_system.hpp"

#include <algorithm>
#include <sstream>

namespace oikumene {
namespace {

float UpgradeReadiness(const Settlement& settlement) {
    if (settlement.level == SettlementLevel::Village) {
        return 1.0F;
    }

    const float population = std::min(1.0F, static_cast<float>(settlement.population) / 80.0F);
    const float age = std::min(1.0F, static_cast<float>(settlement.turns_since_founded) / 20.0F);
    const float food = std::min(1.0F, settlement.stockpile.food / 60.0F);
    const float wood = std::min(1.0F, settlement.stockpile.wood / 30.0F);
    return std::min(std::min(population, age), std::min(food, wood));
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

void SettlementSystem::UpdateSettlements(const World& world,
                                         const SimulationParams& params,
                                         Turn turn,
                                         std::vector<Settlement>& settlements,
                                         EventLog& event_log) {
    for (auto& settlement : settlements) {
        ++settlement.turns_since_founded;
        settlement.local_food_output_last_turn = LocalFoodOutput(world, settlement, params);
        settlement.local_wood_output_last_turn = LocalWoodOutput(world, settlement, params);
        settlement.food_consumption_last_turn =
            static_cast<float>(settlement.population) * params.settlement_food_consumption_per_person;
        settlement.stockpile.food += settlement.local_food_output_last_turn;
        settlement.stockpile.wood += settlement.local_wood_output_last_turn;
        settlement.stockpile.food -= settlement.food_consumption_last_turn;

        if (settlement.stockpile.food > static_cast<float>(settlement.population) * 0.55F) {
            const int growth = std::max(1, static_cast<int>(static_cast<float>(settlement.population) * 0.018F));
            settlement.population += growth;
            settlement.stockpile.food -= static_cast<float>(growth) * 0.7F;
            std::ostringstream summary;
            summary << "Camp " << settlement.id << " population grew to " << settlement.population << ", food surplus "
                    << settlement.stockpile.food << ", food output " << settlement.local_food_output_last_turn
                    << ", wood output " << settlement.local_wood_output_last_turn;
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
            const int loss = std::max(1, static_cast<int>(static_cast<float>(settlement.population) * 0.025F));
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

        settlement.upgrade_readiness = UpgradeReadiness(settlement);
        if (settlement.level == SettlementLevel::Camp && settlement.population >= 80 &&
            settlement.turns_since_founded >= 20 && settlement.stockpile.food >= 60.0F &&
            settlement.stockpile.wood >= 30.0F) {
            settlement.level = SettlementLevel::Village;
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

}  // namespace oikumene
