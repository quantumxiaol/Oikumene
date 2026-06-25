#pragma once

#include <string>
#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/band.hpp"
#include "oikumene/sim/diplomacy.hpp"
#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/occupation.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/route.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/simulation_params.hpp"
#include "oikumene/sim/trade.hpp"
#include "oikumene/sim/vassal_treaty.hpp"
#include "oikumene/sim/war.hpp"
#include "oikumene/sim/war_campaign.hpp"
#include "oikumene/sim/war_target.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

class Simulation {
  public:
    Simulation();
    explicit Simulation(World world, SimulationParams params = {});

    [[nodiscard]] Turn CurrentTurn() const;
    [[nodiscard]] const World& GetWorld() const;
    [[nodiscard]] World& MutableWorld();
    [[nodiscard]] const std::vector<Band>& Bands() const;
    [[nodiscard]] std::vector<Band>& Bands();
    [[nodiscard]] const std::vector<Settlement>& Settlements() const;
    [[nodiscard]] std::vector<Settlement>& Settlements();
    [[nodiscard]] const std::vector<Polity>& Polities() const;
    [[nodiscard]] std::vector<Polity>& Polities();
    [[nodiscard]] const std::vector<Route>& Routes() const;
    [[nodiscard]] std::vector<Route>& Routes();
    [[nodiscard]] const std::vector<TradeAgreement>& Trades() const;
    [[nodiscard]] std::vector<TradeAgreement>& Trades();
    [[nodiscard]] const std::vector<DiplomacyRelation>& DiplomacyRelations() const;
    [[nodiscard]] std::vector<DiplomacyRelation>& DiplomacyRelations();
    [[nodiscard]] const std::vector<WarPressure>& WarPressures() const;
    [[nodiscard]] std::vector<WarPressure>& WarPressures();
    [[nodiscard]] const std::vector<WarTargetCandidate>& WarTargets() const;
    [[nodiscard]] std::vector<WarTargetCandidate>& WarTargets();
    [[nodiscard]] const std::vector<WarCampaign>& Wars() const;
    [[nodiscard]] std::vector<WarCampaign>& Wars();
    [[nodiscard]] const std::vector<OccupationRecord>& Occupations() const;
    [[nodiscard]] std::vector<OccupationRecord>& Occupations();
    [[nodiscard]] const std::vector<VassalTreaty>& VassalTreaties() const;
    [[nodiscard]] std::vector<VassalTreaty>& VassalTreaties();
    [[nodiscard]] const EventLog& Events() const;
    [[nodiscard]] EventLog& Events();
    [[nodiscard]] std::string StatusSummary() const;

    void InitializeBands(int count);
    void AdvanceOneTurn();

  private:
    World world_;
    SimulationParams params_;
    Turn current_turn_ = 0;
    std::vector<Band> bands_;
    std::vector<Settlement> settlements_;
    std::vector<Polity> polities_;
    std::vector<Route> routes_;
    std::vector<TradeAgreement> trades_;
    std::vector<DiplomacyRelation> diplomacy_relations_;
    std::vector<WarPressure> war_pressures_;
    std::vector<WarTargetCandidate> war_target_candidates_;
    std::vector<WarCampaign> war_campaigns_;
    std::vector<OccupationRecord> occupations_;
    std::vector<VassalTreaty> vassal_treaties_;
    EventLog event_log_;
};

} // namespace oikumene
