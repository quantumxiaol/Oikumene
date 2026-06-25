#include "oikumene/sim/war_planner.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace oikumene {
namespace {

float Clamp01(float value) {
    return std::clamp(value, 0.0F, 1.0F);
}

float SafeRatio(float numerator, float denominator) {
    return numerator / std::max(1.0F, denominator);
}

const Polity* PolityById(const std::vector<Polity>& polities, PolityId id) {
    for (const auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

float DependenceForActor(const DiplomacyRelation& relation, PolityId actor_id) {
    if (actor_id == relation.polity_a_id) {
        return relation.dependence_a_on_b;
    }
    if (actor_id == relation.polity_b_id) {
        return relation.dependence_b_on_a;
    }
    return 0.0F;
}

float DependenceForTarget(const DiplomacyRelation& relation, PolityId target_id) {
    if (target_id == relation.polity_a_id) {
        return relation.dependence_a_on_b;
    }
    if (target_id == relation.polity_b_id) {
        return relation.dependence_b_on_a;
    }
    return 0.0F;
}

float GrievanceFromActorToTarget(const DiplomacyRelation& relation, PolityId actor_id) {
    if (actor_id == relation.polity_a_id) {
        return relation.grievance_a_to_b;
    }
    if (actor_id == relation.polity_b_id) {
        return relation.grievance_b_to_a;
    }
    return 0.0F;
}

float RestraintFromActorToTarget(const DiplomacyRelation& relation, PolityId actor_id) {
    if (actor_id == relation.polity_a_id) {
        return relation.restraint_a_to_b;
    }
    if (actor_id == relation.polity_b_id) {
        return relation.restraint_b_to_a;
    }
    return 0.0F;
}

float VassalageFromActorToTarget(const DiplomacyRelation& relation, PolityId actor_id) {
    const float memory_value = actor_id == relation.polity_a_id   ? relation.vassalage_a_to_b
                               : actor_id == relation.polity_b_id ? relation.vassalage_b_to_a
                                                                  : 0.0F;
    if (actor_id == relation.treaty_subject_polity_id) {
        return std::max(relation.treaty_strength, memory_value);
    }
    return memory_value;
}

bool ActorIsTreatySubject(const DiplomacyRelation& relation, PolityId actor_id) {
    return relation.active_vassal_treaty_id >= 0 && relation.treaty_subject_polity_id == actor_id;
}

bool ActorIsTreatyOverlord(const DiplomacyRelation& relation, PolityId actor_id) {
    return relation.active_vassal_treaty_id >= 0 && relation.treaty_overlord_polity_id == actor_id;
}

WarObjective ChooseObjective(const DiplomacyRelation& relation, PolityId actor_id, float trade_conflict_weight,
                             float border_pressure) {
    if (ActorIsTreatySubject(relation, actor_id) && relation.treaty_liberty_desire >= 0.48F) {
        return WarObjective::DependencyBreakout;
    }
    if (VassalageFromActorToTarget(relation, actor_id) >= 0.45F && trade_conflict_weight >= 0.38F) {
        return WarObjective::DependencyBreakout;
    }
    if (trade_conflict_weight >= 0.58F && actor_id == relation.leverage_polity_id) {
        return WarObjective::Blockade;
    }
    if (trade_conflict_weight >= 0.52F && actor_id == relation.dependent_polity_id) {
        return WarObjective::DependencyBreakout;
    }
    if (trade_conflict_weight >= std::max(0.42F, border_pressure)) {
        return WarObjective::TradeCoercion;
    }
    return WarObjective::BorderDispute;
}

std::string BuildReason(const WarPressure& pressure) {
    std::ostringstream stream;
    stream << ToString(pressure.objective) << ": roi " << pressure.war_roi << " pressure "
           << pressure.declaration_pressure << " trade_weight " << pressure.trade_conflict_weight
           << " friendly_penalty " << pressure.friendly_penalty << " grievance " << pressure.grievance_pressure
           << " restraint " << pressure.restraint_pressure << " vassalage " << pressure.vassalage_pressure;
    return stream.str();
}

WarPressure BuildDirectedPressure(int id, const Polity& actor, const Polity& target,
                                  const DiplomacyRelation& relation) {
    WarPressure pressure;
    pressure.id = id;
    pressure.relation_id = relation.id;
    pressure.actor_polity_id = actor.id;
    pressure.target_polity_id = target.id;

    pressure.military_ratio = SafeRatio(actor.military_potential + 8.0F, target.military_potential + 8.0F);
    pressure.military_advantage = Clamp01((pressure.military_ratio - 0.72F) / 1.18F);
    pressure.border_pressure = Clamp01(relation.border_tension * 0.66F + relation.competition * 0.24F);

    const float actor_dependence = DependenceForActor(relation, actor.id);
    const float target_dependence = DependenceForTarget(relation, target.id);
    const bool actor_is_dependent = actor.id == relation.dependent_polity_id;
    const bool actor_has_leverage = actor.id == relation.leverage_polity_id;
    const float actor_grievance = GrievanceFromActorToTarget(relation, actor.id);
    const float target_grievance = GrievanceFromActorToTarget(relation, target.id);
    const float actor_restraint = RestraintFromActorToTarget(relation, actor.id);
    const float target_restraint = RestraintFromActorToTarget(relation, target.id);
    const float actor_vassalage = VassalageFromActorToTarget(relation, actor.id);
    const float target_vassalage = VassalageFromActorToTarget(relation, target.id);
    const bool actor_is_treaty_subject = ActorIsTreatySubject(relation, actor.id);
    const bool actor_is_treaty_overlord = ActorIsTreatyOverlord(relation, actor.id);
    const float treaty_breakout_pressure =
        actor_is_treaty_subject ? Clamp01(relation.treaty_liberty_desire * 0.82F + actor_grievance * 0.26F) : 0.0F;
    const float overlord_restraint =
        actor_is_treaty_overlord ? Clamp01(relation.treaty_loyalty * 0.52F + relation.treaty_strength * 0.34F) : 0.0F;
    pressure.grievance_pressure = Clamp01(actor_grievance * 0.76F + target_grievance * 0.20F);
    pressure.restraint_pressure =
        Clamp01(actor_restraint * 0.70F + target_restraint * 0.16F + target_vassalage * 0.26F + overlord_restraint);
    pressure.vassalage_pressure = Clamp01(actor_vassalage + treaty_breakout_pressure);
    pressure.dependency_pressure =
        Clamp01(actor_dependence * (actor_is_dependent ? 0.72F : 0.24F) +
                target_dependence * (actor_has_leverage ? 0.68F : 0.18F) + actor_vassalage * 0.58F +
                target_vassalage * 0.16F + treaty_breakout_pressure * 0.58F);
    pressure.blockade_pressure = Clamp01(relation.blockade_tendency * (actor_has_leverage   ? 1.00F
                                                                       : actor_is_dependent ? 0.78F
                                                                                            : 0.55F));
    const float route_fragility = relation.has_active_trade ? 1.0F - relation.trade_route_efficiency : 0.0F;
    pressure.trade_conflict_weight =
        Clamp01(pressure.blockade_pressure * 0.48F + pressure.dependency_pressure * 0.38F + route_fragility * 0.22F +
                relation.competition * 0.18F + pressure.grievance_pressure * 0.26F -
                pressure.restraint_pressure * 0.32F - relation.friendship * 0.18F + treaty_breakout_pressure * 0.22F);
    pressure.friendly_penalty = Clamp01(relation.friendship * (relation.has_active_trade ? 0.88F : 0.62F) +
                                        pressure.restraint_pressure * 0.72F);
    pressure.objective = ChooseObjective(relation, actor.id, pressure.trade_conflict_weight, pressure.border_pressure);

    const float objective_bonus = pressure.objective == WarObjective::Blockade             ? 0.22F
                                  : pressure.objective == WarObjective::DependencyBreakout ? 0.18F
                                                                                           : 0.0F;
    pressure.target_value = Clamp01(pressure.border_pressure * 0.42F + pressure.trade_conflict_weight * 0.58F +
                                    relation.competition * 0.20F + pressure.grievance_pressure * 0.17F +
                                    actor_vassalage * 0.14F + treaty_breakout_pressure * 0.16F + objective_bonus);
    const float defender_strength =
        Clamp01(SafeRatio(target.military_potential, actor.military_potential + target.military_potential + 8.0F));
    pressure.campaign_cost = 0.42F + defender_strength * 0.72F + target.stability * 0.22F +
                             pressure.friendly_penalty * 0.62F + pressure.restraint_pressure * 0.36F +
                             target_vassalage * 0.18F + (actor_is_treaty_overlord ? 0.28F : 0.0F);
    pressure.base_roi = pressure.target_value / std::max(0.10F, pressure.campaign_cost);
    pressure.diplomatic_modifier = std::clamp(
        1.0F + relation.competition * 0.24F + pressure.blockade_pressure * 0.34F +
            pressure.dependency_pressure * 0.26F + pressure.grievance_pressure * 0.30F + actor_vassalage * 0.20F -
            pressure.friendly_penalty * 0.74F - pressure.restraint_pressure * 0.50F - target_vassalage * 0.24F +
            treaty_breakout_pressure * 0.18F - (actor_is_treaty_overlord ? 0.42F : 0.0F),
        0.12F, 1.75F);
    pressure.war_roi = pressure.base_roi * (0.34F + pressure.military_advantage * 0.92F) * pressure.diplomatic_modifier;
    pressure.declaration_pressure = Clamp01(pressure.war_roi / 0.82F);
    pressure.high_pressure = pressure.declaration_pressure >= 0.62F;
    pressure.reason = BuildReason(pressure);
    return pressure;
}

} // namespace

std::vector<WarPressure> BuildWarPressures(const std::vector<Polity>& polities,
                                           const std::vector<DiplomacyRelation>& relations) {
    std::vector<WarPressure> pressures;
    pressures.reserve(relations.size() * 2);
    int next_id = 0;
    for (const auto& relation : relations) {
        const auto* polity_a = PolityById(polities, relation.polity_a_id);
        const auto* polity_b = PolityById(polities, relation.polity_b_id);
        if (polity_a == nullptr || polity_b == nullptr) {
            continue;
        }
        pressures.push_back(BuildDirectedPressure(next_id++, *polity_a, *polity_b, relation));
        pressures.push_back(BuildDirectedPressure(next_id++, *polity_b, *polity_a, relation));
    }
    std::sort(pressures.begin(), pressures.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.declaration_pressure > rhs.declaration_pressure; });
    return pressures;
}

} // namespace oikumene
