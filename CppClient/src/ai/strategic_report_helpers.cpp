#include "strategic_report_helpers.hpp"

#include <algorithm>

namespace oikumene::strategic_report {

float Clamp01(float value) {
    return std::clamp(value, 0.0F, 1.0F);
}

const Polity* PolityById(const std::vector<Polity>& polities, PolityId id) {
    for (const auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
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

bool InvolvesPolity(const DiplomacyRelation& relation, PolityId id) {
    return relation.polity_a_id == id || relation.polity_b_id == id;
}

PolityId OtherPolity(const DiplomacyRelation& relation, PolityId id) {
    if (relation.polity_a_id == id) {
        return relation.polity_b_id;
    }
    if (relation.polity_b_id == id) {
        return relation.polity_a_id;
    }
    return kInvalidPolityId;
}

float DirectedValue(float a_to_b, float b_to_a, const DiplomacyRelation& relation, PolityId from_id) {
    return from_id == relation.polity_a_id ? a_to_b : b_to_a;
}

float MaxRelationRisk(const DiplomacyRelation& relation) {
    return std::max({relation.competition, relation.blockade_tendency, relation.treaty_liberty_desire,
                     relation.grievance_a_to_b, relation.grievance_b_to_a});
}

float RelationSortScore(const DiplomacyRelation& relation, PolityId polity_id) {
    const float directed_grievance =
        DirectedValue(relation.grievance_a_to_b, relation.grievance_b_to_a, relation, polity_id);
    const float directed_dependence =
        DirectedValue(relation.dependence_a_on_b, relation.dependence_b_on_a, relation, polity_id);
    return relation.border_tension + relation.blockade_tendency + directed_grievance + directed_dependence +
           relation.treaty_liberty_desire + (relation.has_active_trade ? 0.12F : 0.0F);
}

} // namespace oikumene::strategic_report
