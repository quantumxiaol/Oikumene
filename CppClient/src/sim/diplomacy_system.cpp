#include "oikumene/sim/diplomacy_system.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>

namespace oikumene {
namespace {

float Clamp01(float value) {
    return std::clamp(value, 0.0F, 1.0F);
}

bool SameRelation(const DiplomacyRelation& relation, PolityId a, PolityId b) {
    return (relation.polity_a_id == a && relation.polity_b_id == b) ||
           (relation.polity_a_id == b && relation.polity_b_id == a);
}

const DiplomacyRelation* RelationForPair(const std::vector<DiplomacyRelation>& relations, PolityId a, PolityId b) {
    for (const auto& relation : relations) {
        if (SameRelation(relation, a, b)) {
            return &relation;
        }
    }
    return nullptr;
}

DiplomacyRelation* RelationForPair(std::vector<DiplomacyRelation>& relations, PolityId a, PolityId b) {
    for (auto& relation : relations) {
        if (SameRelation(relation, a, b)) {
            return &relation;
        }
    }
    return nullptr;
}

int NextRelationId(const std::vector<DiplomacyRelation>& relations) {
    int next = 0;
    for (const auto& relation : relations) {
        next = std::max(next, relation.id + 1);
    }
    return next;
}

bool SamePair(const TradeAgreement& trade, PolityId a, PolityId b) {
    return (trade.polity_a_id == a && trade.polity_b_id == b) || (trade.polity_a_id == b && trade.polity_b_id == a);
}

const TradeAgreement* ActiveTradeForPair(const std::vector<TradeAgreement>& trades, PolityId a, PolityId b) {
    for (const auto& trade : trades) {
        if (trade.active && SamePair(trade, a, b)) {
            return &trade;
        }
    }
    return nullptr;
}

const VassalTreaty* ActiveTreatyForPair(const std::vector<VassalTreaty>& treaties, PolityId a, PolityId b) {
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

float ControlledBorderTension(const World& world, PolityId a, PolityId b) {
    int border_edges = 0;
    int contested_near_border = 0;
    static constexpr std::array<std::pair<int, int>, 4> kDirs{
        std::pair<int, int>{1, 0},
        std::pair<int, int>{-1, 0},
        std::pair<int, int>{0, 1},
        std::pair<int, int>{0, -1},
    };

    for (const auto& tile : world.Tiles()) {
        if (tile.controller_polity_id != a && tile.controller_polity_id != b) {
            continue;
        }
        for (const auto& [dx, dy] : kDirs) {
            const int nx = tile.x + dx;
            const int ny = tile.y + dy;
            if (!world.InBounds(nx, ny)) {
                continue;
            }
            const auto& neighbor = world.At(nx, ny);
            if ((tile.controller_polity_id == a && neighbor.controller_polity_id == b) ||
                (tile.controller_polity_id == b && neighbor.controller_polity_id == a)) {
                ++border_edges;
                contested_near_border += tile.is_contested || neighbor.is_contested ? 1 : 0;
            }
        }
    }

    return Clamp01(static_cast<float>(border_edges) / 28.0F + static_cast<float>(contested_near_border) / 42.0F);
}

std::array<float, 4> ExportVector(const Polity& polity) {
    return {
        std::max(0.0F, polity.budget.food_surplus),
        std::max(0.0F, polity.budget.wood_surplus),
        std::max(0.0F, polity.budget.ore_income),
        std::max(0.0F, polity.budget.wealth_surplus),
    };
}

float EconomicOverlap(const Polity& a, const Polity& b) {
    const auto va = ExportVector(a);
    const auto vb = ExportVector(b);
    float dot = 0.0F;
    float norm_a = 0.0F;
    float norm_b = 0.0F;
    for (std::size_t i = 0; i < va.size(); ++i) {
        dot += va[i] * vb[i];
        norm_a += va[i] * va[i];
        norm_b += vb[i] * vb[i];
    }
    if (norm_a <= 0.0001F || norm_b <= 0.0001F) {
        return 0.0F;
    }
    return Clamp01(dot / std::sqrt(norm_a * norm_b));
}

float MilitaryRivalry(const Polity& a, const Polity& b) {
    const float stronger = std::max({1.0F, a.military_potential, b.military_potential});
    return Clamp01(1.0F - std::abs(a.military_potential - b.military_potential) / stronger);
}

float TradeIntensity(const TradeAgreement* trade) {
    if (trade == nullptr) {
        return 0.0F;
    }
    return Clamp01((trade->complementarity + std::max(0.0F, trade->expected_profit)) / 4.0F);
}

float DependencyShare(float directional_value, float complementarity) {
    if (complementarity <= 0.0001F) {
        return 0.0F;
    }
    const float intensity = Clamp01(complementarity / 1.8F);
    return Clamp01((directional_value / complementarity) * intensity);
}

void FillTradeFeatures(DiplomacyRelation& relation, const TradeAgreement* trade) {
    if (trade == nullptr) {
        return;
    }
    relation.has_active_trade = true;
    relation.trade_id = trade->id;
    relation.trade_profit = trade->expected_profit;
    relation.trade_complementarity = trade->complementarity;
    relation.trade_route_efficiency = trade->route_efficiency;
    relation.trade_route_cost = trade->route_cost;

    if (trade->polity_a_id == relation.polity_a_id) {
        relation.dependence_a_on_b = DependencyShare(trade->value_b_to_a, trade->complementarity);
        relation.dependence_b_on_a = DependencyShare(trade->value_a_to_b, trade->complementarity);
    } else {
        relation.dependence_a_on_b = DependencyShare(trade->value_a_to_b, trade->complementarity);
        relation.dependence_b_on_a = DependencyShare(trade->value_b_to_a, trade->complementarity);
    }

    if (relation.dependence_a_on_b > relation.dependence_b_on_a + 0.08F) {
        relation.dependent_polity_id = relation.polity_a_id;
        relation.leverage_polity_id = relation.polity_b_id;
    } else if (relation.dependence_b_on_a > relation.dependence_a_on_b + 0.08F) {
        relation.dependent_polity_id = relation.polity_b_id;
        relation.leverage_polity_id = relation.polity_a_id;
    }
}

void PreserveMemory(DiplomacyRelation& relation, const DiplomacyRelation* previous, Turn turn) {
    relation.memory_updated_turn = turn;
    if (previous == nullptr) {
        return;
    }

    const float elapsed = static_cast<float>(std::max<Turn>(0, turn - previous->memory_updated_turn));
    const float grievance_decay = std::pow(0.996F, elapsed);
    const float vassalage_decay = std::pow(0.998F, elapsed);
    const float restraint_decay = std::pow(0.990F, elapsed);
    relation.grievance_a_to_b = previous->grievance_a_to_b * grievance_decay;
    relation.grievance_b_to_a = previous->grievance_b_to_a * grievance_decay;
    relation.vassalage_a_to_b = previous->vassalage_a_to_b * vassalage_decay;
    relation.vassalage_b_to_a = previous->vassalage_b_to_a * vassalage_decay;
    relation.restraint_a_to_b = previous->restraint_a_to_b * restraint_decay;
    relation.restraint_b_to_a = previous->restraint_b_to_a * restraint_decay;
    relation.last_incident_turn = previous->last_incident_turn;
    relation.last_incident = previous->last_incident;
    relation.incident_count = previous->incident_count;
}

float MaxGrievance(const DiplomacyRelation& relation) {
    return std::max(relation.grievance_a_to_b, relation.grievance_b_to_a);
}

float MaxRestraint(const DiplomacyRelation& relation) {
    return std::max(relation.restraint_a_to_b, relation.restraint_b_to_a);
}

void ApplyMemoryDependence(DiplomacyRelation& relation) {
    relation.dependence_a_on_b = std::max(relation.dependence_a_on_b, relation.vassalage_a_to_b);
    relation.dependence_b_on_a = std::max(relation.dependence_b_on_a, relation.vassalage_b_to_a);
    if (relation.vassalage_a_to_b > relation.vassalage_b_to_a + 0.08F && relation.vassalage_a_to_b > 0.20F) {
        relation.dependent_polity_id = relation.polity_a_id;
        relation.leverage_polity_id = relation.polity_b_id;
    } else if (relation.vassalage_b_to_a > relation.vassalage_a_to_b + 0.08F && relation.vassalage_b_to_a > 0.20F) {
        relation.dependent_polity_id = relation.polity_b_id;
        relation.leverage_polity_id = relation.polity_a_id;
    }
}

void ApplyTreatyDependence(DiplomacyRelation& relation, const VassalTreaty* treaty) {
    if (treaty == nullptr) {
        return;
    }

    relation.active_vassal_treaty_id = treaty->id;
    relation.treaty_overlord_polity_id = treaty->overlord_polity_id;
    relation.treaty_subject_polity_id = treaty->subject_polity_id;
    relation.treaty_strength = treaty->strength;
    relation.treaty_loyalty = treaty->loyalty;
    relation.treaty_liberty_desire = treaty->liberty_desire;
    relation.treaty_tribute_rate = treaty->tribute_rate;

    const float treaty_dependence = Clamp01(0.58F + treaty->strength * 0.28F + treaty->tribute_rate * 0.35F);
    if (treaty->subject_polity_id == relation.polity_a_id && treaty->overlord_polity_id == relation.polity_b_id) {
        relation.dependence_a_on_b = std::max(relation.dependence_a_on_b, treaty_dependence);
        relation.dependent_polity_id = relation.polity_a_id;
        relation.leverage_polity_id = relation.polity_b_id;
    } else if (treaty->subject_polity_id == relation.polity_b_id &&
               treaty->overlord_polity_id == relation.polity_a_id) {
        relation.dependence_b_on_a = std::max(relation.dependence_b_on_a, treaty_dependence);
        relation.dependent_polity_id = relation.polity_b_id;
        relation.leverage_polity_id = relation.polity_a_id;
    }
}

DiplomaticPosture ClassifyPosture(const DiplomacyRelation& relation) {
    const float max_dependence = std::max(relation.dependence_a_on_b, relation.dependence_b_on_a);
    const float max_vassalage = std::max(relation.vassalage_a_to_b, relation.vassalage_b_to_a);
    if (relation.active_vassal_treaty_id >= 0 && relation.treaty_liberty_desire < 0.72F) {
        return DiplomaticPosture::Dependent;
    }
    if (relation.blockade_tendency >= 0.58F && max_dependence >= 0.35F) {
        return DiplomaticPosture::BlockadeRisk;
    }
    if (relation.active_vassal_treaty_id >= 0 && relation.treaty_liberty_desire >= 0.72F &&
        relation.blockade_tendency >= 0.46F) {
        return DiplomaticPosture::BlockadeRisk;
    }
    if (max_vassalage >= 0.58F && relation.blockade_tendency < 0.74F) {
        return DiplomaticPosture::Dependent;
    }
    if (max_dependence >= 0.58F && relation.friendship >= relation.competition * 0.75F) {
        return DiplomaticPosture::Dependent;
    }
    if (relation.friendship >= 0.55F && relation.friendship >= relation.competition) {
        return DiplomaticPosture::Friendly;
    }
    if (relation.competition >= 0.45F) {
        return DiplomaticPosture::Competitive;
    }
    return DiplomaticPosture::Neutral;
}

std::string BuildReason(const DiplomacyRelation& relation) {
    std::ostringstream stream;
    stream << ToString(relation.posture) << ": trade " << relation.trade_id << " friendship " << relation.friendship
           << " competition " << relation.competition << " dependence "
           << std::max(relation.dependence_a_on_b, relation.dependence_b_on_a) << " blockade "
           << relation.blockade_tendency << " grievance " << MaxGrievance(relation) << " restraint "
           << MaxRestraint(relation) << " treaty " << relation.active_vassal_treaty_id << " liberty "
           << relation.treaty_liberty_desire << " last_incident " << ToString(relation.last_incident);
    return stream.str();
}

DiplomacyRelation BuildRelation(int id, const World& world, Turn turn, const Polity& polity_a, const Polity& polity_b,
                                const std::vector<TradeAgreement>& trades,
                                const std::vector<VassalTreaty>& vassal_treaties, const DiplomacyRelation* previous) {
    DiplomacyRelation relation;
    relation.id = id;
    relation.polity_a_id = polity_a.id;
    relation.polity_b_id = polity_b.id;
    PreserveMemory(relation, previous, turn);

    const auto* trade = ActiveTradeForPair(trades, polity_a.id, polity_b.id);
    const auto* treaty = ActiveTreatyForPair(vassal_treaties, polity_a.id, polity_b.id);
    FillTradeFeatures(relation, trade);
    ApplyMemoryDependence(relation);
    ApplyTreatyDependence(relation, treaty);

    relation.border_tension = ControlledBorderTension(world, polity_a.id, polity_b.id);
    relation.economic_overlap = EconomicOverlap(polity_a, polity_b);
    const float trade_intensity = TradeIntensity(trade);
    const float duration_bonus =
        trade == nullptr ? 0.0F : std::min(0.20F, static_cast<float>(turn - trade->opened_turn) / 160.0F);
    const float weak_penalty = trade == nullptr ? 0.0F : static_cast<float>(trade->weak_refresh_count) * 0.08F;
    const float route_efficiency = trade == nullptr ? 0.0F : trade->route_efficiency;
    const float military_rivalry = MilitaryRivalry(polity_a, polity_b);
    const float max_grievance = MaxGrievance(relation);
    const float max_restraint = MaxRestraint(relation);

    relation.friendship =
        Clamp01(0.05F + trade_intensity * 0.58F + route_efficiency * 0.22F + duration_bonus -
                relation.border_tension * 0.25F - weak_penalty - max_grievance * 0.22F + max_restraint * 0.08F);
    relation.competition = Clamp01(relation.border_tension * 0.52F + relation.economic_overlap * 0.22F +
                                   military_rivalry * relation.border_tension * 0.20F - trade_intensity * 0.18F -
                                   relation.friendship * 0.08F + max_grievance * 0.34F - max_restraint * 0.14F);

    const float max_dependence = std::max(relation.dependence_a_on_b, relation.dependence_b_on_a);
    const float route_fragility = trade == nullptr ? 0.0F : 1.0F - route_efficiency;
    relation.blockade_tendency = Clamp01(
        max_dependence * 0.42F + relation.competition * 0.36F + route_fragility * 0.24F - relation.friendship * 0.18F +
        weak_penalty + max_grievance * 0.18F - max_restraint * 0.12F + relation.treaty_liberty_desire * 0.16F);
    relation.posture = ClassifyPosture(relation);
    relation.reason = BuildReason(relation);
    return relation;
}

float& GrievanceFromTo(DiplomacyRelation& relation, PolityId from_id, PolityId to_id) {
    return from_id == relation.polity_a_id && to_id == relation.polity_b_id ? relation.grievance_a_to_b
                                                                            : relation.grievance_b_to_a;
}

float& VassalageFromTo(DiplomacyRelation& relation, PolityId from_id, PolityId to_id) {
    return from_id == relation.polity_a_id && to_id == relation.polity_b_id ? relation.vassalage_a_to_b
                                                                            : relation.vassalage_b_to_a;
}

float& RestraintFromTo(DiplomacyRelation& relation, PolityId from_id, PolityId to_id) {
    return from_id == relation.polity_a_id && to_id == relation.polity_b_id ? relation.restraint_a_to_b
                                                                            : relation.restraint_b_to_a;
}

void AddDirectedMemory(DiplomacyRelation& relation, PolityId from_id, PolityId to_id, float grievance, float vassalage,
                       float restraint) {
    auto& grievance_value = GrievanceFromTo(relation, from_id, to_id);
    auto& vassalage_value = VassalageFromTo(relation, from_id, to_id);
    auto& restraint_value = RestraintFromTo(relation, from_id, to_id);
    grievance_value = Clamp01(grievance_value + grievance);
    vassalage_value = Clamp01(vassalage_value + vassalage);
    restraint_value = Clamp01(restraint_value + restraint);
}

} // namespace

void DiplomacySystem::Reset(std::vector<DiplomacyRelation>& relations) {
    relations.clear();
}

void DiplomacySystem::UpdateDiplomacy(const World& world, Turn turn, const std::vector<Polity>& polities,
                                      const std::vector<TradeAgreement>& trades,
                                      const std::vector<VassalTreaty>& vassal_treaties,
                                      std::vector<DiplomacyRelation>& relations) {
    const std::vector<DiplomacyRelation> previous_relations = relations;
    relations.clear();
    int next_id = 0;
    for (std::size_t i = 0; i < polities.size(); ++i) {
        for (std::size_t j = i + 1; j < polities.size(); ++j) {
            const auto* previous = RelationForPair(previous_relations, polities[i].id, polities[j].id);
            relations.push_back(
                BuildRelation(next_id++, world, turn, polities[i], polities[j], trades, vassal_treaties, previous));
        }
    }
}

void DiplomacySystem::RecordIncident(std::vector<DiplomacyRelation>& relations, Turn turn, PolityId actor_id,
                                     PolityId target_id, DiplomaticIncidentKind kind, float strength) {
    if (actor_id == kInvalidPolityId || target_id == kInvalidPolityId || actor_id == target_id ||
        kind == DiplomaticIncidentKind::None) {
        return;
    }

    auto* relation = RelationForPair(relations, actor_id, target_id);
    if (relation == nullptr) {
        DiplomacyRelation created;
        created.id = NextRelationId(relations);
        created.polity_a_id = std::min(actor_id, target_id);
        created.polity_b_id = std::max(actor_id, target_id);
        created.memory_updated_turn = turn;
        relations.push_back(created);
        relation = &relations.back();
    }

    strength = Clamp01(strength);
    const float scaled = 0.55F + strength * 0.45F;
    switch (kind) {
    case DiplomaticIncidentKind::TerritoryCeded:
        AddDirectedMemory(*relation, target_id, actor_id, 0.58F * scaled, 0.0F, 0.08F * scaled);
        AddDirectedMemory(*relation, actor_id, target_id, 0.10F * scaled, 0.0F, 0.14F * scaled);
        break;
    case DiplomaticIncidentKind::OccupationWithdrawn:
        AddDirectedMemory(*relation, target_id, actor_id, 0.28F * scaled, 0.0F, 0.28F * scaled);
        AddDirectedMemory(*relation, actor_id, target_id, 0.08F * scaled, 0.0F, 0.62F * scaled);
        break;
    case DiplomaticIncidentKind::VassalCreated:
        AddDirectedMemory(*relation, target_id, actor_id, 0.22F * scaled, 0.86F * scaled, 0.12F * scaled);
        AddDirectedMemory(*relation, actor_id, target_id, 0.06F * scaled, 0.0F, 0.56F * scaled);
        break;
    case DiplomaticIncidentKind::OccupationRevolt:
        VassalageFromTo(*relation, target_id, actor_id) *= 0.18F;
        VassalageFromTo(*relation, actor_id, target_id) *= 0.18F;
        AddDirectedMemory(*relation, actor_id, target_id, 0.62F * scaled, 0.0F, 0.04F * scaled);
        AddDirectedMemory(*relation, target_id, actor_id, 0.46F * scaled, 0.0F, 0.06F * scaled);
        break;
    case DiplomaticIncidentKind::None:
        break;
    }

    relation->last_incident = kind;
    relation->last_incident_turn = turn;
    relation->memory_updated_turn = turn;
    ++relation->incident_count;
}

} // namespace oikumene
