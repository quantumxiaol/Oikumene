#include "sim_balance_metrics.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include "oikumene/core/simulation.hpp"
#include "oikumene/sim/control_field.hpp"
#include "oikumene/sim/tech_effects.hpp"
#include "oikumene/world/world_generator.hpp"

namespace oikumene::tools {
namespace {

int CountEvents(const oikumene::Simulation& sim, oikumene::EventType type) {
    int count = 0;
    for (const auto& event : sim.Events().Events()) {
        count += event.type == type ? 1 : 0;
    }
    return count;
}

int CountImprovement(const oikumene::Simulation& sim, oikumene::ImprovementKind kind) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += tile.improvement == kind ? 1 : 0;
    }
    return count;
}

int CountLandTiles(const oikumene::Simulation& sim) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += !tile.is_ocean && !tile.is_lake ? 1 : 0;
    }
    return count;
}

const oikumene::Settlement* SettlementById(const std::vector<oikumene::Settlement>& settlements, int id) {
    for (const auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

float AverageAdminDistanceCost(const oikumene::Simulation& sim, const oikumene::Polity& polity) {
    const auto* capital = SettlementById(sim.Settlements(), polity.capital_settlement_id);
    if (capital == nullptr || polity.member_settlement_ids.size() <= 1) {
        return 0.0F;
    }

    const auto effects = oikumene::ComputeTechEffects(polity.research);
    float total = 0.0F;
    int measured = 0;
    for (const int settlement_id : polity.member_settlement_ids) {
        if (settlement_id == polity.capital_settlement_id) {
            continue;
        }
        const auto* settlement = SettlementById(sim.Settlements(), settlement_id);
        if (settlement == nullptr) {
            continue;
        }
        float cost = oikumene::TerrainPathCost(sim.GetWorld(), capital->x, capital->y, settlement->x, settlement->y,
                                               160.0F, effects.control_path_cost_multiplier,
                                               effects.coastal_control_cost_multiplier, polity.id, true);
        if (!std::isfinite(cost)) {
            cost = 160.0F;
        }
        total += cost;
        ++measured;
    }
    return measured <= 0 ? 0.0F : total / static_cast<float>(measured);
}

} // namespace

SimBalanceMetrics RunOne(const SimBalanceRunConfig& config, std::uint64_t seed) {
    oikumene::WorldGenerationParams world_params;
    world_params.seed = seed;
    world_params.width = config.width;
    world_params.height = config.height;

    oikumene::SimulationParams sim_params;
    sim_params.initial_band_count = config.bands;
    sim_params.enable_routes = config.enable_routes;

    oikumene::Simulation sim(oikumene::WorldGenerator::Generate(world_params), sim_params);
    sim.InitializeBands(config.bands);
    for (int i = 0; i < config.turns; ++i) {
        sim.AdvanceOneTurn();
    }

    SimBalanceMetrics metrics;
    metrics.seed = seed;
    metrics.routes_enabled = config.enable_routes;
    metrics.settlements = static_cast<int>(sim.Settlements().size());
    for (const auto& band : sim.Bands()) {
        metrics.active_bands += band.active ? 1 : 0;
        metrics.total_population += band.active ? band.population : 0;
    }
    float carrying_capacity_sum = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        metrics.villages += settlement.level == oikumene::SettlementLevel::Village ? 1 : 0;
        metrics.total_population += settlement.population;
        metrics.max_settlement_population = std::max(metrics.max_settlement_population, settlement.population);
        metrics.total_food_output += settlement.local_food_output_last_turn;
        metrics.total_food_consumption += settlement.food_consumption_last_turn;
        metrics.total_wood_output += settlement.local_wood_output_last_turn;
        carrying_capacity_sum += settlement.carrying_capacity;
    }
    metrics.camps = metrics.settlements - metrics.villages;
    metrics.average_carrying_capacity =
        metrics.settlements <= 0 ? 0.0F : carrying_capacity_sum / static_cast<float>(metrics.settlements);

    metrics.farms = CountImprovement(sim, oikumene::ImprovementKind::Farm);
    metrics.lumber_camps = CountImprovement(sim, oikumene::ImprovementKind::LumberCamp);
    metrics.pastures = CountImprovement(sim, oikumene::ImprovementKind::Pasture);
    metrics.shallow_mines = CountImprovement(sim, oikumene::ImprovementKind::ShallowMine);
    int controlled_land_tiles = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        metrics.worked_tiles += tile.worked_by_settlement_id >= 0 ? 1 : 0;
        controlled_land_tiles +=
            tile.controller_polity_id != oikumene::kInvalidPolityId && !tile.is_ocean && !tile.is_lake ? 1 : 0;
        metrics.contested_tiles += tile.is_contested ? 1 : 0;
        if (tile.has_route) {
            ++metrics.route_tiles;
            metrics.road_tiles += tile.route_kind == oikumene::RouteKind::Road ? 1 : 0;
            metrics.trail_tiles += tile.route_kind == oikumene::RouteKind::Trail ? 1 : 0;
            metrics.river_route_tiles += tile.route_kind == oikumene::RouteKind::RiverRoute ? 1 : 0;
            metrics.coastal_route_tiles += tile.route_kind == oikumene::RouteKind::CoastalRoute ? 1 : 0;
        }
    }
    const int land_tiles = CountLandTiles(sim);
    metrics.controlled_land_ratio =
        land_tiles <= 0 ? 0.0F : static_cast<float>(controlled_land_tiles) / static_cast<float>(land_tiles);
    metrics.polities = static_cast<int>(sim.Polities().size());
    int member_sum = 0;
    float formation_turn_sum = 0.0F;
    float food_income_sum = 0.0F;
    float wood_income_sum = 0.0F;
    float wealth_income_sum = 0.0F;
    float admin_load_sum = 0.0F;
    float admin_capacity_sum = 0.0F;
    float overextension_sum = 0.0F;
    float stability_sum = 0.0F;
    float occupation_load_sum = 0.0F;
    float occupation_unrest_sum = 0.0F;
    float control_maintenance_sum = 0.0F;
    float unlocked_tech_sum = 0.0F;
    float knowledge_income_sum = 0.0F;
    float first_tech_turn_sum = 0.0F;
    int first_tech_turn_count = 0;
    int pottery_count = 0;
    int irrigation_count = 0;
    int animal_husbandry_count = 0;
    int mining_count = 0;
    int roads_count = 0;
    int administration_count = 0;
    int bronze_count = 0;
    int fortification_count = 0;
    int sailing_count = 0;
    float ore_income_sum = 0.0F;
    float mining_polity_ore_income_sum = 0.0F;
    int mining_polity_count = 0;
    float tool_efficiency_sum = 0.0F;
    float military_potential_sum = 0.0F;
    float route_maintenance_sum = 0.0F;
    float admin_distance_cost_sum = 0.0F;
    float admin_distance_saving_sum = 0.0F;
    float connected_ore_income_sum = 0.0F;
    float unconnected_ore_income_sum = 0.0F;
    for (const auto& polity : sim.Polities()) {
        metrics.largest_polity_population = std::max(metrics.largest_polity_population, polity.population);
        member_sum += static_cast<int>(polity.member_settlement_ids.size());
        formation_turn_sum += static_cast<float>(polity.founded_turn);
        food_income_sum += polity.budget.food_income;
        wood_income_sum += polity.budget.wood_income;
        wealth_income_sum += polity.budget.wealth_income;
        admin_load_sum += polity.admin_load;
        admin_capacity_sum += polity.admin_capacity;
        overextension_sum += polity.overextension;
        stability_sum += polity.stability;
        occupation_load_sum += polity.occupation_load;
        occupation_unrest_sum += polity.occupation_unrest;
        control_maintenance_sum += polity.budget.control_maintenance;
        unlocked_tech_sum += static_cast<float>(polity.research.unlocked.size());
        knowledge_income_sum += polity.knowledge_income;
        if (polity.research.first_unlock_turn >= 0) {
            first_tech_turn_sum += static_cast<float>(polity.research.first_unlock_turn);
            ++first_tech_turn_count;
        }
        pottery_count += oikumene::HasTech(polity.research, oikumene::TechId::Pottery) ? 1 : 0;
        irrigation_count += oikumene::HasTech(polity.research, oikumene::TechId::Irrigation) ? 1 : 0;
        animal_husbandry_count += oikumene::HasTech(polity.research, oikumene::TechId::AnimalHusbandry) ? 1 : 0;
        mining_count += oikumene::HasTech(polity.research, oikumene::TechId::Mining) ? 1 : 0;
        roads_count += oikumene::HasTech(polity.research, oikumene::TechId::Roads) ? 1 : 0;
        administration_count += oikumene::HasTech(polity.research, oikumene::TechId::Administration) ? 1 : 0;
        bronze_count += oikumene::HasTech(polity.research, oikumene::TechId::BronzeWorking) ? 1 : 0;
        fortification_count += oikumene::HasTech(polity.research, oikumene::TechId::Fortification) ? 1 : 0;
        sailing_count += oikumene::HasTech(polity.research, oikumene::TechId::Sailing) ? 1 : 0;
        ore_income_sum += polity.budget.ore_income;
        if (oikumene::HasTech(polity.research, oikumene::TechId::Mining)) {
            mining_polity_ore_income_sum += polity.budget.ore_income;
            ++mining_polity_count;
        }
        tool_efficiency_sum += polity.tool_efficiency;
        military_potential_sum += polity.military_potential;
        metrics.connected_settlements += polity.connected_settlements;
        metrics.connected_mines += polity.connected_mines;
        metrics.connected_mine_potential += polity.connected_mine_potential;
        metrics.active_connected_mines += polity.active_connected_mines;
        route_maintenance_sum += polity.route_maintenance;
        admin_distance_cost_sum += AverageAdminDistanceCost(sim, polity);
        admin_distance_saving_sum += polity.admin_distance_saving;
        connected_ore_income_sum += polity.connected_ore_income;
        unconnected_ore_income_sum += polity.unconnected_ore_income;
    }
    metrics.average_member_settlements_per_polity =
        metrics.polities <= 0 ? 0.0F : static_cast<float>(member_sum) / static_cast<float>(metrics.polities);
    metrics.polity_formation_turn_mean =
        metrics.polities <= 0 ? 0.0F : formation_turn_sum / static_cast<float>(metrics.polities);
    metrics.average_polity_food_income =
        metrics.polities <= 0 ? 0.0F : food_income_sum / static_cast<float>(metrics.polities);
    metrics.average_polity_wood_income =
        metrics.polities <= 0 ? 0.0F : wood_income_sum / static_cast<float>(metrics.polities);
    metrics.average_polity_wealth_income =
        metrics.polities <= 0 ? 0.0F : wealth_income_sum / static_cast<float>(metrics.polities);
    metrics.average_admin_load = metrics.polities <= 0 ? 0.0F : admin_load_sum / static_cast<float>(metrics.polities);
    metrics.average_admin_capacity =
        metrics.polities <= 0 ? 0.0F : admin_capacity_sum / static_cast<float>(metrics.polities);
    metrics.average_overextension =
        metrics.polities <= 0 ? 0.0F : overextension_sum / static_cast<float>(metrics.polities);
    metrics.average_stability = metrics.polities <= 0 ? 0.0F : stability_sum / static_cast<float>(metrics.polities);
    metrics.average_occupation_load =
        metrics.polities <= 0 ? 0.0F : occupation_load_sum / static_cast<float>(metrics.polities);
    metrics.average_occupation_unrest =
        metrics.polities <= 0 ? 0.0F : occupation_unrest_sum / static_cast<float>(metrics.polities);
    metrics.average_control_maintenance =
        metrics.polities <= 0 ? 0.0F : control_maintenance_sum / static_cast<float>(metrics.polities);
    metrics.average_unlocked_techs =
        metrics.polities <= 0 ? 0.0F : unlocked_tech_sum / static_cast<float>(metrics.polities);
    metrics.average_knowledge_income =
        metrics.polities <= 0 ? 0.0F : knowledge_income_sum / static_cast<float>(metrics.polities);
    metrics.first_tech_turn_mean =
        first_tech_turn_count <= 0 ? 0.0F : first_tech_turn_sum / static_cast<float>(first_tech_turn_count);
    metrics.pottery_unlock_rate =
        metrics.polities <= 0 ? 0.0F : static_cast<float>(pottery_count) / static_cast<float>(metrics.polities);
    metrics.irrigation_unlock_rate =
        metrics.polities <= 0 ? 0.0F : static_cast<float>(irrigation_count) / static_cast<float>(metrics.polities);
    metrics.animal_husbandry_unlock_rate =
        metrics.polities <= 0 ? 0.0F
                              : static_cast<float>(animal_husbandry_count) / static_cast<float>(metrics.polities);
    metrics.mining_unlock_rate =
        metrics.polities <= 0 ? 0.0F : static_cast<float>(mining_count) / static_cast<float>(metrics.polities);
    metrics.roads_unlock_rate =
        metrics.polities <= 0 ? 0.0F : static_cast<float>(roads_count) / static_cast<float>(metrics.polities);
    metrics.administration_unlock_rate =
        metrics.polities <= 0 ? 0.0F : static_cast<float>(administration_count) / static_cast<float>(metrics.polities);
    metrics.bronze_working_unlock_rate =
        metrics.polities <= 0 ? 0.0F : static_cast<float>(bronze_count) / static_cast<float>(metrics.polities);
    metrics.fortification_unlock_rate =
        metrics.polities <= 0 ? 0.0F : static_cast<float>(fortification_count) / static_cast<float>(metrics.polities);
    metrics.sailing_unlock_rate =
        metrics.polities <= 0 ? 0.0F : static_cast<float>(sailing_count) / static_cast<float>(metrics.polities);
    metrics.average_ore_income = metrics.polities <= 0 ? 0.0F : ore_income_sum / static_cast<float>(metrics.polities);
    metrics.average_ore_income_for_mining_polities =
        mining_polity_count <= 0 ? 0.0F : mining_polity_ore_income_sum / static_cast<float>(mining_polity_count);
    metrics.average_tool_efficiency =
        metrics.polities <= 0 ? 0.0F : tool_efficiency_sum / static_cast<float>(metrics.polities);
    metrics.average_military_potential =
        metrics.polities <= 0 ? 0.0F : military_potential_sum / static_cast<float>(metrics.polities);
    metrics.average_route_maintenance =
        metrics.polities <= 0 ? 0.0F : route_maintenance_sum / static_cast<float>(metrics.polities);
    metrics.average_admin_distance_cost =
        metrics.polities <= 0 ? 0.0F : admin_distance_cost_sum / static_cast<float>(metrics.polities);
    metrics.average_admin_distance_saving =
        metrics.polities <= 0 ? 0.0F : admin_distance_saving_sum / static_cast<float>(metrics.polities);
    metrics.average_connected_ore_income =
        metrics.polities <= 0 ? 0.0F : connected_ore_income_sum / static_cast<float>(metrics.polities);
    metrics.average_unconnected_ore_income =
        metrics.polities <= 0 ? 0.0F : unconnected_ore_income_sum / static_cast<float>(metrics.polities);
    metrics.famine_events = CountEvents(sim, oikumene::EventType::Famine);
    metrics.farm_events = CountEvents(sim, oikumene::EventType::FarmBuilt);
    metrics.lumber_events = CountEvents(sim, oikumene::EventType::LumberCampBuilt);
    metrics.pasture_events = CountEvents(sim, oikumene::EventType::PastureBuilt);
    metrics.route_events = CountEvents(sim, oikumene::EventType::RouteBuilt);
    metrics.trade_events = CountEvents(sim, oikumene::EventType::TradeOpened);
    metrics.war_declared_events = CountEvents(sim, oikumene::EventType::WarDeclared);
    metrics.war_occupied_events = CountEvents(sim, oikumene::EventType::WarTargetOccupied);
    metrics.war_retreat_events = CountEvents(sim, oikumene::EventType::WarRetreated);
    metrics.peace_events = CountEvents(sim, oikumene::EventType::PeaceSigned);
    metrics.territory_ceded_events = CountEvents(sim, oikumene::EventType::TerritoryCeded);
    metrics.occupation_withdrawn_events = CountEvents(sim, oikumene::EventType::OccupationWithdrawn);
    metrics.vassal_created_events = CountEvents(sim, oikumene::EventType::VassalCreated);
    metrics.occupation_revolt_events = CountEvents(sim, oikumene::EventType::OccupationRevolt);
    metrics.routes = static_cast<int>(sim.Routes().size());
    float trade_profit_sum = 0.0F;
    float trade_complementarity_sum = 0.0F;
    float trade_route_cost_sum = 0.0F;
    float trade_route_efficiency_sum = 0.0F;
    float trade_weak_refresh_sum = 0.0F;
    float trade_path_tiles_sum = 0.0F;
    for (const auto& trade : sim.Trades()) {
        if (!trade.active) {
            continue;
        }
        ++metrics.active_trades;
        trade_profit_sum += trade.expected_profit;
        trade_complementarity_sum += trade.complementarity;
        trade_route_cost_sum += trade.route_cost;
        trade_route_efficiency_sum += trade.route_efficiency;
        trade_weak_refresh_sum += static_cast<float>(trade.weak_refresh_count);
        trade_path_tiles_sum += static_cast<float>(trade.path.size());
    }
    metrics.average_trade_profit =
        metrics.active_trades <= 0 ? 0.0F : trade_profit_sum / static_cast<float>(metrics.active_trades);
    metrics.average_trade_complementarity =
        metrics.active_trades <= 0 ? 0.0F : trade_complementarity_sum / static_cast<float>(metrics.active_trades);
    metrics.average_trade_route_cost =
        metrics.active_trades <= 0 ? 0.0F : trade_route_cost_sum / static_cast<float>(metrics.active_trades);
    metrics.average_trade_route_efficiency =
        metrics.active_trades <= 0 ? 0.0F : trade_route_efficiency_sum / static_cast<float>(metrics.active_trades);
    metrics.average_trade_weak_refresh_count =
        metrics.active_trades <= 0 ? 0.0F : trade_weak_refresh_sum / static_cast<float>(metrics.active_trades);
    metrics.average_trade_path_tiles =
        metrics.active_trades <= 0 ? 0.0F : trade_path_tiles_sum / static_cast<float>(metrics.active_trades);
    metrics.diplomacy_relations = static_cast<int>(sim.DiplomacyRelations().size());
    float friendship_sum = 0.0F;
    float competition_sum = 0.0F;
    float blockade_sum = 0.0F;
    float diplomatic_grievance_sum = 0.0F;
    float diplomatic_vassalage_sum = 0.0F;
    float diplomatic_restraint_sum = 0.0F;
    for (const auto& relation : sim.DiplomacyRelations()) {
        metrics.friendly_relations += relation.posture == oikumene::DiplomaticPosture::Friendly ? 1 : 0;
        metrics.competitive_relations += relation.posture == oikumene::DiplomaticPosture::Competitive ? 1 : 0;
        metrics.dependent_relations += relation.posture == oikumene::DiplomaticPosture::Dependent ? 1 : 0;
        metrics.blockade_risk_relations += relation.posture == oikumene::DiplomaticPosture::BlockadeRisk ? 1 : 0;
        friendship_sum += relation.friendship;
        competition_sum += relation.competition;
        blockade_sum += relation.blockade_tendency;
        diplomatic_grievance_sum += std::max(relation.grievance_a_to_b, relation.grievance_b_to_a);
        diplomatic_vassalage_sum += std::max(relation.vassalage_a_to_b, relation.vassalage_b_to_a);
        diplomatic_restraint_sum += std::max(relation.restraint_a_to_b, relation.restraint_b_to_a);
    }
    metrics.average_friendship =
        metrics.diplomacy_relations <= 0 ? 0.0F : friendship_sum / static_cast<float>(metrics.diplomacy_relations);
    metrics.average_competition =
        metrics.diplomacy_relations <= 0 ? 0.0F : competition_sum / static_cast<float>(metrics.diplomacy_relations);
    metrics.average_blockade_tendency =
        metrics.diplomacy_relations <= 0 ? 0.0F : blockade_sum / static_cast<float>(metrics.diplomacy_relations);
    metrics.average_diplomatic_grievance =
        metrics.diplomacy_relations <= 0 ? 0.0F
                                         : diplomatic_grievance_sum / static_cast<float>(metrics.diplomacy_relations);
    metrics.average_diplomatic_vassalage =
        metrics.diplomacy_relations <= 0 ? 0.0F
                                         : diplomatic_vassalage_sum / static_cast<float>(metrics.diplomacy_relations);
    metrics.average_diplomatic_restraint =
        metrics.diplomacy_relations <= 0 ? 0.0F
                                         : diplomatic_restraint_sum / static_cast<float>(metrics.diplomacy_relations);

    metrics.war_pressure_candidates = static_cast<int>(sim.WarPressures().size());
    float war_roi_sum = 0.0F;
    float trade_conflict_sum = 0.0F;
    float friendly_penalty_sum = 0.0F;
    float blockade_pressure_sum = 0.0F;
    float dependency_pressure_sum = 0.0F;
    float grievance_pressure_sum = 0.0F;
    float restraint_pressure_sum = 0.0F;
    float vassalage_pressure_sum = 0.0F;
    for (const auto& pressure : sim.WarPressures()) {
        metrics.high_war_pressure_candidates += pressure.high_pressure ? 1 : 0;
        war_roi_sum += pressure.war_roi;
        metrics.max_declaration_pressure = std::max(metrics.max_declaration_pressure, pressure.declaration_pressure);
        trade_conflict_sum += pressure.trade_conflict_weight;
        friendly_penalty_sum += pressure.friendly_penalty;
        blockade_pressure_sum += pressure.blockade_pressure;
        dependency_pressure_sum += pressure.dependency_pressure;
        grievance_pressure_sum += pressure.grievance_pressure;
        restraint_pressure_sum += pressure.restraint_pressure;
        vassalage_pressure_sum += pressure.vassalage_pressure;
    }
    if (metrics.war_pressure_candidates > 0) {
        const float count = static_cast<float>(metrics.war_pressure_candidates);
        metrics.average_war_roi = war_roi_sum / count;
        metrics.average_trade_conflict_weight = trade_conflict_sum / count;
        metrics.average_friendly_penalty = friendly_penalty_sum / count;
        metrics.average_blockade_pressure = blockade_pressure_sum / count;
        metrics.average_dependency_pressure = dependency_pressure_sum / count;
        metrics.average_grievance_pressure = grievance_pressure_sum / count;
        metrics.average_restraint_pressure = restraint_pressure_sum / count;
        metrics.average_vassalage_pressure = vassalage_pressure_sum / count;
    }
    metrics.war_target_candidates = static_cast<int>(sim.WarTargets().size());
    float war_target_roi_sum = 0.0F;
    float war_target_value_sum = 0.0F;
    float campaign_cost_sum = 0.0F;
    float occupation_cost_sum = 0.0F;
    for (const auto& target : sim.WarTargets()) {
        metrics.high_war_target_candidates += target.high_value ? 1 : 0;
        war_target_roi_sum += target.roi;
        metrics.max_war_target_score = std::max(metrics.max_war_target_score, target.action_score);
        war_target_value_sum += target.target_value;
        campaign_cost_sum += target.campaign_cost;
        occupation_cost_sum += target.occupation_cost;
    }
    if (metrics.war_target_candidates > 0) {
        const float count = static_cast<float>(metrics.war_target_candidates);
        metrics.average_war_target_roi = war_target_roi_sum / count;
        metrics.average_war_target_value = war_target_value_sum / count;
        metrics.average_campaign_cost = campaign_cost_sum / count;
        metrics.average_occupation_cost = occupation_cost_sum / count;
    }
    metrics.war_campaigns = static_cast<int>(sim.Wars().size());
    float war_progress_sum = 0.0F;
    for (const auto& campaign : sim.Wars()) {
        metrics.active_wars += campaign.status == oikumene::WarCampaignStatus::Active ? 1 : 0;
        metrics.occupied_wars += campaign.status == oikumene::WarCampaignStatus::Occupied ? 1 : 0;
        metrics.withdrawn_wars += campaign.status == oikumene::WarCampaignStatus::Withdrawn ? 1 : 0;
        metrics.peace_wars += campaign.status == oikumene::WarCampaignStatus::Peace ? 1 : 0;
        metrics.war_population_lost += campaign.population_lost;
        metrics.war_food_spent += campaign.food_spent;
        metrics.war_equipment_spent += campaign.equipment_spent;
        war_progress_sum += campaign.progress;
    }
    metrics.average_war_progress =
        metrics.war_campaigns <= 0 ? 0.0F : war_progress_sum / static_cast<float>(metrics.war_campaigns);
    metrics.occupations = static_cast<int>(sim.Occupations().size());
    float active_occupation_unrest_sum = 0.0F;
    float active_occupation_maintenance_sum = 0.0F;
    for (const auto& occupation : sim.Occupations()) {
        metrics.active_occupations += occupation.status == oikumene::OccupationStatus::Active ? 1 : 0;
        metrics.ceded_occupations += occupation.status == oikumene::OccupationStatus::Ceded ? 1 : 0;
        metrics.withdrawn_occupations += occupation.status == oikumene::OccupationStatus::Withdrawn ? 1 : 0;
        metrics.vassalized_occupations += occupation.status == oikumene::OccupationStatus::Vassalized ? 1 : 0;
        metrics.revolted_occupations += occupation.status == oikumene::OccupationStatus::Revolted ? 1 : 0;
        if (occupation.status == oikumene::OccupationStatus::Active) {
            active_occupation_unrest_sum += occupation.unrest;
            active_occupation_maintenance_sum += occupation.maintenance_cost;
        }
    }
    metrics.average_active_occupation_unrest =
        metrics.active_occupations <= 0 ? 0.0F
                                        : active_occupation_unrest_sum / static_cast<float>(metrics.active_occupations);
    metrics.average_active_occupation_maintenance =
        metrics.active_occupations <= 0
            ? 0.0F
            : active_occupation_maintenance_sum / static_cast<float>(metrics.active_occupations);
    metrics.vassal_treaties = static_cast<int>(sim.VassalTreaties().size());
    float vassal_loyalty_sum = 0.0F;
    float vassal_liberty_sum = 0.0F;
    for (const auto& treaty : sim.VassalTreaties()) {
        metrics.active_vassal_treaties += treaty.status == oikumene::VassalTreatyStatus::Active ? 1 : 0;
        metrics.broken_vassal_treaties += treaty.status == oikumene::VassalTreatyStatus::Broken ? 1 : 0;
        if (treaty.status == oikumene::VassalTreatyStatus::Active) {
            vassal_loyalty_sum += treaty.loyalty;
            vassal_liberty_sum += treaty.liberty_desire;
            metrics.total_vassal_tribute_due += treaty.tribute_due;
        }
    }
    metrics.average_vassal_loyalty = metrics.active_vassal_treaties <= 0
                                         ? 0.0F
                                         : vassal_loyalty_sum / static_cast<float>(metrics.active_vassal_treaties);
    metrics.average_vassal_liberty_desire =
        metrics.active_vassal_treaties <= 0 ? 0.0F
                                            : vassal_liberty_sum / static_cast<float>(metrics.active_vassal_treaties);
    metrics.food_output_consumption_ratio = metrics.total_food_output / std::max(1.0F, metrics.total_food_consumption);
    metrics.farm_share_of_worked_tiles =
        metrics.worked_tiles <= 0 ? 0.0F : static_cast<float>(metrics.farms) / static_cast<float>(metrics.worked_tiles);
    return metrics;
}

} // namespace oikumene::tools
