#include "sim_balance_metrics.hpp"

#include <algorithm>
#include <fstream>
#include <vector>

namespace oikumene::tools {

nlohmann::json ToJson(const SimBalanceMetrics& metrics) {
    return nlohmann::json{
        {"seed", metrics.seed},
        {"routes_enabled", metrics.routes_enabled},
        {"settlements", metrics.settlements},
        {"villages", metrics.villages},
        {"camps", metrics.camps},
        {"active_bands", metrics.active_bands},
        {"total_population", metrics.total_population},
        {"max_settlement_population", metrics.max_settlement_population},
        {"farm_count", metrics.farms},
        {"lumbercamp_count", metrics.lumber_camps},
        {"pasture_count", metrics.pastures},
        {"shallow_mine_count", metrics.shallow_mines},
        {"worked_tile_count", metrics.worked_tiles},
        {"polities", metrics.polities},
        {"controlled_land_ratio", metrics.controlled_land_ratio},
        {"contested_tiles", metrics.contested_tiles},
        {"largest_polity_population", metrics.largest_polity_population},
        {"average_member_settlements_per_polity", metrics.average_member_settlements_per_polity},
        {"polity_formation_turn_mean", metrics.polity_formation_turn_mean},
        {"average_polity_food_income", metrics.average_polity_food_income},
        {"average_polity_wood_income", metrics.average_polity_wood_income},
        {"average_polity_wealth_income", metrics.average_polity_wealth_income},
        {"average_admin_load", metrics.average_admin_load},
        {"average_admin_capacity", metrics.average_admin_capacity},
        {"average_overextension", metrics.average_overextension},
        {"average_stability", metrics.average_stability},
        {"average_occupation_load", metrics.average_occupation_load},
        {"average_occupation_unrest", metrics.average_occupation_unrest},
        {"average_control_maintenance", metrics.average_control_maintenance},
        {"average_unlocked_techs", metrics.average_unlocked_techs},
        {"average_knowledge_income", metrics.average_knowledge_income},
        {"first_tech_turn_mean", metrics.first_tech_turn_mean},
        {"pottery_unlock_rate", metrics.pottery_unlock_rate},
        {"irrigation_unlock_rate", metrics.irrigation_unlock_rate},
        {"animal_husbandry_unlock_rate", metrics.animal_husbandry_unlock_rate},
        {"mining_unlock_rate", metrics.mining_unlock_rate},
        {"roads_unlock_rate", metrics.roads_unlock_rate},
        {"administration_unlock_rate", metrics.administration_unlock_rate},
        {"bronze_working_unlock_rate", metrics.bronze_working_unlock_rate},
        {"fortification_unlock_rate", metrics.fortification_unlock_rate},
        {"sailing_unlock_rate", metrics.sailing_unlock_rate},
        {"average_ore_income", metrics.average_ore_income},
        {"average_ore_income_for_mining_polities", metrics.average_ore_income_for_mining_polities},
        {"average_tool_efficiency", metrics.average_tool_efficiency},
        {"average_military_potential", metrics.average_military_potential},
        {"routes", metrics.routes},
        {"route_tile_count", metrics.route_tiles},
        {"road_tile_count", metrics.road_tiles},
        {"trail_tile_count", metrics.trail_tiles},
        {"river_route_tile_count", metrics.river_route_tiles},
        {"coastal_route_tile_count", metrics.coastal_route_tiles},
        {"connected_settlements", metrics.connected_settlements},
        {"connected_mines", metrics.connected_mines},
        {"connected_mine_potential", metrics.connected_mine_potential},
        {"active_connected_mines", metrics.active_connected_mines},
        {"average_route_maintenance", metrics.average_route_maintenance},
        {"average_admin_distance_cost", metrics.average_admin_distance_cost},
        {"average_admin_distance_saving", metrics.average_admin_distance_saving},
        {"average_connected_ore_income", metrics.average_connected_ore_income},
        {"average_unconnected_ore_income", metrics.average_unconnected_ore_income},
        {"total_food_output_last_turn", metrics.total_food_output},
        {"total_food_consumption_last_turn", metrics.total_food_consumption},
        {"total_wood_output_last_turn", metrics.total_wood_output},
        {"average_carrying_capacity", metrics.average_carrying_capacity},
        {"food_output_consumption_ratio", metrics.food_output_consumption_ratio},
        {"farm_share_of_worked_tiles", metrics.farm_share_of_worked_tiles},
        {"famine_events", metrics.famine_events},
        {"farm_built_events", metrics.farm_events},
        {"lumbercamp_built_events", metrics.lumber_events},
        {"pasture_built_events", metrics.pasture_events},
        {"route_built_events", metrics.route_events},
        {"trade_opened_events", metrics.trade_events},
        {"active_trades", metrics.active_trades},
        {"diplomacy_relations", metrics.diplomacy_relations},
        {"friendly_relations", metrics.friendly_relations},
        {"competitive_relations", metrics.competitive_relations},
        {"dependent_relations", metrics.dependent_relations},
        {"blockade_risk_relations", metrics.blockade_risk_relations},
        {"war_pressure_candidates", metrics.war_pressure_candidates},
        {"high_war_pressure_candidates", metrics.high_war_pressure_candidates},
        {"war_target_candidates", metrics.war_target_candidates},
        {"high_war_target_candidates", metrics.high_war_target_candidates},
        {"average_trade_profit", metrics.average_trade_profit},
        {"average_trade_complementarity", metrics.average_trade_complementarity},
        {"average_trade_route_cost", metrics.average_trade_route_cost},
        {"average_trade_route_efficiency", metrics.average_trade_route_efficiency},
        {"average_trade_weak_refresh_count", metrics.average_trade_weak_refresh_count},
        {"average_trade_path_tiles", metrics.average_trade_path_tiles},
        {"average_friendship", metrics.average_friendship},
        {"average_competition", metrics.average_competition},
        {"average_blockade_tendency", metrics.average_blockade_tendency},
        {"average_diplomatic_grievance", metrics.average_diplomatic_grievance},
        {"average_diplomatic_vassalage", metrics.average_diplomatic_vassalage},
        {"average_diplomatic_restraint", metrics.average_diplomatic_restraint},
        {"average_war_roi", metrics.average_war_roi},
        {"max_declaration_pressure", metrics.max_declaration_pressure},
        {"average_trade_conflict_weight", metrics.average_trade_conflict_weight},
        {"average_friendly_penalty", metrics.average_friendly_penalty},
        {"average_blockade_pressure", metrics.average_blockade_pressure},
        {"average_dependency_pressure", metrics.average_dependency_pressure},
        {"average_grievance_pressure", metrics.average_grievance_pressure},
        {"average_restraint_pressure", metrics.average_restraint_pressure},
        {"average_vassalage_pressure", metrics.average_vassalage_pressure},
        {"average_war_target_roi", metrics.average_war_target_roi},
        {"max_war_target_score", metrics.max_war_target_score},
        {"average_war_target_value", metrics.average_war_target_value},
        {"average_campaign_cost", metrics.average_campaign_cost},
        {"average_occupation_cost", metrics.average_occupation_cost},
        {"war_campaigns", metrics.war_campaigns},
        {"active_wars", metrics.active_wars},
        {"occupied_wars", metrics.occupied_wars},
        {"withdrawn_wars", metrics.withdrawn_wars},
        {"peace_wars", metrics.peace_wars},
        {"occupations", metrics.occupations},
        {"active_occupations", metrics.active_occupations},
        {"ceded_occupations", metrics.ceded_occupations},
        {"withdrawn_occupations", metrics.withdrawn_occupations},
        {"vassalized_occupations", metrics.vassalized_occupations},
        {"revolted_occupations", metrics.revolted_occupations},
        {"vassal_treaties", metrics.vassal_treaties},
        {"active_vassal_treaties", metrics.active_vassal_treaties},
        {"broken_vassal_treaties", metrics.broken_vassal_treaties},
        {"average_vassal_loyalty", metrics.average_vassal_loyalty},
        {"average_vassal_liberty_desire", metrics.average_vassal_liberty_desire},
        {"total_vassal_tribute_due", metrics.total_vassal_tribute_due},
        {"average_active_occupation_unrest", metrics.average_active_occupation_unrest},
        {"average_active_occupation_maintenance", metrics.average_active_occupation_maintenance},
        {"war_population_lost", metrics.war_population_lost},
        {"war_food_spent", metrics.war_food_spent},
        {"war_equipment_spent", metrics.war_equipment_spent},
        {"average_war_progress", metrics.average_war_progress},
        {"war_declared_events", metrics.war_declared_events},
        {"war_occupied_events", metrics.war_occupied_events},
        {"war_retreat_events", metrics.war_retreat_events},
        {"peace_events", metrics.peace_events},
        {"territory_ceded_events", metrics.territory_ceded_events},
        {"occupation_withdrawn_events", metrics.occupation_withdrawn_events},
        {"vassal_created_events", metrics.vassal_created_events},
        {"occupation_revolt_events", metrics.occupation_revolt_events},
    };
}

void WriteCsvHeader(std::ofstream& output) {
    output
        << "seed,routes_enabled,settlements,villages,camps,active_bands,total_population,max_settlement_population,"
           "farm_count,lumbercamp_count,pasture_count,shallow_mine_count,worked_tile_count,"
           "polities,controlled_land_ratio,contested_tiles,largest_polity_population,"
           "average_member_settlements_per_polity,polity_formation_turn_mean,"
           "average_polity_food_income,average_polity_wood_income,average_polity_wealth_income,"
           "average_admin_load,average_admin_capacity,average_overextension,average_stability,"
           "average_occupation_load,average_occupation_unrest,"
           "average_control_maintenance,average_unlocked_techs,average_knowledge_income,first_tech_turn_mean,"
           "pottery_unlock_rate,irrigation_unlock_rate,animal_husbandry_unlock_rate,mining_unlock_rate,"
           "roads_unlock_rate,administration_unlock_rate,bronze_working_unlock_rate,fortification_unlock_rate,"
           "sailing_unlock_rate,average_ore_income,average_ore_income_for_mining_polities,"
           "average_tool_efficiency,average_military_potential,"
           "routes,route_tile_count,road_tile_count,trail_tile_count,river_route_tile_count,coastal_route_tile_count,"
           "connected_settlements,connected_mines,connected_mine_potential,active_connected_mines,"
           "average_route_maintenance,average_admin_distance_cost,average_admin_distance_saving,"
           "average_connected_ore_income,average_unconnected_ore_income,"
           "total_food_output_last_turn,total_food_consumption_last_turn,total_wood_output_last_turn,"
           "average_carrying_capacity,food_output_consumption_ratio,farm_share_of_worked_tiles,"
           "famine_events,farm_built_events,lumbercamp_built_events,pasture_built_events,route_built_events,"
           "trade_opened_events,active_trades,diplomacy_relations,friendly_relations,competitive_relations,"
           "dependent_relations,blockade_risk_relations,war_pressure_candidates,high_war_pressure_candidates,"
           "war_target_candidates,high_war_target_candidates,"
           "average_trade_profit,average_trade_complementarity,"
           "average_trade_route_cost,average_trade_route_efficiency,average_trade_weak_refresh_count,"
           "average_trade_path_tiles,average_friendship,average_competition,average_blockade_tendency,"
           "average_diplomatic_grievance,average_diplomatic_vassalage,average_diplomatic_restraint,"
           "average_war_roi,max_declaration_pressure,average_trade_conflict_weight,average_friendly_penalty,"
           "average_blockade_pressure,average_dependency_pressure,average_grievance_pressure,"
           "average_restraint_pressure,average_vassalage_pressure,average_war_target_roi,max_war_target_score,"
           "average_war_target_value,average_campaign_cost,average_occupation_cost,"
           "war_campaigns,active_wars,occupied_wars,withdrawn_wars,peace_wars,"
           "occupations,active_occupations,ceded_occupations,withdrawn_occupations,vassalized_occupations,"
           "revolted_occupations,vassal_treaties,active_vassal_treaties,broken_vassal_treaties,"
           "average_vassal_loyalty,average_vassal_liberty_desire,total_vassal_tribute_due,"
           "average_active_occupation_unrest,average_active_occupation_maintenance,"
           "war_population_lost,war_food_spent,war_equipment_spent,average_war_progress,"
           "war_declared_events,war_occupied_events,war_retreat_events,peace_events,"
           "territory_ceded_events,occupation_withdrawn_events,vassal_created_events,occupation_revolt_events\n";
}

void WriteCsvRow(std::ofstream& output, const SimBalanceMetrics& metrics) {
    output << metrics.seed << ',' << (metrics.routes_enabled ? 1 : 0) << ',' << metrics.settlements << ','
           << metrics.villages << ',' << metrics.camps << ',' << metrics.active_bands << ',' << metrics.total_population
           << ',' << metrics.max_settlement_population << ',' << metrics.farms << ',' << metrics.lumber_camps << ','
           << metrics.pastures << ',' << metrics.shallow_mines << ',' << metrics.worked_tiles << ',' << metrics.polities
           << ',' << metrics.controlled_land_ratio << ',' << metrics.contested_tiles << ','
           << metrics.largest_polity_population << ',' << metrics.average_member_settlements_per_polity << ','
           << metrics.polity_formation_turn_mean << ',' << metrics.average_polity_food_income << ','
           << metrics.average_polity_wood_income << ',' << metrics.average_polity_wealth_income << ','
           << metrics.average_admin_load << ',' << metrics.average_admin_capacity << ','
           << metrics.average_overextension << ',' << metrics.average_stability << ','
           << metrics.average_occupation_load << ',' << metrics.average_occupation_unrest << ','
           << metrics.average_control_maintenance << ',' << metrics.average_unlocked_techs << ','
           << metrics.average_knowledge_income << ',' << metrics.first_tech_turn_mean << ','
           << metrics.pottery_unlock_rate << ',' << metrics.irrigation_unlock_rate << ','
           << metrics.animal_husbandry_unlock_rate << ',' << metrics.mining_unlock_rate << ','
           << metrics.roads_unlock_rate << ',' << metrics.administration_unlock_rate << ','
           << metrics.bronze_working_unlock_rate << ',' << metrics.fortification_unlock_rate << ','
           << metrics.sailing_unlock_rate << ',' << metrics.average_ore_income << ','
           << metrics.average_ore_income_for_mining_polities << ',' << metrics.average_tool_efficiency << ','
           << metrics.average_military_potential << ',' << metrics.routes << ',' << metrics.route_tiles << ','
           << metrics.road_tiles << ',' << metrics.trail_tiles << ',' << metrics.river_route_tiles << ','
           << metrics.coastal_route_tiles << ',' << metrics.connected_settlements << ',' << metrics.connected_mines
           << ',' << metrics.connected_mine_potential << ',' << metrics.active_connected_mines << ','
           << metrics.average_route_maintenance << ',' << metrics.average_admin_distance_cost << ','
           << metrics.average_admin_distance_saving << ',' << metrics.average_connected_ore_income << ','
           << metrics.average_unconnected_ore_income << ',' << metrics.total_food_output << ','
           << metrics.total_food_consumption << ',' << metrics.total_wood_output << ','
           << metrics.average_carrying_capacity << ',' << metrics.food_output_consumption_ratio << ','
           << metrics.farm_share_of_worked_tiles << ',' << metrics.famine_events << ',' << metrics.farm_events << ','
           << metrics.lumber_events << ',' << metrics.pasture_events << ',' << metrics.route_events << ','
           << metrics.trade_events << ',' << metrics.active_trades << ',' << metrics.diplomacy_relations << ','
           << metrics.friendly_relations << ',' << metrics.competitive_relations << ',' << metrics.dependent_relations
           << ',' << metrics.blockade_risk_relations << ',' << metrics.war_pressure_candidates << ','
           << metrics.high_war_pressure_candidates << ',' << metrics.war_target_candidates << ','
           << metrics.high_war_target_candidates << ',' << metrics.average_trade_profit << ','
           << metrics.average_trade_complementarity << ',' << metrics.average_trade_route_cost << ','
           << metrics.average_trade_route_efficiency << ',' << metrics.average_trade_weak_refresh_count << ','
           << metrics.average_trade_path_tiles << ',' << metrics.average_friendship << ','
           << metrics.average_competition << ',' << metrics.average_blockade_tendency << ','
           << metrics.average_diplomatic_grievance << ',' << metrics.average_diplomatic_vassalage << ','
           << metrics.average_diplomatic_restraint << ',' << metrics.average_war_roi << ','
           << metrics.max_declaration_pressure << ',' << metrics.average_trade_conflict_weight << ','
           << metrics.average_friendly_penalty << ',' << metrics.average_blockade_pressure << ','
           << metrics.average_dependency_pressure << ',' << metrics.average_grievance_pressure << ','
           << metrics.average_restraint_pressure << ',' << metrics.average_vassalage_pressure << ','
           << metrics.average_war_target_roi << ',' << metrics.max_war_target_score << ','
           << metrics.average_war_target_value << ',' << metrics.average_campaign_cost << ','
           << metrics.average_occupation_cost << ',' << metrics.war_campaigns << ',' << metrics.active_wars << ','
           << metrics.occupied_wars << ',' << metrics.withdrawn_wars << ',' << metrics.peace_wars << ','
           << metrics.occupations << ',' << metrics.active_occupations << ',' << metrics.ceded_occupations << ','
           << metrics.withdrawn_occupations << ',' << metrics.vassalized_occupations << ','
           << metrics.revolted_occupations << ',' << metrics.vassal_treaties << ',' << metrics.active_vassal_treaties
           << ',' << metrics.broken_vassal_treaties << ',' << metrics.average_vassal_loyalty << ','
           << metrics.average_vassal_liberty_desire << ',' << metrics.total_vassal_tribute_due << ','
           << metrics.average_active_occupation_unrest << ',' << metrics.average_active_occupation_maintenance << ','
           << metrics.war_population_lost << ',' << metrics.war_food_spent << ',' << metrics.war_equipment_spent << ','
           << metrics.average_war_progress << ',' << metrics.war_declared_events << ',' << metrics.war_occupied_events
           << ',' << metrics.war_retreat_events << ',' << metrics.peace_events << ',' << metrics.territory_ceded_events
           << ',' << metrics.occupation_withdrawn_events << ',' << metrics.vassal_created_events << ','
           << metrics.occupation_revolt_events << '\n';
}

nlohmann::json Aggregate(const std::vector<SimBalanceMetrics>& metrics) {
    auto mean = [&](auto getter) {
        float total = 0.0F;
        for (const auto& item : metrics) {
            total += static_cast<float>(getter(item));
        }
        return metrics.empty() ? 0.0F : total / static_cast<float>(metrics.size());
    };
    auto min_value = [&](auto getter) {
        if (metrics.empty()) {
            return 0.0F;
        }
        float value = static_cast<float>(getter(metrics.front()));
        for (const auto& item : metrics) {
            value = std::min(value, static_cast<float>(getter(item)));
        }
        return value;
    };
    auto max_value = [&](auto getter) {
        if (metrics.empty()) {
            return 0.0F;
        }
        float value = static_cast<float>(getter(metrics.front()));
        for (const auto& item : metrics) {
            value = std::max(value, static_cast<float>(getter(item)));
        }
        return value;
    };

    return nlohmann::json{
        {"count", metrics.size()},
        {"mean_total_population", mean([](const SimBalanceMetrics& item) { return item.total_population; })},
        {"min_total_population", min_value([](const SimBalanceMetrics& item) { return item.total_population; })},
        {"max_total_population", max_value([](const SimBalanceMetrics& item) { return item.total_population; })},
        {"mean_max_settlement_population",
         mean([](const SimBalanceMetrics& item) { return item.max_settlement_population; })},
        {"mean_settlements", mean([](const SimBalanceMetrics& item) { return item.settlements; })},
        {"mean_villages", mean([](const SimBalanceMetrics& item) { return item.villages; })},
        {"mean_farms", mean([](const SimBalanceMetrics& item) { return item.farms; })},
        {"mean_lumbercamps", mean([](const SimBalanceMetrics& item) { return item.lumber_camps; })},
        {"mean_pastures", mean([](const SimBalanceMetrics& item) { return item.pastures; })},
        {"mean_shallow_mines", mean([](const SimBalanceMetrics& item) { return item.shallow_mines; })},
        {"mean_worked_tiles", mean([](const SimBalanceMetrics& item) { return item.worked_tiles; })},
        {"mean_polities", mean([](const SimBalanceMetrics& item) { return item.polities; })},
        {"mean_controlled_land_ratio", mean([](const SimBalanceMetrics& item) { return item.controlled_land_ratio; })},
        {"mean_contested_tiles", mean([](const SimBalanceMetrics& item) { return item.contested_tiles; })},
        {"mean_largest_polity_population",
         mean([](const SimBalanceMetrics& item) { return item.largest_polity_population; })},
        {"mean_member_settlements_per_polity",
         mean([](const SimBalanceMetrics& item) { return item.average_member_settlements_per_polity; })},
        {"mean_polity_formation_turn",
         mean([](const SimBalanceMetrics& item) { return item.polity_formation_turn_mean; })},
        {"mean_polity_food_income",
         mean([](const SimBalanceMetrics& item) { return item.average_polity_food_income; })},
        {"mean_polity_wood_income",
         mean([](const SimBalanceMetrics& item) { return item.average_polity_wood_income; })},
        {"mean_polity_wealth_income",
         mean([](const SimBalanceMetrics& item) { return item.average_polity_wealth_income; })},
        {"mean_admin_load", mean([](const SimBalanceMetrics& item) { return item.average_admin_load; })},
        {"mean_admin_capacity", mean([](const SimBalanceMetrics& item) { return item.average_admin_capacity; })},
        {"mean_overextension", mean([](const SimBalanceMetrics& item) { return item.average_overextension; })},
        {"mean_stability", mean([](const SimBalanceMetrics& item) { return item.average_stability; })},
        {"mean_occupation_load", mean([](const SimBalanceMetrics& item) { return item.average_occupation_load; })},
        {"mean_occupation_unrest", mean([](const SimBalanceMetrics& item) { return item.average_occupation_unrest; })},
        {"mean_control_maintenance",
         mean([](const SimBalanceMetrics& item) { return item.average_control_maintenance; })},
        {"mean_unlocked_techs", mean([](const SimBalanceMetrics& item) { return item.average_unlocked_techs; })},
        {"mean_knowledge_income", mean([](const SimBalanceMetrics& item) { return item.average_knowledge_income; })},
        {"mean_first_tech_turn", mean([](const SimBalanceMetrics& item) { return item.first_tech_turn_mean; })},
        {"pottery_unlock_rate", mean([](const SimBalanceMetrics& item) { return item.pottery_unlock_rate; })},
        {"irrigation_unlock_rate", mean([](const SimBalanceMetrics& item) { return item.irrigation_unlock_rate; })},
        {"animal_husbandry_unlock_rate",
         mean([](const SimBalanceMetrics& item) { return item.animal_husbandry_unlock_rate; })},
        {"mining_unlock_rate", mean([](const SimBalanceMetrics& item) { return item.mining_unlock_rate; })},
        {"roads_unlock_rate", mean([](const SimBalanceMetrics& item) { return item.roads_unlock_rate; })},
        {"administration_unlock_rate",
         mean([](const SimBalanceMetrics& item) { return item.administration_unlock_rate; })},
        {"bronze_working_unlock_rate",
         mean([](const SimBalanceMetrics& item) { return item.bronze_working_unlock_rate; })},
        {"fortification_unlock_rate",
         mean([](const SimBalanceMetrics& item) { return item.fortification_unlock_rate; })},
        {"sailing_unlock_rate", mean([](const SimBalanceMetrics& item) { return item.sailing_unlock_rate; })},
        {"mean_ore_income", mean([](const SimBalanceMetrics& item) { return item.average_ore_income; })},
        {"mean_ore_income_for_mining_polities",
         mean([](const SimBalanceMetrics& item) { return item.average_ore_income_for_mining_polities; })},
        {"mean_tool_efficiency", mean([](const SimBalanceMetrics& item) { return item.average_tool_efficiency; })},
        {"mean_military_potential",
         mean([](const SimBalanceMetrics& item) { return item.average_military_potential; })},
        {"mean_routes", mean([](const SimBalanceMetrics& item) { return item.routes; })},
        {"mean_active_trades", mean([](const SimBalanceMetrics& item) { return item.active_trades; })},
        {"mean_diplomacy_relations", mean([](const SimBalanceMetrics& item) { return item.diplomacy_relations; })},
        {"mean_friendly_relations", mean([](const SimBalanceMetrics& item) { return item.friendly_relations; })},
        {"mean_competitive_relations", mean([](const SimBalanceMetrics& item) { return item.competitive_relations; })},
        {"mean_dependent_relations", mean([](const SimBalanceMetrics& item) { return item.dependent_relations; })},
        {"mean_blockade_risk_relations",
         mean([](const SimBalanceMetrics& item) { return item.blockade_risk_relations; })},
        {"mean_war_pressure_candidates",
         mean([](const SimBalanceMetrics& item) { return item.war_pressure_candidates; })},
        {"mean_high_war_pressure_candidates",
         mean([](const SimBalanceMetrics& item) { return item.high_war_pressure_candidates; })},
        {"mean_war_target_candidates", mean([](const SimBalanceMetrics& item) { return item.war_target_candidates; })},
        {"mean_high_war_target_candidates",
         mean([](const SimBalanceMetrics& item) { return item.high_war_target_candidates; })},
        {"mean_route_tiles", mean([](const SimBalanceMetrics& item) { return item.route_tiles; })},
        {"mean_road_tiles", mean([](const SimBalanceMetrics& item) { return item.road_tiles; })},
        {"mean_trail_tiles", mean([](const SimBalanceMetrics& item) { return item.trail_tiles; })},
        {"mean_river_route_tiles", mean([](const SimBalanceMetrics& item) { return item.river_route_tiles; })},
        {"mean_coastal_route_tiles", mean([](const SimBalanceMetrics& item) { return item.coastal_route_tiles; })},
        {"mean_connected_settlements", mean([](const SimBalanceMetrics& item) { return item.connected_settlements; })},
        {"mean_connected_mines", mean([](const SimBalanceMetrics& item) { return item.connected_mines; })},
        {"mean_connected_mine_potential",
         mean([](const SimBalanceMetrics& item) { return item.connected_mine_potential; })},
        {"mean_active_connected_mines",
         mean([](const SimBalanceMetrics& item) { return item.active_connected_mines; })},
        {"mean_route_maintenance", mean([](const SimBalanceMetrics& item) { return item.average_route_maintenance; })},
        {"mean_admin_distance_cost",
         mean([](const SimBalanceMetrics& item) { return item.average_admin_distance_cost; })},
        {"mean_admin_distance_saving",
         mean([](const SimBalanceMetrics& item) { return item.average_admin_distance_saving; })},
        {"mean_connected_ore_income",
         mean([](const SimBalanceMetrics& item) { return item.average_connected_ore_income; })},
        {"mean_unconnected_ore_income",
         mean([](const SimBalanceMetrics& item) { return item.average_unconnected_ore_income; })},
        {"mean_trade_profit", mean([](const SimBalanceMetrics& item) { return item.average_trade_profit; })},
        {"mean_trade_complementarity",
         mean([](const SimBalanceMetrics& item) { return item.average_trade_complementarity; })},
        {"mean_trade_route_cost", mean([](const SimBalanceMetrics& item) { return item.average_trade_route_cost; })},
        {"mean_trade_route_efficiency",
         mean([](const SimBalanceMetrics& item) { return item.average_trade_route_efficiency; })},
        {"mean_trade_weak_refresh_count",
         mean([](const SimBalanceMetrics& item) { return item.average_trade_weak_refresh_count; })},
        {"mean_trade_path_tiles", mean([](const SimBalanceMetrics& item) { return item.average_trade_path_tiles; })},
        {"mean_friendship", mean([](const SimBalanceMetrics& item) { return item.average_friendship; })},
        {"mean_competition", mean([](const SimBalanceMetrics& item) { return item.average_competition; })},
        {"mean_blockade_tendency", mean([](const SimBalanceMetrics& item) { return item.average_blockade_tendency; })},
        {"mean_diplomatic_grievance",
         mean([](const SimBalanceMetrics& item) { return item.average_diplomatic_grievance; })},
        {"mean_diplomatic_vassalage",
         mean([](const SimBalanceMetrics& item) { return item.average_diplomatic_vassalage; })},
        {"mean_diplomatic_restraint",
         mean([](const SimBalanceMetrics& item) { return item.average_diplomatic_restraint; })},
        {"mean_war_roi", mean([](const SimBalanceMetrics& item) { return item.average_war_roi; })},
        {"mean_max_declaration_pressure",
         mean([](const SimBalanceMetrics& item) { return item.max_declaration_pressure; })},
        {"mean_trade_conflict_weight",
         mean([](const SimBalanceMetrics& item) { return item.average_trade_conflict_weight; })},
        {"mean_friendly_penalty", mean([](const SimBalanceMetrics& item) { return item.average_friendly_penalty; })},
        {"mean_blockade_pressure", mean([](const SimBalanceMetrics& item) { return item.average_blockade_pressure; })},
        {"mean_dependency_pressure",
         mean([](const SimBalanceMetrics& item) { return item.average_dependency_pressure; })},
        {"mean_grievance_pressure",
         mean([](const SimBalanceMetrics& item) { return item.average_grievance_pressure; })},
        {"mean_restraint_pressure",
         mean([](const SimBalanceMetrics& item) { return item.average_restraint_pressure; })},
        {"mean_vassalage_pressure",
         mean([](const SimBalanceMetrics& item) { return item.average_vassalage_pressure; })},
        {"mean_war_target_roi", mean([](const SimBalanceMetrics& item) { return item.average_war_target_roi; })},
        {"mean_max_war_target_score", mean([](const SimBalanceMetrics& item) { return item.max_war_target_score; })},
        {"mean_war_target_value", mean([](const SimBalanceMetrics& item) { return item.average_war_target_value; })},
        {"mean_campaign_cost", mean([](const SimBalanceMetrics& item) { return item.average_campaign_cost; })},
        {"mean_occupation_cost", mean([](const SimBalanceMetrics& item) { return item.average_occupation_cost; })},
        {"mean_war_campaigns", mean([](const SimBalanceMetrics& item) { return item.war_campaigns; })},
        {"mean_active_wars", mean([](const SimBalanceMetrics& item) { return item.active_wars; })},
        {"mean_occupied_wars", mean([](const SimBalanceMetrics& item) { return item.occupied_wars; })},
        {"mean_withdrawn_wars", mean([](const SimBalanceMetrics& item) { return item.withdrawn_wars; })},
        {"mean_peace_wars", mean([](const SimBalanceMetrics& item) { return item.peace_wars; })},
        {"mean_occupations", mean([](const SimBalanceMetrics& item) { return item.occupations; })},
        {"mean_active_occupations", mean([](const SimBalanceMetrics& item) { return item.active_occupations; })},
        {"mean_ceded_occupations", mean([](const SimBalanceMetrics& item) { return item.ceded_occupations; })},
        {"mean_withdrawn_occupations", mean([](const SimBalanceMetrics& item) { return item.withdrawn_occupations; })},
        {"mean_vassalized_occupations",
         mean([](const SimBalanceMetrics& item) { return item.vassalized_occupations; })},
        {"mean_revolted_occupations", mean([](const SimBalanceMetrics& item) { return item.revolted_occupations; })},
        {"mean_vassal_treaties", mean([](const SimBalanceMetrics& item) { return item.vassal_treaties; })},
        {"mean_active_vassal_treaties",
         mean([](const SimBalanceMetrics& item) { return item.active_vassal_treaties; })},
        {"mean_broken_vassal_treaties",
         mean([](const SimBalanceMetrics& item) { return item.broken_vassal_treaties; })},
        {"mean_vassal_loyalty", mean([](const SimBalanceMetrics& item) { return item.average_vassal_loyalty; })},
        {"mean_vassal_liberty_desire",
         mean([](const SimBalanceMetrics& item) { return item.average_vassal_liberty_desire; })},
        {"mean_vassal_tribute_due", mean([](const SimBalanceMetrics& item) { return item.total_vassal_tribute_due; })},
        {"mean_active_occupation_unrest",
         mean([](const SimBalanceMetrics& item) { return item.average_active_occupation_unrest; })},
        {"mean_active_occupation_maintenance",
         mean([](const SimBalanceMetrics& item) { return item.average_active_occupation_maintenance; })},
        {"mean_war_population_lost", mean([](const SimBalanceMetrics& item) { return item.war_population_lost; })},
        {"mean_war_food_spent", mean([](const SimBalanceMetrics& item) { return item.war_food_spent; })},
        {"mean_war_equipment_spent", mean([](const SimBalanceMetrics& item) { return item.war_equipment_spent; })},
        {"mean_war_progress", mean([](const SimBalanceMetrics& item) { return item.average_war_progress; })},
        {"mean_food_output", mean([](const SimBalanceMetrics& item) { return item.total_food_output; })},
        {"mean_food_consumption", mean([](const SimBalanceMetrics& item) { return item.total_food_consumption; })},
        {"mean_food_output_consumption_ratio",
         mean([](const SimBalanceMetrics& item) { return item.food_output_consumption_ratio; })},
        {"mean_farm_share_of_worked_tiles",
         mean([](const SimBalanceMetrics& item) { return item.farm_share_of_worked_tiles; })},
        {"mean_wood_output", mean([](const SimBalanceMetrics& item) { return item.total_wood_output; })},
        {"mean_carrying_capacity", mean([](const SimBalanceMetrics& item) { return item.average_carrying_capacity; })},
        {"mean_famine_events", mean([](const SimBalanceMetrics& item) { return item.famine_events; })},
        {"mean_farm_built_events", mean([](const SimBalanceMetrics& item) { return item.farm_events; })},
        {"mean_lumbercamp_built_events", mean([](const SimBalanceMetrics& item) { return item.lumber_events; })},
        {"mean_pasture_built_events", mean([](const SimBalanceMetrics& item) { return item.pasture_events; })},
        {"mean_route_built_events", mean([](const SimBalanceMetrics& item) { return item.route_events; })},
        {"mean_trade_opened_events", mean([](const SimBalanceMetrics& item) { return item.trade_events; })},
        {"mean_war_declared_events", mean([](const SimBalanceMetrics& item) { return item.war_declared_events; })},
        {"mean_war_occupied_events", mean([](const SimBalanceMetrics& item) { return item.war_occupied_events; })},
        {"mean_war_retreat_events", mean([](const SimBalanceMetrics& item) { return item.war_retreat_events; })},
        {"mean_peace_events", mean([](const SimBalanceMetrics& item) { return item.peace_events; })},
        {"mean_territory_ceded_events",
         mean([](const SimBalanceMetrics& item) { return item.territory_ceded_events; })},
        {"mean_occupation_withdrawn_events",
         mean([](const SimBalanceMetrics& item) { return item.occupation_withdrawn_events; })},
        {"mean_vassal_created_events", mean([](const SimBalanceMetrics& item) { return item.vassal_created_events; })},
        {"mean_occupation_revolt_events",
         mean([](const SimBalanceMetrics& item) { return item.occupation_revolt_events; })},
    };
}

} // namespace oikumene::tools
