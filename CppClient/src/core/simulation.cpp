#include "oikumene/core/simulation.hpp"

#include <sstream>

#include "oikumene/sim/band_system.hpp"
#include "oikumene/sim/diplomacy_system.hpp"
#include "oikumene/sim/occupation_system.hpp"
#include "oikumene/sim/polity_system.hpp"
#include "oikumene/sim/route_system.hpp"
#include "oikumene/sim/settlement_system.hpp"
#include "oikumene/sim/technology_system.hpp"
#include "oikumene/sim/trade_system.hpp"
#include "oikumene/sim/war_planner.hpp"
#include "oikumene/sim/war_system.hpp"
#include "oikumene/sim/war_target_planner.hpp"
#include "oikumene/world/world_generation_params.hpp"
#include "oikumene/world/world_generator.hpp"

namespace oikumene {
namespace {

constexpr int kWarTargetRefreshInterval = 5;

} // namespace

Simulation::Simulation() : Simulation(WorldGenerator::Generate(WorldGenerationParams{}), SimulationParams{}) {}

Simulation::Simulation(World world, SimulationParams params) : world_(std::move(world)), params_(params) {}

Turn Simulation::CurrentTurn() const {
    return current_turn_;
}

const World& Simulation::GetWorld() const {
    return world_;
}

World& Simulation::MutableWorld() {
    return world_;
}

const std::vector<Band>& Simulation::Bands() const {
    return bands_;
}

std::vector<Band>& Simulation::Bands() {
    return bands_;
}

const std::vector<Settlement>& Simulation::Settlements() const {
    return settlements_;
}

std::vector<Settlement>& Simulation::Settlements() {
    return settlements_;
}

const std::vector<Polity>& Simulation::Polities() const {
    return polities_;
}

std::vector<Polity>& Simulation::Polities() {
    return polities_;
}

const std::vector<Route>& Simulation::Routes() const {
    return routes_;
}

std::vector<Route>& Simulation::Routes() {
    return routes_;
}

const std::vector<TradeAgreement>& Simulation::Trades() const {
    return trades_;
}

std::vector<TradeAgreement>& Simulation::Trades() {
    return trades_;
}

const std::vector<DiplomacyRelation>& Simulation::DiplomacyRelations() const {
    return diplomacy_relations_;
}

std::vector<DiplomacyRelation>& Simulation::DiplomacyRelations() {
    return diplomacy_relations_;
}

const std::vector<WarPressure>& Simulation::WarPressures() const {
    return war_pressures_;
}

std::vector<WarPressure>& Simulation::WarPressures() {
    return war_pressures_;
}

const std::vector<WarTargetCandidate>& Simulation::WarTargets() const {
    return war_target_candidates_;
}

std::vector<WarTargetCandidate>& Simulation::WarTargets() {
    return war_target_candidates_;
}

const std::vector<WarCampaign>& Simulation::Wars() const {
    return war_campaigns_;
}

std::vector<WarCampaign>& Simulation::Wars() {
    return war_campaigns_;
}

const std::vector<OccupationRecord>& Simulation::Occupations() const {
    return occupations_;
}

std::vector<OccupationRecord>& Simulation::Occupations() {
    return occupations_;
}

const EventLog& Simulation::Events() const {
    return event_log_;
}

EventLog& Simulation::Events() {
    return event_log_;
}

std::string Simulation::StatusSummary() const {
    std::ostringstream stream;
    int active_bands = 0;
    for (const auto& band : bands_) {
        active_bands += band.active ? 1 : 0;
    }
    int villages = 0;
    for (const auto& settlement : settlements_) {
        villages += settlement.level == SettlementLevel::Village ? 1 : 0;
    }
    int active_wars = 0;
    for (const auto& campaign : war_campaigns_) {
        active_wars += campaign.status == WarCampaignStatus::Active ? 1 : 0;
    }
    int active_occupations = 0;
    for (const auto& occupation : occupations_) {
        active_occupations += occupation.status == OccupationStatus::Active ? 1 : 0;
    }
    stream << "Turn " << current_turn_ << " bands " << active_bands << "/" << bands_.size() << " settlements "
           << settlements_.size() << " villages " << villages << " polities " << polities_.size() << " wars "
           << active_wars << " occ " << active_occupations;
    return stream.str();
}

void Simulation::InitializeBands(int count) {
    settlements_.clear();
    routes_.clear();
    trades_.clear();
    diplomacy_relations_.clear();
    war_pressures_.clear();
    war_target_candidates_.clear();
    war_campaigns_.clear();
    occupations_.clear();
    PolitySystem::Reset(world_, settlements_, polities_);
    RouteSystem::Reset(world_, routes_, polities_);
    TradeSystem::Reset(trades_, polities_);
    DiplomacySystem::Reset(diplomacy_relations_);
    WarSystem::Reset(war_campaigns_);
    OccupationSystem::Reset(occupations_, polities_);
    event_log_.Events().clear();
    current_turn_ = 0;
    BandSystem::InitializeBands(world_, params_, count, bands_);
}

void Simulation::AdvanceOneTurn() {
    BandSystem::UpdateBands(world_, params_, current_turn_, bands_, settlements_, event_log_);
    SettlementSystem::UpdateSettlements(world_, params_, current_turn_, settlements_, polities_, event_log_);
    PolitySystem::UpdatePolities(world_, params_, current_turn_, settlements_, polities_, event_log_);
    TechnologySystem::UpdateTechnologies(world_, current_turn_, settlements_, polities_, event_log_);
    RouteSystem::UpdateRoutes(world_, current_turn_, settlements_, polities_, routes_, event_log_,
                              params_.enable_routes);
    TradeSystem::UpdateTrades(world_, current_turn_, settlements_, polities_, trades_, event_log_);
    DiplomacySystem::UpdateDiplomacy(world_, current_turn_, polities_, trades_, diplomacy_relations_);
    war_pressures_ = BuildWarPressures(polities_, diplomacy_relations_);
    if (war_pressures_.empty()) {
        war_target_candidates_.clear();
    } else if (current_turn_ % kWarTargetRefreshInterval == 0) {
        war_target_candidates_ = BuildWarTargetCandidates(world_, settlements_, polities_, trades_, war_pressures_);
    }
    WarSystem::UpdateWars(world_, current_turn_, settlements_, polities_, trades_, war_target_candidates_,
                          war_campaigns_, event_log_);
    OccupationSystem::UpdateOccupations(world_, current_turn_, settlements_, polities_, war_campaigns_, occupations_,
                                        diplomacy_relations_, event_log_);
    ++current_turn_;
}

} // namespace oikumene
