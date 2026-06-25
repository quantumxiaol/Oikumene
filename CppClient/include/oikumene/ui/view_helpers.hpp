#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <raylib.h>

#include "oikumene/ai/remote_decision_provider.hpp"
#include "oikumene/core/types.hpp"
#include "oikumene/sim/band.hpp"
#include "oikumene/sim/diplomacy.hpp"
#include "oikumene/sim/occupation.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/route.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/tech_effects.hpp"
#include "oikumene/sim/trade.hpp"
#include "oikumene/sim/vassal_treaty.hpp"
#include "oikumene/sim/war.hpp"
#include "oikumene/sim/war_campaign.hpp"
#include "oikumene/sim/war_target.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

[[nodiscard]] Color StatusColor(bool online);
[[nodiscard]] std::string StatusText(const HealthStatus& status);
[[nodiscard]] std::string Fixed(float value, int precision = 2);
[[nodiscard]] std::string Truncate(std::string text, std::size_t limit);

[[nodiscard]] int ActiveBandCount(const std::vector<Band>& bands);
[[nodiscard]] int VillageCount(const std::vector<Settlement>& settlements);
[[nodiscard]] int TotalPopulation(const std::vector<Band>& bands, const std::vector<Settlement>& settlements);

[[nodiscard]] const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id);
[[nodiscard]] const Settlement* SettlementAt(const std::vector<Settlement>& settlements, int x, int y);
[[nodiscard]] const Band* BandById(const std::vector<Band>& bands, int id);
[[nodiscard]] const Band* ActiveBandAt(const std::vector<Band>& bands, int x, int y);
[[nodiscard]] const Polity* PolityById(const std::vector<Polity>& polities, PolityId id);
[[nodiscard]] const Route* RouteById(const std::vector<Route>& routes, int id);
[[nodiscard]] const DiplomacyRelation* DiplomacyRelationForPair(const std::vector<DiplomacyRelation>& relations,
                                                                PolityId a, PolityId b);

[[nodiscard]] std::vector<const TradeAgreement*> ActiveTradesAtTile(const std::vector<TradeAgreement>& trades, int x,
                                                                    int y);
[[nodiscard]] std::string TradeAgreementLine(const TradeAgreement& trade);
[[nodiscard]] std::string DiplomacyRelationLine(const DiplomacyRelation& relation);
[[nodiscard]] std::string TradeIdsForPolity(const std::vector<TradeAgreement>& trades, PolityId polity_id);
[[nodiscard]] int DiplomacyPostureCount(const std::vector<DiplomacyRelation>& relations, DiplomaticPosture posture);
[[nodiscard]] std::vector<const DiplomacyRelation*>
DiplomacyRelationsForPolity(const std::vector<DiplomacyRelation>& relations, PolityId polity_id);

[[nodiscard]] std::string WarPressureLine(const WarPressure& pressure);
[[nodiscard]] std::vector<const WarPressure*> WarPressuresForActor(const std::vector<WarPressure>& pressures,
                                                                   PolityId polity_id);
[[nodiscard]] std::string WarTargetLine(const WarTargetCandidate& target);
[[nodiscard]] std::vector<const WarTargetCandidate*> WarTargetsForActor(const std::vector<WarTargetCandidate>& targets,
                                                                        PolityId polity_id);
[[nodiscard]] std::string WarCampaignLine(const WarCampaign& campaign);
[[nodiscard]] std::vector<const WarCampaign*> WarsForPolity(const std::vector<WarCampaign>& campaigns,
                                                            PolityId polity_id);
[[nodiscard]] std::string OccupationLine(const OccupationRecord& occupation);
[[nodiscard]] std::vector<const OccupationRecord*>
OccupationsForPolity(const std::vector<OccupationRecord>& occupations, PolityId polity_id);
[[nodiscard]] std::string VassalTreatyLine(const VassalTreaty& treaty);
[[nodiscard]] std::vector<const VassalTreaty*> VassalTreatiesForPolity(const std::vector<VassalTreaty>& treaties,
                                                                       PolityId polity_id);

[[nodiscard]] int ContestedTileCount(const World& world);
[[nodiscard]] int RouteTileCount(const World& world);
[[nodiscard]] int ActiveTradeCount(const std::vector<TradeAgreement>& trades);
[[nodiscard]] float TotalTradeProfit(const std::vector<TradeAgreement>& trades);
[[nodiscard]] float ControlledLandRatio(const World& world);
[[nodiscard]] int LargestPolityPopulation(const std::vector<Polity>& polities);
[[nodiscard]] float AverageUnlockedTechs(const std::vector<Polity>& polities);
[[nodiscard]] float TechUnlockRate(const std::vector<Polity>& polities, TechId tech);
[[nodiscard]] std::string TechList(const ResearchState& research, std::size_t limit = 4);
[[nodiscard]] std::string EffectsSummary(const TechEffects& effects);

} // namespace oikumene
