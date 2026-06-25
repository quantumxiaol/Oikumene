#include "oikumene/io/simulation_json.hpp"

#include "oikumene/ai/strategic_report.hpp"
#include "oikumene/sim/simulation_metrics.hpp"
#include "oikumene/world/improvement.hpp"
#include "oikumene/world/resource.hpp"
#include "oikumene/world/route_kind.hpp"

namespace oikumene {

nlohmann::json ImprovedTilesToJson(const Simulation& sim) {
    nlohmann::json tiles = nlohmann::json::array();
    for (const auto& tile : sim.GetWorld().Tiles()) {
        if (tile.improvement == ImprovementKind::None && tile.worked_by_settlement_id < 0) {
            continue;
        }
        tiles.push_back(nlohmann::json{
            {"x", tile.x},
            {"y", tile.y},
            {"improvement", ToString(tile.improvement)},
            {"worked_by_settlement_id", tile.worked_by_settlement_id},
            {"soil_quality", tile.soil_quality},
            {"forest_cover", tile.forest_cover},
            {"resource", ToString(tile.resource)},
        });
    }
    return tiles;
}

nlohmann::json RouteTilesToJson(const Simulation& sim) {
    nlohmann::json tiles = nlohmann::json::array();
    for (const auto& tile : sim.GetWorld().Tiles()) {
        if (!tile.has_route) {
            continue;
        }
        tiles.push_back(nlohmann::json{
            {"x", tile.x},
            {"y", tile.y},
            {"route_id", tile.route_id},
            {"polity_id", tile.route_polity_id},
            {"kind", ToString(tile.route_kind)},
            {"quality", tile.route_quality},
        });
    }
    return tiles;
}

nlohmann::json FinalStateToJson(const Simulation& sim) {
    nlohmann::json bands = nlohmann::json::array();
    for (const auto& band : sim.Bands()) {
        bands.push_back(BandToJson(band));
    }

    nlohmann::json settlements = nlohmann::json::array();
    for (const auto& settlement : sim.Settlements()) {
        settlements.push_back(SettlementToJson(settlement));
    }
    nlohmann::json polities = nlohmann::json::array();
    for (const auto& polity : sim.Polities()) {
        polities.push_back(PolityToJson(polity));
    }
    nlohmann::json routes = nlohmann::json::array();
    for (const auto& route : sim.Routes()) {
        routes.push_back(RouteToJson(route));
    }
    nlohmann::json trades = nlohmann::json::array();
    for (const auto& trade : sim.Trades()) {
        trades.push_back(TradeToJson(trade));
    }
    nlohmann::json diplomacy = nlohmann::json::array();
    for (const auto& relation : sim.DiplomacyRelations()) {
        diplomacy.push_back(DiplomacyToJson(relation));
    }
    nlohmann::json war_pressures = nlohmann::json::array();
    for (const auto& pressure : sim.WarPressures()) {
        war_pressures.push_back(WarPressureToJson(pressure));
    }
    nlohmann::json war_targets = nlohmann::json::array();
    for (const auto& target : sim.WarTargets()) {
        war_targets.push_back(WarTargetToJson(target));
    }
    nlohmann::json wars = nlohmann::json::array();
    for (const auto& campaign : sim.Wars()) {
        wars.push_back(WarCampaignToJson(campaign));
    }
    nlohmann::json occupations = nlohmann::json::array();
    for (const auto& occupation : sim.Occupations()) {
        occupations.push_back(OccupationToJson(occupation));
    }
    nlohmann::json vassal_treaties = nlohmann::json::array();
    for (const auto& treaty : sim.VassalTreaties()) {
        vassal_treaties.push_back(VassalTreatyToJson(treaty));
    }

    return nlohmann::json{
        {"turn", sim.CurrentTurn()},
        {"status", sim.StatusSummary()},
        {"active_bands", CountActiveBands(sim)},
        {"settlements", settlements},
        {"polities", polities},
        {"routes", routes},
        {"trades", trades},
        {"diplomacy_relations", diplomacy},
        {"war_pressures", war_pressures},
        {"war_targets", war_targets},
        {"wars", wars},
        {"occupations", occupations},
        {"vassal_treaties", vassal_treaties},
        {"bands", bands},
        {"improved_tiles", ImprovedTilesToJson(sim)},
        {"route_tiles", RouteTilesToJson(sim)},
        {"event_count", sim.Events().Size()},
    };
}

nlohmann::json SummaryToJson(const SimulationRunMetadata& metadata, const Simulation& sim) {
    const int total_population = TotalPopulation(sim);
    return nlohmann::json{
        {"seed", metadata.seed},
        {"width", metadata.width},
        {"height", metadata.height},
        {"turns", metadata.turns},
        {"initial_bands", metadata.initial_bands},
        {"routes_enabled", metadata.routes_enabled},
        {"active_bands", CountActiveBands(sim)},
        {"inactive_bands", CountInactiveBands(sim)},
        {"settlements", sim.Settlements().size()},
        {"camps", CountCamps(sim)},
        {"villages", CountVillages(sim)},
        {"total_population", total_population},
        {"population_total", total_population},
        {"average_settlement_score", AverageSettlementScore(sim)},
        {"average_settlement_fertility", AverageSettlementFertility(sim)},
        {"max_settlement_population", MaxSettlementPopulation(sim)},
        {"farm_count", CountImprovement(sim, ImprovementKind::Farm)},
        {"lumbercamp_count", CountImprovement(sim, ImprovementKind::LumberCamp)},
        {"pasture_count", CountImprovement(sim, ImprovementKind::Pasture)},
        {"shallow_mine_count", CountImprovement(sim, ImprovementKind::ShallowMine)},
        {"worked_tile_count", CountWorkedTiles(sim)},
        {"total_food_output_last_turn", TotalFoodOutputLastTurn(sim)},
        {"total_food_consumption_last_turn", TotalFoodConsumptionLastTurn(sim)},
        {"total_wood_output_last_turn", TotalWoodOutputLastTurn(sim)},
        {"average_carrying_capacity", AverageCarryingCapacity(sim)},
        {"polities", sim.Polities().size()},
        {"controlled_land_ratio", ControlledLandRatio(sim)},
        {"contested_tiles", CountContestedTiles(sim)},
        {"routes", sim.Routes().size()},
        {"active_trades", CountActiveTrades(sim)},
        {"total_trade_profit", TotalTradeProfit(sim)},
        {"average_trade_route_efficiency", AverageTradeRouteEfficiency(sim)},
        {"diplomacy_relations", sim.DiplomacyRelations().size()},
        {"friendly_relations", CountDiplomacyPosture(sim, DiplomaticPosture::Friendly)},
        {"competitive_relations", CountDiplomacyPosture(sim, DiplomaticPosture::Competitive)},
        {"dependent_relations", CountDiplomacyPosture(sim, DiplomaticPosture::Dependent)},
        {"blockade_risk_relations", CountDiplomacyPosture(sim, DiplomaticPosture::BlockadeRisk)},
        {"average_friendship", AverageFriendship(sim)},
        {"average_competition", AverageCompetition(sim)},
        {"average_blockade_tendency", AverageBlockadeTendency(sim)},
        {"average_diplomatic_grievance", AverageDiplomaticGrievance(sim)},
        {"average_diplomatic_vassalage", AverageDiplomaticVassalage(sim)},
        {"average_diplomatic_restraint", AverageDiplomaticRestraint(sim)},
        {"war_pressure_candidates", sim.WarPressures().size()},
        {"high_war_pressure_candidates", CountHighWarPressures(sim)},
        {"average_war_roi", AverageWarRoi(sim)},
        {"max_declaration_pressure", MaxDeclarationPressure(sim)},
        {"average_trade_conflict_weight", AverageTradeConflictWeight(sim)},
        {"average_friendly_penalty", AverageFriendlyPenalty(sim)},
        {"average_blockade_pressure", AverageBlockadePressure(sim)},
        {"average_dependency_pressure", AverageDependencyPressure(sim)},
        {"average_grievance_pressure", AverageGrievancePressure(sim)},
        {"average_restraint_pressure", AverageRestraintPressure(sim)},
        {"average_vassalage_pressure", AverageVassalagePressure(sim)},
        {"war_target_candidates", sim.WarTargets().size()},
        {"high_war_target_candidates", CountHighWarTargets(sim)},
        {"average_war_target_roi", AverageWarTargetRoi(sim)},
        {"max_war_target_score", MaxWarTargetScore(sim)},
        {"average_war_target_value", AverageWarTargetValue(sim)},
        {"average_campaign_cost", AverageCampaignCost(sim)},
        {"average_occupation_cost", AverageOccupationCost(sim)},
        {"war_campaigns", sim.Wars().size()},
        {"active_wars", CountWarsByStatus(sim, WarCampaignStatus::Active)},
        {"occupied_wars", CountWarsByStatus(sim, WarCampaignStatus::Occupied)},
        {"withdrawn_wars", CountWarsByStatus(sim, WarCampaignStatus::Withdrawn)},
        {"peace_wars", CountWarsByStatus(sim, WarCampaignStatus::Peace)},
        {"occupations", sim.Occupations().size()},
        {"active_occupations", CountOccupationsByStatus(sim, OccupationStatus::Active)},
        {"ceded_occupations", CountOccupationsByStatus(sim, OccupationStatus::Ceded)},
        {"withdrawn_occupations", CountOccupationsByStatus(sim, OccupationStatus::Withdrawn)},
        {"vassalized_occupations", CountOccupationsByStatus(sim, OccupationStatus::Vassalized)},
        {"revolted_occupations", CountOccupationsByStatus(sim, OccupationStatus::Revolted)},
        {"vassal_treaties", sim.VassalTreaties().size()},
        {"active_vassal_treaties", CountVassalTreatiesByStatus(sim, VassalTreatyStatus::Active)},
        {"broken_vassal_treaties", CountVassalTreatiesByStatus(sim, VassalTreatyStatus::Broken)},
        {"average_vassal_loyalty", AverageActiveVassalLoyalty(sim)},
        {"average_vassal_liberty_desire", AverageActiveVassalLibertyDesire(sim)},
        {"total_vassal_tribute_due", TotalActiveVassalTributeDue(sim)},
        {"average_active_occupation_unrest", AverageActiveOccupationUnrest(sim)},
        {"average_active_occupation_maintenance", AverageActiveOccupationMaintenance(sim)},
        {"war_population_lost", TotalWarPopulationLost(sim)},
        {"war_food_spent", TotalWarFoodSpent(sim)},
        {"war_equipment_spent", TotalWarEquipmentSpent(sim)},
        {"average_war_progress", AverageWarProgress(sim)},
        {"route_tile_count", CountRouteTiles(sim)},
        {"road_tile_count", CountRouteKindTiles(sim, RouteKind::Road)},
        {"trail_tile_count", CountRouteKindTiles(sim, RouteKind::Trail)},
        {"river_route_tile_count", CountRouteKindTiles(sim, RouteKind::RiverRoute)},
        {"coastal_route_tile_count", CountRouteKindTiles(sim, RouteKind::CoastalRoute)},
        {"average_admin_load", AverageAdminLoad(sim)},
        {"average_admin_capacity", AverageAdminCapacity(sim)},
        {"average_overextension", AverageOverextension(sim)},
        {"average_stability", AverageStability(sim)},
        {"average_unlocked_techs", AverageUnlockedTechs(sim)},
        {"average_knowledge_income", AverageKnowledgeIncome(sim)},
        {"mining_unlock_rate", TechUnlockRate(sim, TechId::Mining)},
        {"roads_unlock_rate", TechUnlockRate(sim, TechId::Roads)},
        {"administration_unlock_rate", TechUnlockRate(sim, TechId::Administration)},
        {"event_count", sim.Events().Size()},
        {"migration_events", CountEvents(sim, EventType::BandMigrated)},
        {"settlement_founded_events", CountEvents(sim, EventType::SettlementFounded)},
        {"population_growth_events", CountEvents(sim, EventType::PopulationGrowth)},
        {"famine_events", CountEvents(sim, EventType::Famine)},
        {"village_upgrade_events", CountEvents(sim, EventType::SettlementUpgraded)},
        {"farm_built_events", CountEvents(sim, EventType::FarmBuilt)},
        {"lumbercamp_built_events", CountEvents(sim, EventType::LumberCampBuilt)},
        {"pasture_built_events", CountEvents(sim, EventType::PastureBuilt)},
        {"route_built_events", CountEvents(sim, EventType::RouteBuilt)},
        {"trade_opened_events", CountEvents(sim, EventType::TradeOpened)},
        {"territory_ceded_events", CountEvents(sim, EventType::TerritoryCeded)},
        {"occupation_withdrawn_events", CountEvents(sim, EventType::OccupationWithdrawn)},
        {"vassal_created_events", CountEvents(sim, EventType::VassalCreated)},
        {"occupation_revolt_events", CountEvents(sim, EventType::OccupationRevolt)},
        {"carrying_capacity_reached_events", CountEvents(sim, EventType::CarryingCapacityReached)},
    };
}

nlohmann::json StateSampleToJson(const Simulation& sim) {
    return nlohmann::json{
        {"turn", sim.CurrentTurn()},
        {"active_bands", CountActiveBands(sim)},
        {"inactive_bands", CountInactiveBands(sim)},
        {"settlements", sim.Settlements().size()},
        {"camps", CountCamps(sim)},
        {"villages", CountVillages(sim)},
        {"total_population", TotalPopulation(sim)},
        {"event_count", sim.Events().Size()},
        {"average_settlement_score", AverageSettlementScore(sim)},
        {"average_settlement_fertility", AverageSettlementFertility(sim)},
        {"max_settlement_population", MaxSettlementPopulation(sim)},
        {"farm_count", CountImprovement(sim, ImprovementKind::Farm)},
        {"lumbercamp_count", CountImprovement(sim, ImprovementKind::LumberCamp)},
        {"pasture_count", CountImprovement(sim, ImprovementKind::Pasture)},
        {"worked_tile_count", CountWorkedTiles(sim)},
        {"total_food_output_last_turn", TotalFoodOutputLastTurn(sim)},
        {"total_food_consumption_last_turn", TotalFoodConsumptionLastTurn(sim)},
        {"total_wood_output_last_turn", TotalWoodOutputLastTurn(sim)},
        {"average_carrying_capacity", AverageCarryingCapacity(sim)},
        {"polities", sim.Polities().size()},
        {"controlled_land_ratio", ControlledLandRatio(sim)},
        {"contested_tiles", CountContestedTiles(sim)},
        {"routes", sim.Routes().size()},
        {"active_trades", CountActiveTrades(sim)},
        {"total_trade_profit", TotalTradeProfit(sim)},
        {"average_trade_route_efficiency", AverageTradeRouteEfficiency(sim)},
        {"diplomacy_relations", sim.DiplomacyRelations().size()},
        {"friendly_relations", CountDiplomacyPosture(sim, DiplomaticPosture::Friendly)},
        {"competitive_relations", CountDiplomacyPosture(sim, DiplomaticPosture::Competitive)},
        {"dependent_relations", CountDiplomacyPosture(sim, DiplomaticPosture::Dependent)},
        {"blockade_risk_relations", CountDiplomacyPosture(sim, DiplomaticPosture::BlockadeRisk)},
        {"average_friendship", AverageFriendship(sim)},
        {"average_competition", AverageCompetition(sim)},
        {"average_blockade_tendency", AverageBlockadeTendency(sim)},
        {"average_diplomatic_grievance", AverageDiplomaticGrievance(sim)},
        {"average_diplomatic_vassalage", AverageDiplomaticVassalage(sim)},
        {"average_diplomatic_restraint", AverageDiplomaticRestraint(sim)},
        {"war_pressure_candidates", sim.WarPressures().size()},
        {"high_war_pressure_candidates", CountHighWarPressures(sim)},
        {"average_war_roi", AverageWarRoi(sim)},
        {"max_declaration_pressure", MaxDeclarationPressure(sim)},
        {"average_trade_conflict_weight", AverageTradeConflictWeight(sim)},
        {"average_friendly_penalty", AverageFriendlyPenalty(sim)},
        {"average_blockade_pressure", AverageBlockadePressure(sim)},
        {"average_dependency_pressure", AverageDependencyPressure(sim)},
        {"average_grievance_pressure", AverageGrievancePressure(sim)},
        {"average_restraint_pressure", AverageRestraintPressure(sim)},
        {"average_vassalage_pressure", AverageVassalagePressure(sim)},
        {"war_target_candidates", sim.WarTargets().size()},
        {"high_war_target_candidates", CountHighWarTargets(sim)},
        {"average_war_target_roi", AverageWarTargetRoi(sim)},
        {"max_war_target_score", MaxWarTargetScore(sim)},
        {"average_war_target_value", AverageWarTargetValue(sim)},
        {"average_campaign_cost", AverageCampaignCost(sim)},
        {"average_occupation_cost", AverageOccupationCost(sim)},
        {"war_campaigns", sim.Wars().size()},
        {"active_wars", CountWarsByStatus(sim, WarCampaignStatus::Active)},
        {"occupied_wars", CountWarsByStatus(sim, WarCampaignStatus::Occupied)},
        {"withdrawn_wars", CountWarsByStatus(sim, WarCampaignStatus::Withdrawn)},
        {"peace_wars", CountWarsByStatus(sim, WarCampaignStatus::Peace)},
        {"occupations", sim.Occupations().size()},
        {"active_occupations", CountOccupationsByStatus(sim, OccupationStatus::Active)},
        {"ceded_occupations", CountOccupationsByStatus(sim, OccupationStatus::Ceded)},
        {"withdrawn_occupations", CountOccupationsByStatus(sim, OccupationStatus::Withdrawn)},
        {"vassalized_occupations", CountOccupationsByStatus(sim, OccupationStatus::Vassalized)},
        {"revolted_occupations", CountOccupationsByStatus(sim, OccupationStatus::Revolted)},
        {"vassal_treaties", sim.VassalTreaties().size()},
        {"active_vassal_treaties", CountVassalTreatiesByStatus(sim, VassalTreatyStatus::Active)},
        {"broken_vassal_treaties", CountVassalTreatiesByStatus(sim, VassalTreatyStatus::Broken)},
        {"average_vassal_loyalty", AverageActiveVassalLoyalty(sim)},
        {"average_vassal_liberty_desire", AverageActiveVassalLibertyDesire(sim)},
        {"total_vassal_tribute_due", TotalActiveVassalTributeDue(sim)},
        {"average_active_occupation_unrest", AverageActiveOccupationUnrest(sim)},
        {"average_active_occupation_maintenance", AverageActiveOccupationMaintenance(sim)},
        {"war_population_lost", TotalWarPopulationLost(sim)},
        {"war_food_spent", TotalWarFoodSpent(sim)},
        {"war_equipment_spent", TotalWarEquipmentSpent(sim)},
        {"average_war_progress", AverageWarProgress(sim)},
        {"route_tile_count", CountRouteTiles(sim)},
        {"road_tile_count", CountRouteKindTiles(sim, RouteKind::Road)},
        {"trail_tile_count", CountRouteKindTiles(sim, RouteKind::Trail)},
        {"river_route_tile_count", CountRouteKindTiles(sim, RouteKind::RiverRoute)},
        {"coastal_route_tile_count", CountRouteKindTiles(sim, RouteKind::CoastalRoute)},
        {"average_admin_load", AverageAdminLoad(sim)},
        {"average_admin_capacity", AverageAdminCapacity(sim)},
        {"average_overextension", AverageOverextension(sim)},
        {"average_stability", AverageStability(sim)},
        {"average_unlocked_techs", AverageUnlockedTechs(sim)},
        {"average_knowledge_income", AverageKnowledgeIncome(sim)},
        {"mining_unlock_rate", TechUnlockRate(sim, TechId::Mining)},
        {"roads_unlock_rate", TechUnlockRate(sim, TechId::Roads)},
        {"administration_unlock_rate", TechUnlockRate(sim, TechId::Administration)},
    };
}

nlohmann::json StrategicReportsToJson(const Simulation& sim) {
    nlohmann::json reports = nlohmann::json::array();
    for (const auto& report : BuildStrategicReports(sim)) {
        reports.push_back(ToJson(report));
    }
    return reports;
}

nlohmann::json RecentEventsToJson(const EventLog& events, std::size_t max_count) {
    nlohmann::json values = nlohmann::json::array();
    const auto& all_events = events.Events();
    const std::size_t first = all_events.size() > max_count ? all_events.size() - max_count : 0;
    for (std::size_t i = first; i < all_events.size(); ++i) {
        values.push_back(ToJson(all_events[i]));
    }
    return values;
}

} // namespace oikumene
