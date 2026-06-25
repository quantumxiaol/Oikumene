#include "strategic_report_actions.hpp"

#include <algorithm>
#include <cmath>

#include "oikumene/sim/technology_system.hpp"
#include "strategic_report_helpers.hpp"

namespace oikumene::strategic_report {

float ResearchBenefitFor(const Simulation& sim, const Polity& polity, TechId tech) {
    const TechId preferred = ChooseResearchFor(sim.GetWorld(), sim.Settlements(), polity);
    float benefit = tech == preferred ? 0.86F : 0.52F;
    if (tech == TechId::Mining && (polity.connected_mine_potential > 0 || polity.unconnected_ore_income > 0.05F)) {
        benefit += 0.14F;
    }
    if (tech == TechId::Administration && polity.overextension > 0.18F) {
        benefit += 0.18F;
    }
    if ((tech == TechId::Pottery || tech == TechId::Irrigation) && polity.budget.food_surplus < 0.2F) {
        benefit += 0.14F;
    }
    if (tech == TechId::Roads && polity.admin_distance_saving < 1.0F && polity.member_settlement_ids.size() >= 2) {
        benefit += 0.10F;
    }
    return Clamp01(benefit);
}

void AddResearchActions(const Simulation& sim, const Polity& polity, std::vector<CandidateAction>& actions) {
    for (const auto& def : AllTechDefs()) {
        if (!CanResearchTech(sim.GetWorld(), sim.Settlements(), polity, def.id)) {
            continue;
        }
        actions.push_back(CandidateAction{
            .action_id = "research_" + ToString(def.id),
            .type = ActionType::ResearchTech,
            .target = "tech:" + ToString(def.id),
            .cost = def.cost,
            .estimated_benefit = ResearchBenefitFor(sim, polity, def.id),
            .risk = polity.overextension > 0.35F ? 0.12 : 0.05,
        });
    }
}

void AddTradeActions(const Simulation& sim, const Polity& polity, std::vector<CandidateAction>& actions) {
    for (const auto& relation : sim.DiplomacyRelations()) {
        if (!InvolvesPolity(relation, polity.id) || relation.has_active_trade || MaxRelationRisk(relation) > 0.72F) {
            continue;
        }
        const PolityId other = OtherPolity(relation, polity.id);
        actions.push_back(CandidateAction{
            .action_id = "open_trade_" + std::to_string(other),
            .type = ActionType::OpenTrade,
            .target = "polity:" + std::to_string(other),
            .cost = std::max(1.0F, relation.trade_route_cost * 0.05F),
            .estimated_benefit = Clamp01(0.34F + relation.friendship * 0.34F +
                                         std::max(relation.dependence_a_on_b, relation.dependence_b_on_a) * 0.12F),
            .risk = Clamp01(relation.competition * 0.26F + relation.border_tension * 0.18F),
        });
    }
}

void AddWarActions(const Simulation& sim, const Polity& polity, std::vector<CandidateAction>& actions) {
    std::vector<const WarTargetCandidate*> targets;
    for (const auto& target : sim.WarTargets()) {
        if (target.actor_polity_id == polity.id && target.high_value) {
            targets.push_back(&target);
        }
    }
    std::sort(targets.begin(), targets.end(),
              [](const auto* lhs, const auto* rhs) { return lhs->action_score > rhs->action_score; });
    for (const auto* target : targets) {
        actions.push_back(CandidateAction{
            .action_id = "prepare_war_target_" + std::to_string(target->id),
            .type = ActionType::PrepareWar,
            .target = "war_target:" + std::to_string(target->id),
            .cost = target->campaign_cost,
            .estimated_benefit = Clamp01(target->action_score),
            .risk = Clamp01(target->occupation_cost * 0.45F + target->campaign_cost * 0.08F),
        });
    }
}

void AddPeaceActions(const Simulation& sim, const Polity& polity, std::vector<CandidateAction>& actions) {
    for (const auto& war : sim.Wars()) {
        if (war.status != WarCampaignStatus::Active ||
            (war.actor_polity_id != polity.id && war.target_polity_id != polity.id)) {
            continue;
        }
        const PolityId other = war.actor_polity_id == polity.id ? war.target_polity_id : war.actor_polity_id;
        actions.push_back(CandidateAction{
            .action_id = "offer_peace_war_" + std::to_string(war.id),
            .type = ActionType::OfferPeace,
            .target = "war:" + std::to_string(war.id) + ":polity:" + std::to_string(other),
            .cost = std::max(1.0F, war.action_score * 2.0F),
            .estimated_benefit = Clamp01(0.30F + war.food_spent * 0.03F + war.population_lost * 0.02F),
            .risk = Clamp01(0.18F + war.progress * 0.20F),
        });
    }
}

std::vector<CandidateAction> BuildCandidateActions(const Simulation& sim, const Polity& polity,
                                                   const StrategicReportOptions& options) {
    std::vector<CandidateAction> actions;
    AddResearchActions(sim, polity, actions);
    AddTradeActions(sim, polity, actions);
    AddWarActions(sim, polity, actions);
    AddPeaceActions(sim, polity, actions);
    std::sort(actions.begin(), actions.end(), [](const CandidateAction& lhs, const CandidateAction& rhs) {
        return lhs.estimated_benefit - lhs.risk > rhs.estimated_benefit - rhs.risk;
    });
    if (static_cast<int>(actions.size()) > options.max_candidate_actions) {
        actions.resize(static_cast<std::size_t>(options.max_candidate_actions));
    }
    return actions;
}

} // namespace oikumene::strategic_report
