#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "oikumene/sim/war_target_planner.hpp"

namespace {

oikumene::World MakeWarTargetWorld() {
    oikumene::World world(12, 8, 123);
    for (auto& tile : world.Tiles()) {
        tile.biome = oikumene::Biome::Grassland;
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.42F;
        tile.soil_quality = 0.45F;
        tile.defense_bonus = 0.08F;
    }

    auto& copper = world.At(8, 3);
    copper.controller_polity_id = 1;
    copper.resource = oikumene::ResourceKind::Copper;
    copper.resource_amount = 0.92F;
    copper.biome = oikumene::Biome::Hill;
    copper.defense_bonus = 0.24F;

    auto& contested = world.At(6, 2);
    contested.controller_polity_id = 1;
    contested.is_contested = true;
    contested.has_river = true;
    contested.fertility = 0.72F;

    return world;
}

std::vector<oikumene::Settlement> MakeSettlements() {
    return {
        oikumene::Settlement{
            .id = 0,
            .x = 1,
            .y = 2,
            .polity_id = 0,
            .is_capital = true,
            .population = 260,
            .level = oikumene::SettlementLevel::Village,
        },
        oikumene::Settlement{
            .id = 1,
            .x = 9,
            .y = 2,
            .polity_id = 1,
            .is_capital = true,
            .population = 340,
            .level = oikumene::SettlementLevel::Village,
            .local_food_output_last_turn = 4.2F,
            .ore_output_last_turn = 0.3F,
        },
    };
}

std::vector<oikumene::Polity> MakePolities() {
    return {
        oikumene::Polity{
            .id = 0,
            .capital_settlement_id = 0,
            .member_settlement_ids = {0},
            .population = 260,
            .stability = 0.82F,
            .military_potential = 32.0F,
            .tool_efficiency = 1.15F,
        },
        oikumene::Polity{
            .id = 1,
            .capital_settlement_id = 1,
            .member_settlement_ids = {1},
            .population = 340,
            .stability = 0.78F,
            .military_potential = 22.0F,
            .tool_efficiency = 1.00F,
        },
    };
}

oikumene::WarPressure MakePressure(float friendly_penalty = 0.05F) {
    return oikumene::WarPressure{
        .id = 4,
        .relation_id = 2,
        .actor_polity_id = 0,
        .target_polity_id = 1,
        .objective = oikumene::WarObjective::TradeCoercion,
        .military_ratio = 1.25F,
        .military_advantage = 0.48F,
        .border_pressure = 0.45F,
        .trade_conflict_weight = 0.66F,
        .dependency_pressure = 0.38F,
        .blockade_pressure = 0.32F,
        .friendly_penalty = friendly_penalty,
        .target_value = 0.62F,
        .campaign_cost = 0.72F,
        .base_roi = 0.86F,
        .diplomatic_modifier = 1.08F,
        .war_roi = 0.64F,
        .declaration_pressure = 0.78F,
        .high_pressure = true,
    };
}

const oikumene::WarTargetCandidate* FirstKind(const std::vector<oikumene::WarTargetCandidate>& candidates,
                                              oikumene::WarTargetKind kind) {
    const auto it = std::find_if(candidates.begin(), candidates.end(),
                                 [&](const auto& candidate) { return candidate.kind == kind; });
    return it == candidates.end() ? nullptr : &(*it);
}

const oikumene::WarTargetCandidate* TargetAt(const std::vector<oikumene::WarTargetCandidate>& candidates,
                                             oikumene::WarTargetKind kind, int x, int y) {
    const auto it = std::find_if(candidates.begin(), candidates.end(), [&](const auto& candidate) {
        return candidate.kind == kind && candidate.x == x && candidate.y == y;
    });
    return it == candidates.end() ? nullptr : &(*it);
}

void TestResourceTargetCandidate() {
    const auto world = MakeWarTargetWorld();
    const auto settlements = MakeSettlements();
    const auto polities = MakePolities();

    const auto candidates = oikumene::BuildWarTargetCandidates(world, settlements, polities, {}, {MakePressure()});
    const auto* resource = TargetAt(candidates, oikumene::WarTargetKind::ResourceRegion, 8, 3);

    assert(resource != nullptr);
    assert(resource->resource_value > 0.70F);
    assert(resource->target_value > 0.30F);
    assert(resource->campaign_cost > resource->occupation_cost);
    assert(!resource->path.empty());
}

void TestFriendlyPenaltyLowersTargetRoi() {
    const auto world = MakeWarTargetWorld();
    const auto settlements = MakeSettlements();
    const auto polities = MakePolities();

    const auto hostile = oikumene::BuildWarTargetCandidates(world, settlements, polities, {}, {MakePressure(0.02F)});
    const auto friendly = oikumene::BuildWarTargetCandidates(world, settlements, polities, {}, {MakePressure(0.88F)});
    const auto* hostile_resource = TargetAt(hostile, oikumene::WarTargetKind::ResourceRegion, 8, 3);
    const auto* friendly_resource = TargetAt(friendly, oikumene::WarTargetKind::ResourceRegion, 8, 3);

    assert(hostile_resource != nullptr);
    assert(friendly_resource != nullptr);
    assert(friendly_resource->occupation_cost > hostile_resource->occupation_cost);
    assert(friendly_resource->roi < hostile_resource->roi);
}

void TestTradeRouteNodeCandidate() {
    const auto world = MakeWarTargetWorld();
    const auto settlements = MakeSettlements();
    const auto polities = MakePolities();
    const std::vector<oikumene::TradeAgreement> trades{
        oikumene::TradeAgreement{
            .id = 9,
            .polity_a_id = 0,
            .polity_b_id = 1,
            .active = true,
            .weak_refresh_count = 2,
            .complementarity = 1.4F,
            .route_cost = 28.0F,
            .route_efficiency = 0.25F,
            .expected_profit = 1.8F,
            .path = {{1, 2}, {3, 2}, {5, 2}, {7, 2}, {9, 2}},
        },
    };

    const auto candidates = oikumene::BuildWarTargetCandidates(world, settlements, polities, trades, {MakePressure()});
    const auto* trade_node = FirstKind(candidates, oikumene::WarTargetKind::TradeRouteNode);

    assert(trade_node != nullptr);
    assert(trade_node->target_trade_id == 9);
    assert(trade_node->trade_value > 0.55F);
    assert(trade_node->x == 5 && trade_node->y == 2);
}

} // namespace

int main() {
    TestResourceTargetCandidate();
    TestFriendlyPenaltyLowersTargetRoi();
    TestTradeRouteNodeCandidate();
    std::cout << "war target planner tests passed\n";
    return 0;
}
