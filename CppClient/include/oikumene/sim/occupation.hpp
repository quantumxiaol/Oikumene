#pragma once

#include <string>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/war_target.hpp"

namespace oikumene {

enum class OccupationStatus {
    Active,
    Ceded,
    Withdrawn,
    Vassalized,
    Revolted,
};

[[nodiscard]] std::string ToString(OccupationStatus status);

struct OccupationRecord {
    int id = -1;
    int source_campaign_id = -1;
    PolityId occupier_polity_id = kInvalidPolityId;
    PolityId previous_owner_polity_id = kInvalidPolityId;
    PolityId subject_polity_id = kInvalidPolityId;
    int vassal_treaty_id = -1;
    WarTargetKind target_kind = WarTargetKind::Settlement;
    int settlement_id = -1;
    int x = 0;
    int y = 0;
    OccupationStatus status = OccupationStatus::Active;

    Turn started_turn = 0;
    Turn ended_turn = -1;
    Turn last_update_turn = 0;
    int turns_held = 0;

    float occupation_value = 0.0F;
    float maintenance_cost = 0.0F;
    float cumulative_maintenance = 0.0F;
    float cumulative_shortfall = 0.0F;
    float unrest = 0.0F;
    float integration = 0.0F;
    float revolt_risk = 0.0F;
    float border_stability_delta = 0.0F;
    std::string outcome_reason;
};

} // namespace oikumene
