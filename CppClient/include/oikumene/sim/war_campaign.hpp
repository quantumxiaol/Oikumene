#pragma once

#include <string>
#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/pathfinding.hpp"
#include "oikumene/sim/war.hpp"
#include "oikumene/sim/war_target.hpp"

namespace oikumene {

enum class WarCampaignStatus {
    Active,
    Occupied,
    Withdrawn,
    Peace,
};

[[nodiscard]] std::string ToString(WarCampaignStatus status);

struct WarCampaign {
    int id = -1;
    int target_candidate_id = -1;
    PolityId actor_polity_id = kInvalidPolityId;
    PolityId target_polity_id = kInvalidPolityId;
    WarObjective objective = WarObjective::BorderDispute;
    WarTargetKind target_kind = WarTargetKind::Settlement;
    WarCampaignStatus status = WarCampaignStatus::Active;

    int x = 0;
    int y = 0;
    int target_settlement_id = -1;
    int target_trade_id = -1;
    std::vector<TileCoord> path;
    Turn started_turn = 0;
    Turn ended_turn = -1;
    Turn last_update_turn = 0;
    int turns_active = 0;

    float target_value = 0.0F;
    float campaign_cost = 0.0F;
    float occupation_cost = 0.0F;
    float action_score = 0.0F;
    float progress = 0.0F;
    float mobilized_manpower = 0.0F;
    float attacker_strength = 0.0F;
    float defender_strength = 0.0F;

    float population_lost = 0.0F;
    float food_spent = 0.0F;
    float equipment_spent = 0.0F;
    float occupation_profit = 0.0F;
    int supply_failures = 0;
    std::string outcome_reason;
};

} // namespace oikumene
