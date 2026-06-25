#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "oikumene/sim/war_system.hpp"

namespace {

oikumene::World MakeWorld() {
    oikumene::World world(10, 6, 55);
    for (auto& tile : world.Tiles()) {
        tile.biome = oikumene::Biome::Grassland;
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.45F;
        tile.defense_bonus = 0.06F;
    }
    world.At(7, 2).controller_polity_id = 1;
    world.At(7, 2).settlement_id = 1;
    return world;
}

std::vector<oikumene::Settlement> MakeSettlements(float food = 180.0F, float ore = 18.0F) {
    oikumene::Settlement actor;
    actor.id = 0;
    actor.x = 1;
    actor.y = 2;
    actor.polity_id = 0;
    actor.is_capital = true;
    actor.population = 260;
    actor.level = oikumene::SettlementLevel::Village;
    actor.stockpile.food = food;
    actor.stockpile.wood = 40.0F;
    actor.stockpile.ore = ore;

    oikumene::Settlement defender;
    defender.id = 1;
    defender.x = 7;
    defender.y = 2;
    defender.polity_id = 1;
    defender.is_capital = true;
    defender.population = 210;
    defender.level = oikumene::SettlementLevel::Village;
    defender.stockpile.food = 80.0F;

    return {actor, defender};
}

std::vector<oikumene::Polity> MakePolities() {
    return {
        oikumene::Polity{
            .id = 0,
            .name = "Attacker",
            .capital_settlement_id = 0,
            .member_settlement_ids = {0},
            .population = 260,
            .stability = 0.88F,
            .military_potential = 46.0F,
            .tool_efficiency = 1.22F,
        },
        oikumene::Polity{
            .id = 1,
            .name = "Defender",
            .capital_settlement_id = 1,
            .member_settlement_ids = {1},
            .population = 210,
            .stability = 0.72F,
            .military_potential = 12.0F,
            .tool_efficiency = 0.95F,
        },
    };
}

oikumene::WarTargetCandidate MakeTarget(float action_score = 0.86F) {
    return oikumene::WarTargetCandidate{
        .id = 3,
        .pressure_id = 1,
        .actor_polity_id = 0,
        .target_polity_id = 1,
        .objective = oikumene::WarObjective::BorderDispute,
        .kind = oikumene::WarTargetKind::Settlement,
        .x = 7,
        .y = 2,
        .target_settlement_id = 1,
        .target_trade_id = -1,
        .path = {{1, 2}, {3, 2}, {5, 2}, {7, 2}},
        .route_cost = 6.0F,
        .target_value = 1.10F,
        .mobilization_cost = 0.24F,
        .supply_cost = 0.20F,
        .equipment_cost = 0.16F,
        .terrain_attrition = 0.08F,
        .defender_power = 0.30F,
        .defense_cost = 0.28F,
        .occupation_cost = 0.28F,
        .campaign_cost = 1.08F,
        .roi = 1.02F,
        .action_score = action_score,
        .high_value = action_score >= 0.64F,
    };
}

int CountEvents(const oikumene::EventLog& log, oikumene::EventType type) {
    int count = 0;
    for (const auto& event : log.Events()) {
        count += event.type == type ? 1 : 0;
    }
    return count;
}

const oikumene::WarCampaign* FirstCampaign(const std::vector<oikumene::WarCampaign>& campaigns) {
    return campaigns.empty() ? nullptr : &campaigns.front();
}

void TestHighScoreTargetOpensAndOccupies() {
    auto world = MakeWorld();
    auto settlements = MakeSettlements();
    auto polities = MakePolities();
    std::vector<oikumene::WarCampaign> campaigns;
    oikumene::EventLog log;
    const std::vector<oikumene::WarTargetCandidate> targets{MakeTarget()};

    for (int turn = 10; turn < 28; ++turn) {
        oikumene::WarSystem::UpdateWars(world, turn, settlements, polities, {}, targets, campaigns, log);
    }

    const auto* campaign = FirstCampaign(campaigns);
    assert(campaign != nullptr);
    assert(campaign->status == oikumene::WarCampaignStatus::Occupied);
    assert(campaign->food_spent > 0.0F);
    assert(campaign->equipment_spent > 0.0F);
    assert(settlements[1].polity_id == 0);
    assert(!settlements[1].is_capital);
    assert(CountEvents(log, oikumene::EventType::WarDeclared) == 1);
    assert(CountEvents(log, oikumene::EventType::WarTargetOccupied) == 1);
}

void TestLowScoreTargetDoesNotOpenWar() {
    auto world = MakeWorld();
    auto settlements = MakeSettlements();
    auto polities = MakePolities();
    std::vector<oikumene::WarCampaign> campaigns;
    oikumene::EventLog log;
    const std::vector<oikumene::WarTargetCandidate> targets{MakeTarget(0.42F)};

    oikumene::WarSystem::UpdateWars(world, 10, settlements, polities, {}, targets, campaigns, log);

    assert(campaigns.empty());
    assert(CountEvents(log, oikumene::EventType::WarDeclared) == 0);
}

void TestSupplyFailureCausesRetreat() {
    auto world = MakeWorld();
    auto settlements = MakeSettlements(0.0F, 0.0F);
    auto polities = MakePolities();
    std::vector<oikumene::WarCampaign> campaigns;
    oikumene::EventLog log;
    auto target = MakeTarget();
    target.campaign_cost = 3.2F;
    target.occupation_cost = 0.5F;
    const std::vector<oikumene::WarTargetCandidate> targets{target};

    for (int turn = 10; turn < 28; ++turn) {
        oikumene::WarSystem::UpdateWars(world, turn, settlements, polities, {}, targets, campaigns, log);
    }

    const auto* campaign = FirstCampaign(campaigns);
    assert(campaign != nullptr);
    assert(campaign->status == oikumene::WarCampaignStatus::Withdrawn);
    assert(campaign->supply_failures >= 4);
    assert(settlements[1].polity_id == 1);
    assert(CountEvents(log, oikumene::EventType::WarRetreated) == 1);
}

} // namespace

int main() {
    TestHighScoreTargetOpensAndOccupies();
    TestLowScoreTargetDoesNotOpenWar();
    TestSupplyFailureCausesRetreat();
    std::cout << "war system tests passed\n";
    return 0;
}
