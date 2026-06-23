#include "oikumene/sim/settlement_system.hpp"

#include <algorithm>
#include <sstream>

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

void SettlementSystem::UpdateSettlements(const World& world,
                                         const SimulationParams& params,
                                         Turn turn,
                                         std::vector<Settlement>& settlements,
                                         EventLog& event_log) {
    for (auto& settlement : settlements) {
        ++settlement.turns_since_founded;
        settlement.stockpile.food += LocalFoodOutput(world, settlement, params);
        settlement.stockpile.wood += LocalWoodOutput(world, settlement, params);
        settlement.stockpile.food -= static_cast<float>(settlement.population) * params.settlement_food_consumption_per_person;

        if (settlement.stockpile.food > static_cast<float>(settlement.population) * 0.55F) {
            const int growth = std::max(1, static_cast<int>(static_cast<float>(settlement.population) * 0.018F));
            settlement.population += growth;
            settlement.stockpile.food -= static_cast<float>(growth) * 0.7F;
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::PopulationGrowth,
                .actor_id = settlement.id,
                .x = settlement.x,
                .y = settlement.y,
                .summary = "Settlement population grew",
            });
        } else if (settlement.stockpile.food < 0.0F) {
            const int loss = std::max(1, static_cast<int>(static_cast<float>(settlement.population) * 0.025F));
            settlement.population = std::max(1, settlement.population - loss);
            settlement.stockpile.food = 0.0F;
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::Famine,
                .actor_id = settlement.id,
                .x = settlement.x,
                .y = settlement.y,
                .summary = "Settlement suffered food shortage",
            });
        }

        if (settlement.level == SettlementLevel::Camp && settlement.population >= 80 &&
            settlement.turns_since_founded >= 20 && settlement.stockpile.food >= 60.0F &&
            settlement.stockpile.wood >= 30.0F) {
            settlement.level = SettlementLevel::Village;
            settlement.stockpile.food -= 35.0F;
            settlement.stockpile.wood -= 18.0F;
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::SettlementUpgraded,
                .actor_id = settlement.id,
                .x = settlement.x,
                .y = settlement.y,
                .summary = "Camp upgraded to village",
            });
        }
    }
}

}  // namespace oikumene
