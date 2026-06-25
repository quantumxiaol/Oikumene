#include "oikumene/io/simulation_json.hpp"

#include "oikumene/ai/strategic_report.hpp"
#include "oikumene/sim/simulation_metrics.hpp"
#include "oikumene/sim/technology.hpp"
#include "oikumene/world/improvement.hpp"
#include "oikumene/world/resource.hpp"
#include "oikumene/world/route_kind.hpp"

namespace oikumene {

nlohmann::json StockpileToJson(const Stockpile& stockpile) {
    return nlohmann::json{
        {"food", stockpile.food},   {"wood", stockpile.wood},     {"ore", stockpile.ore},
        {"metal", stockpile.metal}, {"wealth", stockpile.wealth},
    };
}

nlohmann::json ToJson(const SimEvent& event) {
    return nlohmann::json{
        {"turn", event.turn}, {"type", ToString(event.type)}, {"actor_id", event.actor_id}, {"x", event.x},
        {"y", event.y},       {"summary", event.summary},
    };
}

nlohmann::json BandToJson(const Band& band) {
    return nlohmann::json{
        {"id", band.id},
        {"x", band.x},
        {"y", band.y},
        {"population", band.population},
        {"food", band.food},
        {"target_x", band.target_x},
        {"target_y", band.target_y},
        {"turns_alive", band.turns_alive},
        {"state", ToString(band.state)},
        {"active", band.active},
        {"last_decision_reason", band.last_decision_reason},
        {"current_tile_score", band.current_tile_score},
        {"best_seen_score", band.best_seen_score},
        {"forage_yield_last_turn", band.forage_yield_last_turn},
    };
}

nlohmann::json SettlementToJson(const Settlement& settlement) {
    nlohmann::json worked_tiles = nlohmann::json::array();
    for (const int tile_index : settlement.worked_tile_indices) {
        worked_tiles.push_back(tile_index);
    }

    return nlohmann::json{
        {"id", settlement.id},
        {"x", settlement.x},
        {"y", settlement.y},
        {"founder_band_id", settlement.founder_band_id},
        {"polity_id", settlement.polity_id},
        {"is_capital", settlement.is_capital},
        {"population", settlement.population},
        {"level", ToString(settlement.level)},
        {"stockpile", StockpileToJson(settlement.stockpile)},
        {"turns_since_founded", settlement.turns_since_founded},
        {"local_food_output_last_turn", settlement.local_food_output_last_turn},
        {"local_wood_output_last_turn", settlement.local_wood_output_last_turn},
        {"food_consumption_last_turn", settlement.food_consumption_last_turn},
        {"upgrade_readiness", settlement.upgrade_readiness},
        {"work_radius", settlement.work_radius},
        {"worked_tile_indices", worked_tiles},
        {"worked_tile_count", settlement.worked_tile_count},
        {"ore_output_last_turn", settlement.ore_output_last_turn},
        {"carrying_capacity", settlement.carrying_capacity},
        {"carrying_capacity_ratio", settlement.carrying_capacity_ratio},
    };
}

nlohmann::json PolityBudgetToJson(const PolityBudget& budget) {
    return nlohmann::json{
        {"food_income", budget.food_income},
        {"wood_income", budget.wood_income},
        {"ore_income", budget.ore_income},
        {"wealth_income", budget.wealth_income},
        {"food_maintenance", budget.food_maintenance},
        {"wood_maintenance", budget.wood_maintenance},
        {"admin_maintenance", budget.admin_maintenance},
        {"control_maintenance", budget.control_maintenance},
        {"food_surplus", budget.food_surplus},
        {"wood_surplus", budget.wood_surplus},
        {"wealth_surplus", budget.wealth_surplus},
    };
}

nlohmann::json TechListToJson(const std::vector<TechId>& techs) {
    nlohmann::json values = nlohmann::json::array();
    for (const auto tech : techs) {
        values.push_back(ToString(tech));
    }
    return values;
}

nlohmann::json TechEffectsToJson(const TechEffects& effects) {
    return nlohmann::json{
        {"farm_output_multiplier", effects.farm_output_multiplier},
        {"river_farm_output_multiplier", effects.river_farm_output_multiplier},
        {"pasture_output_multiplier", effects.pasture_output_multiplier},
        {"horse_value_multiplier", effects.horse_value_multiplier},
        {"food_storage_multiplier", effects.food_storage_multiplier},
        {"famine_severity_multiplier", effects.famine_severity_multiplier},
        {"carrying_capacity_multiplier", effects.carrying_capacity_multiplier},
        {"admin_capacity_multiplier", effects.admin_capacity_multiplier},
        {"distance_admin_load_multiplier", effects.distance_admin_load_multiplier},
        {"overextension_penalty_multiplier", effects.overextension_penalty_multiplier},
        {"control_path_cost_multiplier", effects.control_path_cost_multiplier},
        {"coastal_control_cost_multiplier", effects.coastal_control_cost_multiplier},
        {"contested_stability_loss_multiplier", effects.contested_stability_loss_multiplier},
        {"mining_enabled", effects.mining_enabled},
        {"roads_enabled", effects.roads_enabled},
        {"coastal_trade_enabled", effects.coastal_trade_enabled},
        {"bronze_working_enabled", effects.bronze_working_enabled},
        {"fortification_enabled", effects.fortification_enabled},
    };
}

nlohmann::json ResearchToJson(const Polity& polity) {
    return nlohmann::json{
        {"current", ToString(polity.research.current)},
        {"progress", polity.research.progress},
        {"current_cost", TechCost(polity.research.current)},
        {"knowledge_income", polity.knowledge_income},
        {"first_unlock_turn", polity.research.first_unlock_turn},
        {"unlocked", TechListToJson(polity.research.unlocked)},
    };
}

nlohmann::json PolityToJson(const Polity& polity) {
    const auto effects = ComputeTechEffects(polity.research);
    return nlohmann::json{
        {"id", polity.id},
        {"name", polity.name},
        {"level", ToString(polity.level)},
        {"capital_settlement_id", polity.capital_settlement_id},
        {"member_settlement_ids", polity.member_settlement_ids},
        {"population", polity.population},
        {"food", polity.food},
        {"wood", polity.wood},
        {"ore", polity.ore},
        {"wealth", polity.wealth},
        {"control_power", polity.control_power},
        {"admin_range", polity.admin_range},
        {"legitimacy", polity.legitimacy},
        {"stability", polity.stability},
        {"admin_load", polity.admin_load},
        {"admin_capacity", polity.admin_capacity},
        {"overextension", polity.overextension},
        {"controlled_tile_count", polity.controlled_tile_count},
        {"contested_tile_count", polity.contested_tile_count},
        {"founded_turn", polity.founded_turn},
        {"budget", PolityBudgetToJson(polity.budget)},
        {"research", ResearchToJson(polity)},
        {"current_research", ToString(polity.research.current)},
        {"research_progress", polity.research.progress},
        {"knowledge_income", polity.knowledge_income},
        {"unlocked_techs", TechListToJson(polity.research.unlocked)},
        {"active_effects", TechEffectsToJson(effects)},
        {"military_potential", polity.military_potential},
        {"tool_efficiency", polity.tool_efficiency},
        {"route_ids", polity.route_ids},
        {"route_maintenance", polity.route_maintenance},
        {"connected_settlements", polity.connected_settlements},
        {"connected_mines", polity.connected_mines},
        {"connected_mine_potential", polity.connected_mine_potential},
        {"active_connected_mines", polity.active_connected_mines},
        {"connected_ore_income", polity.connected_ore_income},
        {"unconnected_ore_income", polity.unconnected_ore_income},
        {"admin_distance_saving", polity.admin_distance_saving},
        {"trade_ids", polity.trade_ids},
        {"active_trade_count", polity.active_trade_count},
        {"trade_income", polity.trade_income},
        {"trade_expense", polity.trade_expense},
        {"trade_profit", polity.trade_profit},
        {"trade_complementarity", polity.trade_complementarity},
        {"trade_route_cost", polity.trade_route_cost},
        {"trade_route_efficiency", polity.trade_route_efficiency},
        {"occupation_load", polity.occupation_load},
        {"occupation_unrest", polity.occupation_unrest},
        {"occupied_settlements", polity.occupied_settlements},
        {"overlord_polity_id", polity.overlord_polity_id},
        {"active_overlord_treaty_id", polity.active_overlord_treaty_id},
        {"subject_treaty_ids", polity.subject_treaty_ids},
        {"vassal_count", polity.vassal_count},
        {"vassal_tribute_income", polity.vassal_tribute_income},
        {"vassal_tribute_paid", polity.vassal_tribute_paid},
        {"vassal_liberty_desire", polity.vassal_liberty_desire},
        {"overlord_protection", polity.overlord_protection},
    };
}

nlohmann::json RouteToJson(const Route& route) {
    nlohmann::json path = nlohmann::json::array();
    for (const auto& coord : route.path) {
        path.push_back(nlohmann::json{{"x", coord.x}, {"y", coord.y}});
    }
    return nlohmann::json{
        {"id", route.id},
        {"polity_id", route.polity_id},
        {"from_settlement_id", route.from_settlement_id},
        {"to_settlement_id", route.to_settlement_id},
        {"target_x", route.target_x},
        {"target_y", route.target_y},
        {"purpose", ToString(route.purpose)},
        {"kind", ToString(route.kind)},
        {"path", path},
        {"tile_count", route.path.size()},
        {"build_cost_wood", route.build_cost_wood},
        {"build_cost_wealth", route.build_cost_wealth},
        {"maintenance", route.maintenance},
        {"route_value", route.route_value},
        {"roi", route.roi},
        {"reason", route.reason},
    };
}

nlohmann::json TradeToJson(const TradeAgreement& trade) {
    nlohmann::json path = nlohmann::json::array();
    for (const auto& coord : trade.path) {
        path.push_back(nlohmann::json{{"x", coord.x}, {"y", coord.y}});
    }
    return nlohmann::json{
        {"id", trade.id},
        {"polity_a_id", trade.polity_a_id},
        {"polity_b_id", trade.polity_b_id},
        {"opened_turn", trade.opened_turn},
        {"last_evaluated_turn", trade.last_evaluated_turn},
        {"active", trade.active},
        {"weak_refresh_count", trade.weak_refresh_count},
        {"export_from_a", ToString(trade.export_from_a)},
        {"export_from_b", ToString(trade.export_from_b)},
        {"value_a_to_b", trade.value_a_to_b},
        {"value_b_to_a", trade.value_b_to_a},
        {"complementarity", trade.complementarity},
        {"route_cost", trade.route_cost},
        {"route_cost_without_network", trade.route_cost_without_network},
        {"route_saving", trade.route_saving},
        {"route_efficiency", trade.route_efficiency},
        {"gross_value", trade.gross_value},
        {"transport_cost", trade.transport_cost},
        {"expected_profit", trade.expected_profit},
        {"path", path},
        {"tile_count", trade.path.size()},
        {"reason", trade.reason},
    };
}

nlohmann::json DiplomacyToJson(const DiplomacyRelation& relation) {
    return nlohmann::json{
        {"id", relation.id},
        {"polity_a_id", relation.polity_a_id},
        {"polity_b_id", relation.polity_b_id},
        {"has_active_trade", relation.has_active_trade},
        {"trade_id", relation.trade_id},
        {"trade_profit", relation.trade_profit},
        {"trade_complementarity", relation.trade_complementarity},
        {"trade_route_efficiency", relation.trade_route_efficiency},
        {"trade_route_cost", relation.trade_route_cost},
        {"border_tension", relation.border_tension},
        {"economic_overlap", relation.economic_overlap},
        {"dependence_a_on_b", relation.dependence_a_on_b},
        {"dependence_b_on_a", relation.dependence_b_on_a},
        {"dependent_polity_id", relation.dependent_polity_id},
        {"leverage_polity_id", relation.leverage_polity_id},
        {"active_vassal_treaty_id", relation.active_vassal_treaty_id},
        {"treaty_overlord_polity_id", relation.treaty_overlord_polity_id},
        {"treaty_subject_polity_id", relation.treaty_subject_polity_id},
        {"treaty_strength", relation.treaty_strength},
        {"treaty_loyalty", relation.treaty_loyalty},
        {"treaty_liberty_desire", relation.treaty_liberty_desire},
        {"treaty_tribute_rate", relation.treaty_tribute_rate},
        {"friendship", relation.friendship},
        {"competition", relation.competition},
        {"blockade_tendency", relation.blockade_tendency},
        {"grievance_a_to_b", relation.grievance_a_to_b},
        {"grievance_b_to_a", relation.grievance_b_to_a},
        {"vassalage_a_to_b", relation.vassalage_a_to_b},
        {"vassalage_b_to_a", relation.vassalage_b_to_a},
        {"restraint_a_to_b", relation.restraint_a_to_b},
        {"restraint_b_to_a", relation.restraint_b_to_a},
        {"memory_updated_turn", relation.memory_updated_turn},
        {"last_incident_turn", relation.last_incident_turn},
        {"last_incident", ToString(relation.last_incident)},
        {"incident_count", relation.incident_count},
        {"posture", ToString(relation.posture)},
        {"reason", relation.reason},
    };
}

nlohmann::json WarPressureToJson(const WarPressure& pressure) {
    return nlohmann::json{
        {"id", pressure.id},
        {"relation_id", pressure.relation_id},
        {"actor_polity_id", pressure.actor_polity_id},
        {"target_polity_id", pressure.target_polity_id},
        {"objective", ToString(pressure.objective)},
        {"military_ratio", pressure.military_ratio},
        {"military_advantage", pressure.military_advantage},
        {"border_pressure", pressure.border_pressure},
        {"trade_conflict_weight", pressure.trade_conflict_weight},
        {"dependency_pressure", pressure.dependency_pressure},
        {"blockade_pressure", pressure.blockade_pressure},
        {"friendly_penalty", pressure.friendly_penalty},
        {"grievance_pressure", pressure.grievance_pressure},
        {"restraint_pressure", pressure.restraint_pressure},
        {"vassalage_pressure", pressure.vassalage_pressure},
        {"target_value", pressure.target_value},
        {"campaign_cost", pressure.campaign_cost},
        {"base_roi", pressure.base_roi},
        {"diplomatic_modifier", pressure.diplomatic_modifier},
        {"war_roi", pressure.war_roi},
        {"declaration_pressure", pressure.declaration_pressure},
        {"high_pressure", pressure.high_pressure},
        {"reason", pressure.reason},
    };
}

nlohmann::json WarTargetToJson(const WarTargetCandidate& target) {
    nlohmann::json path = nlohmann::json::array();
    for (const auto& coord : target.path) {
        path.push_back(nlohmann::json{{"x", coord.x}, {"y", coord.y}});
    }
    return nlohmann::json{
        {"id", target.id},
        {"pressure_id", target.pressure_id},
        {"actor_polity_id", target.actor_polity_id},
        {"target_polity_id", target.target_polity_id},
        {"objective", ToString(target.objective)},
        {"kind", ToString(target.kind)},
        {"x", target.x},
        {"y", target.y},
        {"target_settlement_id", target.target_settlement_id},
        {"target_trade_id", target.target_trade_id},
        {"path", path},
        {"tile_count", target.path.size()},
        {"route_cost", target.route_cost},
        {"farmland_value", target.farmland_value},
        {"resource_value", target.resource_value},
        {"settlement_value", target.settlement_value},
        {"trade_value", target.trade_value},
        {"strategic_value", target.strategic_value},
        {"target_value", target.target_value},
        {"mobilization_cost", target.mobilization_cost},
        {"supply_cost", target.supply_cost},
        {"equipment_cost", target.equipment_cost},
        {"terrain_attrition", target.terrain_attrition},
        {"defender_power", target.defender_power},
        {"defense_cost", target.defense_cost},
        {"occupation_cost", target.occupation_cost},
        {"campaign_cost", target.campaign_cost},
        {"roi", target.roi},
        {"action_score", target.action_score},
        {"high_value", target.high_value},
        {"reason", target.reason},
    };
}

nlohmann::json WarCampaignToJson(const WarCampaign& campaign) {
    nlohmann::json path = nlohmann::json::array();
    for (const auto& coord : campaign.path) {
        path.push_back(nlohmann::json{{"x", coord.x}, {"y", coord.y}});
    }
    return nlohmann::json{
        {"id", campaign.id},
        {"target_candidate_id", campaign.target_candidate_id},
        {"actor_polity_id", campaign.actor_polity_id},
        {"target_polity_id", campaign.target_polity_id},
        {"objective", ToString(campaign.objective)},
        {"target_kind", ToString(campaign.target_kind)},
        {"status", ToString(campaign.status)},
        {"x", campaign.x},
        {"y", campaign.y},
        {"target_settlement_id", campaign.target_settlement_id},
        {"target_trade_id", campaign.target_trade_id},
        {"path", path},
        {"tile_count", campaign.path.size()},
        {"started_turn", campaign.started_turn},
        {"ended_turn", campaign.ended_turn},
        {"last_update_turn", campaign.last_update_turn},
        {"turns_active", campaign.turns_active},
        {"target_value", campaign.target_value},
        {"campaign_cost", campaign.campaign_cost},
        {"occupation_cost", campaign.occupation_cost},
        {"action_score", campaign.action_score},
        {"progress", campaign.progress},
        {"mobilized_manpower", campaign.mobilized_manpower},
        {"attacker_strength", campaign.attacker_strength},
        {"defender_strength", campaign.defender_strength},
        {"population_lost", campaign.population_lost},
        {"food_spent", campaign.food_spent},
        {"equipment_spent", campaign.equipment_spent},
        {"occupation_profit", campaign.occupation_profit},
        {"supply_failures", campaign.supply_failures},
        {"outcome_reason", campaign.outcome_reason},
    };
}

nlohmann::json OccupationToJson(const OccupationRecord& occupation) {
    return nlohmann::json{
        {"id", occupation.id},
        {"source_campaign_id", occupation.source_campaign_id},
        {"occupier_polity_id", occupation.occupier_polity_id},
        {"previous_owner_polity_id", occupation.previous_owner_polity_id},
        {"subject_polity_id", occupation.subject_polity_id},
        {"vassal_treaty_id", occupation.vassal_treaty_id},
        {"target_kind", ToString(occupation.target_kind)},
        {"settlement_id", occupation.settlement_id},
        {"x", occupation.x},
        {"y", occupation.y},
        {"status", ToString(occupation.status)},
        {"started_turn", occupation.started_turn},
        {"ended_turn", occupation.ended_turn},
        {"last_update_turn", occupation.last_update_turn},
        {"turns_held", occupation.turns_held},
        {"occupation_value", occupation.occupation_value},
        {"maintenance_cost", occupation.maintenance_cost},
        {"cumulative_maintenance", occupation.cumulative_maintenance},
        {"cumulative_shortfall", occupation.cumulative_shortfall},
        {"unrest", occupation.unrest},
        {"integration", occupation.integration},
        {"revolt_risk", occupation.revolt_risk},
        {"border_stability_delta", occupation.border_stability_delta},
        {"outcome_reason", occupation.outcome_reason},
    };
}

nlohmann::json VassalTreatyToJson(const VassalTreaty& treaty) {
    return nlohmann::json{
        {"id", treaty.id},
        {"source_occupation_id", treaty.source_occupation_id},
        {"overlord_polity_id", treaty.overlord_polity_id},
        {"subject_polity_id", treaty.subject_polity_id},
        {"status", ToString(treaty.status)},
        {"started_turn", treaty.started_turn},
        {"last_update_turn", treaty.last_update_turn},
        {"ended_turn", treaty.ended_turn},
        {"strength", treaty.strength},
        {"autonomy", treaty.autonomy},
        {"tribute_rate", treaty.tribute_rate},
        {"protection", treaty.protection},
        {"loyalty", treaty.loyalty},
        {"liberty_desire", treaty.liberty_desire},
        {"tribute_due", treaty.tribute_due},
        {"military_obligation", treaty.military_obligation},
        {"reason", treaty.reason},
    };
}

} // namespace oikumene
