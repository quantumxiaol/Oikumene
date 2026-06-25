#include "oikumene/sim/simulation_metrics.hpp"

#include <algorithm>

namespace oikumene {

int CountActiveBands(const Simulation& sim) {
    int count = 0;
    for (const auto& band : sim.Bands()) {
        count += band.active ? 1 : 0;
    }
    return count;
}

int CountVillages(const Simulation& sim) {
    int count = 0;
    for (const auto& settlement : sim.Settlements()) {
        count += settlement.level == SettlementLevel::Village ? 1 : 0;
    }
    return count;
}

int CountCamps(const Simulation& sim) {
    return static_cast<int>(sim.Settlements().size()) - CountVillages(sim);
}

int CountInactiveBands(const Simulation& sim) {
    return static_cast<int>(sim.Bands().size()) - CountActiveBands(sim);
}

int CountEvents(const Simulation& sim, EventType type) {
    int count = 0;
    for (const auto& event : sim.Events().Events()) {
        count += event.type == type ? 1 : 0;
    }
    return count;
}

int CountImprovement(const Simulation& sim, ImprovementKind kind) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += tile.improvement == kind ? 1 : 0;
    }
    return count;
}

int CountWorkedTiles(const Simulation& sim) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += tile.worked_by_settlement_id >= 0 ? 1 : 0;
    }
    return count;
}

float TotalFoodOutputLastTurn(const Simulation& sim) {
    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += settlement.local_food_output_last_turn;
    }
    return total;
}

float TotalFoodConsumptionLastTurn(const Simulation& sim) {
    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += settlement.food_consumption_last_turn;
    }
    return total;
}

float TotalWoodOutputLastTurn(const Simulation& sim) {
    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += settlement.local_wood_output_last_turn;
    }
    return total;
}

float AverageCarryingCapacity(const Simulation& sim) {
    if (sim.Settlements().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += settlement.carrying_capacity;
    }
    return total / static_cast<float>(sim.Settlements().size());
}

float ControlledLandRatio(const Simulation& sim) {
    int land = 0;
    int controlled = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        if (tile.is_ocean || tile.is_lake) {
            continue;
        }
        ++land;
        controlled += tile.controller_polity_id != kInvalidPolityId ? 1 : 0;
    }
    return land <= 0 ? 0.0F : static_cast<float>(controlled) / static_cast<float>(land);
}

int CountContestedTiles(const Simulation& sim) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += tile.is_contested ? 1 : 0;
    }
    return count;
}

int CountRouteTiles(const Simulation& sim) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += tile.has_route ? 1 : 0;
    }
    return count;
}

int CountRouteKindTiles(const Simulation& sim, RouteKind kind) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += tile.has_route && tile.route_kind == kind ? 1 : 0;
    }
    return count;
}

int CountActiveTrades(const Simulation& sim) {
    int count = 0;
    for (const auto& trade : sim.Trades()) {
        count += trade.active ? 1 : 0;
    }
    return count;
}

float TotalTradeProfit(const Simulation& sim) {
    float total = 0.0F;
    for (const auto& trade : sim.Trades()) {
        if (trade.active) {
            total += trade.expected_profit;
        }
    }
    return total;
}

float AverageTradeRouteEfficiency(const Simulation& sim) {
    float total = 0.0F;
    int count = 0;
    for (const auto& trade : sim.Trades()) {
        if (!trade.active) {
            continue;
        }
        total += trade.route_efficiency;
        ++count;
    }
    return count <= 0 ? 0.0F : total / static_cast<float>(count);
}

} // namespace oikumene
