#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <nlohmann/json.hpp>

#include "oikumene/core/simulation.hpp"
#include "oikumene/sim/band.hpp"
#include "oikumene/sim/diplomacy.hpp"
#include "oikumene/sim/event.hpp"
#include "oikumene/sim/event_log.hpp"
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

namespace oikumene {

struct SimulationRunMetadata {
    std::uint64_t seed = 0;
    int width = 0;
    int height = 0;
    int turns = 0;
    int initial_bands = 0;
    bool routes_enabled = true;
};

[[nodiscard]] nlohmann::json StockpileToJson(const Stockpile& stockpile);
[[nodiscard]] nlohmann::json ToJson(const SimEvent& event);
[[nodiscard]] nlohmann::json BandToJson(const Band& band);
[[nodiscard]] nlohmann::json SettlementToJson(const Settlement& settlement);
[[nodiscard]] nlohmann::json PolityBudgetToJson(const PolityBudget& budget);
[[nodiscard]] nlohmann::json TechListToJson(const std::vector<TechId>& techs);
[[nodiscard]] nlohmann::json TechEffectsToJson(const TechEffects& effects);
[[nodiscard]] nlohmann::json ResearchToJson(const Polity& polity);
[[nodiscard]] nlohmann::json PolityToJson(const Polity& polity);
[[nodiscard]] nlohmann::json RouteToJson(const Route& route);
[[nodiscard]] nlohmann::json TradeToJson(const TradeAgreement& trade);
[[nodiscard]] nlohmann::json DiplomacyToJson(const DiplomacyRelation& relation);
[[nodiscard]] nlohmann::json WarPressureToJson(const WarPressure& pressure);
[[nodiscard]] nlohmann::json WarTargetToJson(const WarTargetCandidate& target);
[[nodiscard]] nlohmann::json WarCampaignToJson(const WarCampaign& campaign);
[[nodiscard]] nlohmann::json OccupationToJson(const OccupationRecord& occupation);
[[nodiscard]] nlohmann::json VassalTreatyToJson(const VassalTreaty& treaty);
[[nodiscard]] nlohmann::json ImprovedTilesToJson(const Simulation& sim);
[[nodiscard]] nlohmann::json RouteTilesToJson(const Simulation& sim);
[[nodiscard]] nlohmann::json FinalStateToJson(const Simulation& sim);
[[nodiscard]] nlohmann::json SummaryToJson(const SimulationRunMetadata& metadata, const Simulation& sim);
[[nodiscard]] nlohmann::json StateSampleToJson(const Simulation& sim);
[[nodiscard]] nlohmann::json StrategicReportsToJson(const Simulation& sim);
[[nodiscard]] nlohmann::json RecentEventsToJson(const EventLog& events, std::size_t max_count);

} // namespace oikumene
