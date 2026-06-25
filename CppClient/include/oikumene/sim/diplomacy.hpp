#pragma once

#include <string>

#include "oikumene/core/types.hpp"

namespace oikumene {

enum class DiplomaticPosture {
    Neutral,
    Friendly,
    Competitive,
    Dependent,
    BlockadeRisk,
};

enum class DiplomaticIncidentKind {
    None,
    TerritoryCeded,
    OccupationWithdrawn,
    VassalCreated,
    OccupationRevolt,
};

[[nodiscard]] std::string ToString(DiplomaticPosture posture);
[[nodiscard]] std::string ToString(DiplomaticIncidentKind kind);

struct DiplomacyRelation {
    int id = -1;
    PolityId polity_a_id = kInvalidPolityId;
    PolityId polity_b_id = kInvalidPolityId;

    bool has_active_trade = false;
    int trade_id = -1;
    float trade_profit = 0.0F;
    float trade_complementarity = 0.0F;
    float trade_route_efficiency = 0.0F;
    float trade_route_cost = 0.0F;

    float border_tension = 0.0F;
    float economic_overlap = 0.0F;
    float dependence_a_on_b = 0.0F;
    float dependence_b_on_a = 0.0F;
    PolityId dependent_polity_id = kInvalidPolityId;
    PolityId leverage_polity_id = kInvalidPolityId;
    int active_vassal_treaty_id = -1;
    PolityId treaty_overlord_polity_id = kInvalidPolityId;
    PolityId treaty_subject_polity_id = kInvalidPolityId;
    float treaty_strength = 0.0F;
    float treaty_loyalty = 0.0F;
    float treaty_liberty_desire = 0.0F;
    float treaty_tribute_rate = 0.0F;

    float friendship = 0.0F;
    float competition = 0.0F;
    float blockade_tendency = 0.0F;

    float grievance_a_to_b = 0.0F;
    float grievance_b_to_a = 0.0F;
    float vassalage_a_to_b = 0.0F;
    float vassalage_b_to_a = 0.0F;
    float restraint_a_to_b = 0.0F;
    float restraint_b_to_a = 0.0F;
    Turn memory_updated_turn = 0;
    Turn last_incident_turn = -1;
    DiplomaticIncidentKind last_incident = DiplomaticIncidentKind::None;
    int incident_count = 0;

    DiplomaticPosture posture = DiplomaticPosture::Neutral;
    std::string reason;
};

} // namespace oikumene
