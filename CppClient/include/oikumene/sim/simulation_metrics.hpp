#pragma once

#include "oikumene/core/simulation.hpp"
#include "oikumene/sim/diplomacy.hpp"
#include "oikumene/sim/event.hpp"
#include "oikumene/sim/occupation.hpp"
#include "oikumene/sim/route.hpp"
#include "oikumene/sim/technology.hpp"
#include "oikumene/sim/vassal_treaty.hpp"
#include "oikumene/sim/war_campaign.hpp"
#include "oikumene/world/improvement.hpp"

namespace oikumene {

[[nodiscard]] int CountActiveBands(const Simulation& sim);
[[nodiscard]] int CountVillages(const Simulation& sim);
[[nodiscard]] int CountCamps(const Simulation& sim);
[[nodiscard]] int CountInactiveBands(const Simulation& sim);
[[nodiscard]] int CountEvents(const Simulation& sim, EventType type);
[[nodiscard]] int CountImprovement(const Simulation& sim, ImprovementKind kind);
[[nodiscard]] int CountWorkedTiles(const Simulation& sim);
[[nodiscard]] float TotalFoodOutputLastTurn(const Simulation& sim);
[[nodiscard]] float TotalFoodConsumptionLastTurn(const Simulation& sim);
[[nodiscard]] float TotalWoodOutputLastTurn(const Simulation& sim);
[[nodiscard]] float AverageCarryingCapacity(const Simulation& sim);
[[nodiscard]] float ControlledLandRatio(const Simulation& sim);
[[nodiscard]] int CountContestedTiles(const Simulation& sim);
[[nodiscard]] int CountRouteTiles(const Simulation& sim);
[[nodiscard]] int CountRouteKindTiles(const Simulation& sim, RouteKind kind);
[[nodiscard]] int CountActiveTrades(const Simulation& sim);
[[nodiscard]] float TotalTradeProfit(const Simulation& sim);
[[nodiscard]] float AverageTradeRouteEfficiency(const Simulation& sim);
[[nodiscard]] int CountDiplomacyPosture(const Simulation& sim, DiplomaticPosture posture);
[[nodiscard]] float AverageFriendship(const Simulation& sim);
[[nodiscard]] float AverageCompetition(const Simulation& sim);
[[nodiscard]] float AverageBlockadeTendency(const Simulation& sim);
[[nodiscard]] float AverageDiplomaticGrievance(const Simulation& sim);
[[nodiscard]] float AverageDiplomaticVassalage(const Simulation& sim);
[[nodiscard]] float AverageDiplomaticRestraint(const Simulation& sim);
[[nodiscard]] int CountHighWarPressures(const Simulation& sim);
[[nodiscard]] float AverageWarRoi(const Simulation& sim);
[[nodiscard]] float MaxDeclarationPressure(const Simulation& sim);
[[nodiscard]] float AverageTradeConflictWeight(const Simulation& sim);
[[nodiscard]] float AverageFriendlyPenalty(const Simulation& sim);
[[nodiscard]] float AverageBlockadePressure(const Simulation& sim);
[[nodiscard]] float AverageDependencyPressure(const Simulation& sim);
[[nodiscard]] float AverageGrievancePressure(const Simulation& sim);
[[nodiscard]] float AverageRestraintPressure(const Simulation& sim);
[[nodiscard]] float AverageVassalagePressure(const Simulation& sim);
[[nodiscard]] int CountHighWarTargets(const Simulation& sim);
[[nodiscard]] float AverageWarTargetRoi(const Simulation& sim);
[[nodiscard]] float MaxWarTargetScore(const Simulation& sim);
[[nodiscard]] float AverageWarTargetValue(const Simulation& sim);
[[nodiscard]] float AverageCampaignCost(const Simulation& sim);
[[nodiscard]] float AverageOccupationCost(const Simulation& sim);
[[nodiscard]] int CountWarsByStatus(const Simulation& sim, WarCampaignStatus status);
[[nodiscard]] int CountOccupationsByStatus(const Simulation& sim, OccupationStatus status);
[[nodiscard]] float AverageActiveOccupationUnrest(const Simulation& sim);
[[nodiscard]] float AverageActiveOccupationMaintenance(const Simulation& sim);
[[nodiscard]] int CountVassalTreatiesByStatus(const Simulation& sim, VassalTreatyStatus status);
[[nodiscard]] float AverageActiveVassalLoyalty(const Simulation& sim);
[[nodiscard]] float AverageActiveVassalLibertyDesire(const Simulation& sim);
[[nodiscard]] float TotalActiveVassalTributeDue(const Simulation& sim);
[[nodiscard]] float TotalWarPopulationLost(const Simulation& sim);
[[nodiscard]] float TotalWarFoodSpent(const Simulation& sim);
[[nodiscard]] float TotalWarEquipmentSpent(const Simulation& sim);
[[nodiscard]] float AverageWarProgress(const Simulation& sim);
[[nodiscard]] float AverageAdminLoad(const Simulation& sim);
[[nodiscard]] float AverageAdminCapacity(const Simulation& sim);
[[nodiscard]] float AverageOverextension(const Simulation& sim);
[[nodiscard]] float AverageStability(const Simulation& sim);
[[nodiscard]] float AverageUnlockedTechs(const Simulation& sim);
[[nodiscard]] float AverageKnowledgeIncome(const Simulation& sim);
[[nodiscard]] float TechUnlockRate(const Simulation& sim, TechId tech);
[[nodiscard]] float AverageSettlementScore(const Simulation& sim);
[[nodiscard]] float AverageSettlementFertility(const Simulation& sim);
[[nodiscard]] int MaxSettlementPopulation(const Simulation& sim);
[[nodiscard]] int TotalPopulation(const Simulation& sim);

} // namespace oikumene
