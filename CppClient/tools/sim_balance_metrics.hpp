#pragma once

#include <cstdint>
#include <iosfwd>
#include <vector>

#include <nlohmann/json.hpp>

namespace oikumene::tools {

struct SimBalanceRunConfig {
    int width = 80;
    int height = 56;
    int bands = 8;
    int turns = 200;
    bool enable_routes = true;
};

struct SimBalanceMetrics {
    std::uint64_t seed = 0;
    bool routes_enabled = true;
    int settlements = 0;
    int villages = 0;
    int camps = 0;
    int active_bands = 0;
    int total_population = 0;
    int max_settlement_population = 0;
    int farms = 0;
    int lumber_camps = 0;
    int pastures = 0;
    int shallow_mines = 0;
    int worked_tiles = 0;
    int polities = 0;
    int contested_tiles = 0;
    int largest_polity_population = 0;
    int famine_events = 0;
    int farm_events = 0;
    int lumber_events = 0;
    int pasture_events = 0;
    int route_events = 0;
    int trade_events = 0;
    int war_declared_events = 0;
    int war_occupied_events = 0;
    int war_retreat_events = 0;
    int peace_events = 0;
    int territory_ceded_events = 0;
    int occupation_withdrawn_events = 0;
    int vassal_created_events = 0;
    int occupation_revolt_events = 0;
    int routes = 0;
    int active_trades = 0;
    int diplomacy_relations = 0;
    int friendly_relations = 0;
    int competitive_relations = 0;
    int dependent_relations = 0;
    int blockade_risk_relations = 0;
    int war_pressure_candidates = 0;
    int high_war_pressure_candidates = 0;
    int war_target_candidates = 0;
    int high_war_target_candidates = 0;
    int war_campaigns = 0;
    int active_wars = 0;
    int occupied_wars = 0;
    int withdrawn_wars = 0;
    int peace_wars = 0;
    int occupations = 0;
    int active_occupations = 0;
    int ceded_occupations = 0;
    int withdrawn_occupations = 0;
    int vassalized_occupations = 0;
    int revolted_occupations = 0;
    int vassal_treaties = 0;
    int active_vassal_treaties = 0;
    int broken_vassal_treaties = 0;
    int route_tiles = 0;
    int road_tiles = 0;
    int trail_tiles = 0;
    int river_route_tiles = 0;
    int coastal_route_tiles = 0;
    int connected_settlements = 0;
    int connected_mines = 0;
    int connected_mine_potential = 0;
    int active_connected_mines = 0;
    float total_food_output = 0.0F;
    float total_food_consumption = 0.0F;
    float total_wood_output = 0.0F;
    float average_carrying_capacity = 0.0F;
    float food_output_consumption_ratio = 0.0F;
    float farm_share_of_worked_tiles = 0.0F;
    float controlled_land_ratio = 0.0F;
    float average_member_settlements_per_polity = 0.0F;
    float polity_formation_turn_mean = 0.0F;
    float average_polity_food_income = 0.0F;
    float average_polity_wood_income = 0.0F;
    float average_polity_wealth_income = 0.0F;
    float average_admin_load = 0.0F;
    float average_admin_capacity = 0.0F;
    float average_overextension = 0.0F;
    float average_stability = 0.0F;
    float average_control_maintenance = 0.0F;
    float average_unlocked_techs = 0.0F;
    float average_knowledge_income = 0.0F;
    float first_tech_turn_mean = 0.0F;
    float pottery_unlock_rate = 0.0F;
    float irrigation_unlock_rate = 0.0F;
    float animal_husbandry_unlock_rate = 0.0F;
    float mining_unlock_rate = 0.0F;
    float roads_unlock_rate = 0.0F;
    float administration_unlock_rate = 0.0F;
    float bronze_working_unlock_rate = 0.0F;
    float fortification_unlock_rate = 0.0F;
    float sailing_unlock_rate = 0.0F;
    float average_ore_income = 0.0F;
    float average_ore_income_for_mining_polities = 0.0F;
    float average_tool_efficiency = 0.0F;
    float average_military_potential = 0.0F;
    float average_route_maintenance = 0.0F;
    float average_admin_distance_cost = 0.0F;
    float average_admin_distance_saving = 0.0F;
    float average_connected_ore_income = 0.0F;
    float average_unconnected_ore_income = 0.0F;
    float average_trade_profit = 0.0F;
    float average_trade_complementarity = 0.0F;
    float average_trade_route_cost = 0.0F;
    float average_trade_route_efficiency = 0.0F;
    float average_trade_weak_refresh_count = 0.0F;
    float average_trade_path_tiles = 0.0F;
    float average_friendship = 0.0F;
    float average_competition = 0.0F;
    float average_blockade_tendency = 0.0F;
    float average_diplomatic_grievance = 0.0F;
    float average_diplomatic_vassalage = 0.0F;
    float average_diplomatic_restraint = 0.0F;
    float average_war_roi = 0.0F;
    float max_declaration_pressure = 0.0F;
    float average_trade_conflict_weight = 0.0F;
    float average_friendly_penalty = 0.0F;
    float average_blockade_pressure = 0.0F;
    float average_dependency_pressure = 0.0F;
    float average_grievance_pressure = 0.0F;
    float average_restraint_pressure = 0.0F;
    float average_vassalage_pressure = 0.0F;
    float average_war_target_roi = 0.0F;
    float max_war_target_score = 0.0F;
    float average_war_target_value = 0.0F;
    float average_campaign_cost = 0.0F;
    float average_occupation_cost = 0.0F;
    float war_population_lost = 0.0F;
    float war_food_spent = 0.0F;
    float war_equipment_spent = 0.0F;
    float average_war_progress = 0.0F;
    float average_active_occupation_unrest = 0.0F;
    float average_active_occupation_maintenance = 0.0F;
    float average_vassal_loyalty = 0.0F;
    float average_vassal_liberty_desire = 0.0F;
    float total_vassal_tribute_due = 0.0F;
    float average_occupation_load = 0.0F;
    float average_occupation_unrest = 0.0F;
};

[[nodiscard]] SimBalanceMetrics RunOne(const SimBalanceRunConfig& config, std::uint64_t seed);
[[nodiscard]] nlohmann::json ToJson(const SimBalanceMetrics& metrics);
[[nodiscard]] nlohmann::json Aggregate(const std::vector<SimBalanceMetrics>& metrics);
void WriteCsvHeader(std::ofstream& output);
void WriteCsvRow(std::ofstream& output, const SimBalanceMetrics& metrics);

} // namespace oikumene::tools
