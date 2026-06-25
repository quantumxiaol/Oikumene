#include "oikumene/sim/vassal_system.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace oikumene {
namespace {

float Clamp01(float value) {
    return std::clamp(value, 0.0F, 1.0F);
}

Polity* PolityById(std::vector<Polity>& polities, PolityId id) {
    for (auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

const Polity* PolityById(const std::vector<Polity>& polities, PolityId id) {
    for (const auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

bool SameTreatyPair(const VassalTreaty& treaty, PolityId overlord_id, PolityId subject_id) {
    return treaty.overlord_polity_id == overlord_id && treaty.subject_polity_id == subject_id;
}

int NextTreatyId(const std::vector<VassalTreaty>& treaties) {
    int next = 0;
    for (const auto& treaty : treaties) {
        next = std::max(next, treaty.id + 1);
    }
    return next;
}

void ResetPolityVassalState(std::vector<Polity>& polities) {
    for (auto& polity : polities) {
        polity.overlord_polity_id = kInvalidPolityId;
        polity.active_overlord_treaty_id = -1;
        polity.subject_treaty_ids.clear();
        polity.vassal_count = 0;
        polity.vassal_tribute_income = 0.0F;
        polity.vassal_tribute_paid = 0.0F;
        polity.vassal_liberty_desire = 0.0F;
        polity.overlord_protection = 0.0F;
    }
}

float GrievanceFromSubjectToOverlord(const DiplomacyRelation& relation, PolityId subject_id, PolityId overlord_id) {
    if (relation.polity_a_id == subject_id && relation.polity_b_id == overlord_id) {
        return relation.grievance_a_to_b;
    }
    if (relation.polity_b_id == subject_id && relation.polity_a_id == overlord_id) {
        return relation.grievance_b_to_a;
    }
    return 0.0F;
}

float VassalageFromSubjectToOverlord(const DiplomacyRelation& relation, PolityId subject_id, PolityId overlord_id) {
    if (relation.polity_a_id == subject_id && relation.polity_b_id == overlord_id) {
        return relation.vassalage_a_to_b;
    }
    if (relation.polity_b_id == subject_id && relation.polity_a_id == overlord_id) {
        return relation.vassalage_b_to_a;
    }
    return 0.0F;
}

const DiplomacyRelation* RelationForPair(const std::vector<DiplomacyRelation>& relations, PolityId a, PolityId b) {
    for (const auto& relation : relations) {
        if ((relation.polity_a_id == a && relation.polity_b_id == b) ||
            (relation.polity_a_id == b && relation.polity_b_id == a)) {
            return &relation;
        }
    }
    return nullptr;
}

void UpdateTreatyTerms(Turn turn, const Polity& overlord, const Polity& subject,
                       const std::vector<DiplomacyRelation>& relations, VassalTreaty& treaty) {
    const auto* relation = RelationForPair(relations, overlord.id, subject.id);
    const float grievance =
        relation == nullptr ? 0.0F : GrievanceFromSubjectToOverlord(*relation, subject.id, overlord.id);
    const float memory_vassalage =
        relation == nullptr ? 0.0F : VassalageFromSubjectToOverlord(*relation, subject.id, overlord.id);
    const float military_ratio =
        (subject.military_potential + 6.0F) / std::max(8.0F, overlord.military_potential + 6.0F);

    treaty.last_update_turn = turn;
    treaty.strength = Clamp01(std::max(treaty.strength, memory_vassalage * 0.65F));
    treaty.autonomy = Clamp01(0.72F - treaty.strength * 0.28F + subject.stability * 0.08F);
    treaty.tribute_rate = Clamp01(0.08F + treaty.strength * 0.14F);
    treaty.protection = Clamp01(0.24F + treaty.strength * 0.30F + overlord.military_potential / 160.0F);
    treaty.tribute_due = std::max(0.0F, subject.budget.wealth_surplus) * treaty.tribute_rate +
                         std::max(0.0F, subject.budget.food_surplus) * treaty.tribute_rate * 0.025F;
    treaty.military_obligation = Clamp01(treaty.strength * 0.35F + (1.0F - treaty.autonomy) * 0.28F);
    treaty.loyalty =
        Clamp01(0.42F + subject.stability * 0.20F + treaty.protection * 0.16F + treaty.strength * 0.18F +
                memory_vassalage * 0.10F - grievance * 0.28F - subject.overextension * 0.10F - military_ratio * 0.08F);
    treaty.liberty_desire =
        Clamp01(0.22F + military_ratio * 0.28F + (1.0F - subject.stability) * 0.22F + grievance * 0.28F -
                treaty.autonomy * 0.12F - treaty.strength * 0.08F - treaty.protection * 0.07F);

    std::ostringstream reason;
    reason << "vassal treaty P" << subject.id << " under P" << overlord.id << " strength " << treaty.strength
           << " loyalty " << treaty.loyalty << " liberty " << treaty.liberty_desire << " tribute "
           << treaty.tribute_due;
    treaty.reason = reason.str();
}

} // namespace

const VassalTreaty* ActiveVassalTreatyForPair(const std::vector<VassalTreaty>& treaties, PolityId a, PolityId b) {
    for (const auto& treaty : treaties) {
        if (treaty.status != VassalTreatyStatus::Active) {
            continue;
        }
        if ((treaty.overlord_polity_id == a && treaty.subject_polity_id == b) ||
            (treaty.overlord_polity_id == b && treaty.subject_polity_id == a)) {
            return &treaty;
        }
    }
    return nullptr;
}

const VassalTreaty* ActiveVassalTreatyForSubject(const std::vector<VassalTreaty>& treaties, PolityId subject_id) {
    for (const auto& treaty : treaties) {
        if (treaty.status == VassalTreatyStatus::Active && treaty.subject_polity_id == subject_id) {
            return &treaty;
        }
    }
    return nullptr;
}

void VassalSystem::Reset(std::vector<VassalTreaty>& treaties, std::vector<Polity>& polities) {
    treaties.clear();
    ResetPolityVassalState(polities);
}

int VassalSystem::CreateOrRefreshTreaty(std::vector<VassalTreaty>& treaties, Turn turn, PolityId overlord_id,
                                        PolityId subject_id, int source_occupation_id, float strength) {
    if (overlord_id == kInvalidPolityId || subject_id == kInvalidPolityId || overlord_id == subject_id) {
        return -1;
    }
    strength = Clamp01(strength);
    for (auto& treaty : treaties) {
        if (SameTreatyPair(treaty, overlord_id, subject_id)) {
            treaty.status = VassalTreatyStatus::Active;
            treaty.ended_turn = -1;
            treaty.source_occupation_id = source_occupation_id;
            treaty.last_update_turn = turn;
            treaty.strength = Clamp01(std::max(treaty.strength, 0.45F + strength * 0.40F));
            treaty.reason = "refreshed by occupation settlement";
            return treaty.id;
        }
    }

    VassalTreaty treaty;
    treaty.id = NextTreatyId(treaties);
    treaty.source_occupation_id = source_occupation_id;
    treaty.overlord_polity_id = overlord_id;
    treaty.subject_polity_id = subject_id;
    treaty.started_turn = turn;
    treaty.last_update_turn = turn;
    treaty.strength = Clamp01(0.45F + strength * 0.40F);
    treaty.autonomy = Clamp01(0.72F - treaty.strength * 0.28F);
    treaty.tribute_rate = Clamp01(0.08F + treaty.strength * 0.14F);
    treaty.protection = Clamp01(0.24F + treaty.strength * 0.30F);
    treaty.loyalty = Clamp01(0.40F + treaty.strength * 0.22F);
    treaty.liberty_desire = Clamp01(0.38F - treaty.strength * 0.12F);
    treaty.reason = "created by occupation settlement";
    treaties.push_back(treaty);
    return treaties.back().id;
}

void VassalSystem::UpdateTreaties(Turn turn, std::vector<Polity>& polities,
                                  const std::vector<DiplomacyRelation>& diplomacy_relations,
                                  std::vector<VassalTreaty>& treaties) {
    ResetPolityVassalState(polities);
    for (auto& treaty : treaties) {
        if (treaty.status != VassalTreatyStatus::Active) {
            continue;
        }
        auto* overlord = PolityById(polities, treaty.overlord_polity_id);
        auto* subject = PolityById(polities, treaty.subject_polity_id);
        if (overlord == nullptr || subject == nullptr) {
            treaty.status = VassalTreatyStatus::Broken;
            treaty.ended_turn = turn;
            treaty.last_update_turn = turn;
            treaty.reason = "overlord or subject polity disappeared";
            continue;
        }

        UpdateTreatyTerms(turn, *overlord, *subject, diplomacy_relations, treaty);
        overlord->subject_treaty_ids.push_back(treaty.id);
        ++overlord->vassal_count;
        overlord->vassal_tribute_income += treaty.tribute_due;
        subject->overlord_polity_id = overlord->id;
        subject->active_overlord_treaty_id = treaty.id;
        subject->vassal_tribute_paid += treaty.tribute_due;
        subject->vassal_liberty_desire = std::max(subject->vassal_liberty_desire, treaty.liberty_desire);
        subject->overlord_protection = std::max(subject->overlord_protection, treaty.protection);
    }
}

} // namespace oikumene
