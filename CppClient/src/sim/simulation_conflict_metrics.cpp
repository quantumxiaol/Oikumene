#include "oikumene/sim/simulation_metrics.hpp"

#include <algorithm>

namespace oikumene {

int CountDiplomacyPosture(const Simulation& sim, DiplomaticPosture posture) {
    int count = 0;
    for (const auto& relation : sim.DiplomacyRelations()) {
        count += relation.posture == posture ? 1 : 0;
    }
    return count;
}

float AverageFriendship(const Simulation& sim) {
    if (sim.DiplomacyRelations().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& relation : sim.DiplomacyRelations()) {
        total += relation.friendship;
    }
    return total / static_cast<float>(sim.DiplomacyRelations().size());
}

float AverageCompetition(const Simulation& sim) {
    if (sim.DiplomacyRelations().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& relation : sim.DiplomacyRelations()) {
        total += relation.competition;
    }
    return total / static_cast<float>(sim.DiplomacyRelations().size());
}

float AverageBlockadeTendency(const Simulation& sim) {
    if (sim.DiplomacyRelations().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& relation : sim.DiplomacyRelations()) {
        total += relation.blockade_tendency;
    }
    return total / static_cast<float>(sim.DiplomacyRelations().size());
}

float AverageDiplomaticGrievance(const Simulation& sim) {
    if (sim.DiplomacyRelations().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& relation : sim.DiplomacyRelations()) {
        total += std::max(relation.grievance_a_to_b, relation.grievance_b_to_a);
    }
    return total / static_cast<float>(sim.DiplomacyRelations().size());
}

float AverageDiplomaticVassalage(const Simulation& sim) {
    if (sim.DiplomacyRelations().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& relation : sim.DiplomacyRelations()) {
        total += std::max(relation.vassalage_a_to_b, relation.vassalage_b_to_a);
    }
    return total / static_cast<float>(sim.DiplomacyRelations().size());
}

float AverageDiplomaticRestraint(const Simulation& sim) {
    if (sim.DiplomacyRelations().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& relation : sim.DiplomacyRelations()) {
        total += std::max(relation.restraint_a_to_b, relation.restraint_b_to_a);
    }
    return total / static_cast<float>(sim.DiplomacyRelations().size());
}

int CountHighWarPressures(const Simulation& sim) {
    int count = 0;
    for (const auto& pressure : sim.WarPressures()) {
        count += pressure.high_pressure ? 1 : 0;
    }
    return count;
}

float AverageWarRoi(const Simulation& sim) {
    if (sim.WarPressures().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& pressure : sim.WarPressures()) {
        total += pressure.war_roi;
    }
    return total / static_cast<float>(sim.WarPressures().size());
}

float MaxDeclarationPressure(const Simulation& sim) {
    float maximum = 0.0F;
    for (const auto& pressure : sim.WarPressures()) {
        maximum = std::max(maximum, pressure.declaration_pressure);
    }
    return maximum;
}

float AverageTradeConflictWeight(const Simulation& sim) {
    if (sim.WarPressures().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& pressure : sim.WarPressures()) {
        total += pressure.trade_conflict_weight;
    }
    return total / static_cast<float>(sim.WarPressures().size());
}

float AverageFriendlyPenalty(const Simulation& sim) {
    if (sim.WarPressures().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& pressure : sim.WarPressures()) {
        total += pressure.friendly_penalty;
    }
    return total / static_cast<float>(sim.WarPressures().size());
}

float AverageBlockadePressure(const Simulation& sim) {
    if (sim.WarPressures().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& pressure : sim.WarPressures()) {
        total += pressure.blockade_pressure;
    }
    return total / static_cast<float>(sim.WarPressures().size());
}

float AverageDependencyPressure(const Simulation& sim) {
    if (sim.WarPressures().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& pressure : sim.WarPressures()) {
        total += pressure.dependency_pressure;
    }
    return total / static_cast<float>(sim.WarPressures().size());
}

float AverageGrievancePressure(const Simulation& sim) {
    if (sim.WarPressures().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& pressure : sim.WarPressures()) {
        total += pressure.grievance_pressure;
    }
    return total / static_cast<float>(sim.WarPressures().size());
}

float AverageRestraintPressure(const Simulation& sim) {
    if (sim.WarPressures().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& pressure : sim.WarPressures()) {
        total += pressure.restraint_pressure;
    }
    return total / static_cast<float>(sim.WarPressures().size());
}

float AverageVassalagePressure(const Simulation& sim) {
    if (sim.WarPressures().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& pressure : sim.WarPressures()) {
        total += pressure.vassalage_pressure;
    }
    return total / static_cast<float>(sim.WarPressures().size());
}

int CountHighWarTargets(const Simulation& sim) {
    int count = 0;
    for (const auto& target : sim.WarTargets()) {
        count += target.high_value ? 1 : 0;
    }
    return count;
}

float AverageWarTargetRoi(const Simulation& sim) {
    if (sim.WarTargets().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& target : sim.WarTargets()) {
        total += target.roi;
    }
    return total / static_cast<float>(sim.WarTargets().size());
}

float MaxWarTargetScore(const Simulation& sim) {
    float maximum = 0.0F;
    for (const auto& target : sim.WarTargets()) {
        maximum = std::max(maximum, target.action_score);
    }
    return maximum;
}

float AverageWarTargetValue(const Simulation& sim) {
    if (sim.WarTargets().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& target : sim.WarTargets()) {
        total += target.target_value;
    }
    return total / static_cast<float>(sim.WarTargets().size());
}

float AverageCampaignCost(const Simulation& sim) {
    if (sim.WarTargets().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& target : sim.WarTargets()) {
        total += target.campaign_cost;
    }
    return total / static_cast<float>(sim.WarTargets().size());
}

float AverageOccupationCost(const Simulation& sim) {
    if (sim.WarTargets().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& target : sim.WarTargets()) {
        total += target.occupation_cost;
    }
    return total / static_cast<float>(sim.WarTargets().size());
}

int CountWarsByStatus(const Simulation& sim, WarCampaignStatus status) {
    int count = 0;
    for (const auto& campaign : sim.Wars()) {
        count += campaign.status == status ? 1 : 0;
    }
    return count;
}

int CountOccupationsByStatus(const Simulation& sim, OccupationStatus status) {
    int count = 0;
    for (const auto& occupation : sim.Occupations()) {
        count += occupation.status == status ? 1 : 0;
    }
    return count;
}

float AverageActiveOccupationUnrest(const Simulation& sim) {
    float total = 0.0F;
    int count = 0;
    for (const auto& occupation : sim.Occupations()) {
        if (occupation.status != OccupationStatus::Active) {
            continue;
        }
        total += occupation.unrest;
        ++count;
    }
    return count <= 0 ? 0.0F : total / static_cast<float>(count);
}

float AverageActiveOccupationMaintenance(const Simulation& sim) {
    float total = 0.0F;
    int count = 0;
    for (const auto& occupation : sim.Occupations()) {
        if (occupation.status != OccupationStatus::Active) {
            continue;
        }
        total += occupation.maintenance_cost;
        ++count;
    }
    return count <= 0 ? 0.0F : total / static_cast<float>(count);
}

int CountVassalTreatiesByStatus(const Simulation& sim, VassalTreatyStatus status) {
    int count = 0;
    for (const auto& treaty : sim.VassalTreaties()) {
        count += treaty.status == status ? 1 : 0;
    }
    return count;
}

float AverageActiveVassalLoyalty(const Simulation& sim) {
    float total = 0.0F;
    int count = 0;
    for (const auto& treaty : sim.VassalTreaties()) {
        if (treaty.status != VassalTreatyStatus::Active) {
            continue;
        }
        total += treaty.loyalty;
        ++count;
    }
    return count <= 0 ? 0.0F : total / static_cast<float>(count);
}

float AverageActiveVassalLibertyDesire(const Simulation& sim) {
    float total = 0.0F;
    int count = 0;
    for (const auto& treaty : sim.VassalTreaties()) {
        if (treaty.status != VassalTreatyStatus::Active) {
            continue;
        }
        total += treaty.liberty_desire;
        ++count;
    }
    return count <= 0 ? 0.0F : total / static_cast<float>(count);
}

float TotalActiveVassalTributeDue(const Simulation& sim) {
    float total = 0.0F;
    for (const auto& treaty : sim.VassalTreaties()) {
        if (treaty.status == VassalTreatyStatus::Active) {
            total += treaty.tribute_due;
        }
    }
    return total;
}

float TotalWarPopulationLost(const Simulation& sim) {
    float total = 0.0F;
    for (const auto& campaign : sim.Wars()) {
        total += campaign.population_lost;
    }
    return total;
}

float TotalWarFoodSpent(const Simulation& sim) {
    float total = 0.0F;
    for (const auto& campaign : sim.Wars()) {
        total += campaign.food_spent;
    }
    return total;
}

float TotalWarEquipmentSpent(const Simulation& sim) {
    float total = 0.0F;
    for (const auto& campaign : sim.Wars()) {
        total += campaign.equipment_spent;
    }
    return total;
}

float AverageWarProgress(const Simulation& sim) {
    if (sim.Wars().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& campaign : sim.Wars()) {
        total += campaign.progress;
    }
    return total / static_cast<float>(sim.Wars().size());
}

} // namespace oikumene
