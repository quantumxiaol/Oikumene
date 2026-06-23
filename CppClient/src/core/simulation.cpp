#include "oikumene/core/simulation.hpp"

#include <sstream>

#include "oikumene/sim/band_system.hpp"
#include "oikumene/sim/polity_system.hpp"
#include "oikumene/sim/route_system.hpp"
#include "oikumene/sim/settlement_system.hpp"
#include "oikumene/sim/technology_system.hpp"
#include "oikumene/world/world_generation_params.hpp"
#include "oikumene/world/world_generator.hpp"

namespace oikumene {

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
    stream << "Turn " << current_turn_ << " bands " << active_bands << "/" << bands_.size() << " settlements "
           << settlements_.size() << " villages " << villages << " polities " << polities_.size();
    return stream.str();
}

void Simulation::InitializeBands(int count) {
    settlements_.clear();
    routes_.clear();
    PolitySystem::Reset(world_, settlements_, polities_);
    RouteSystem::Reset(world_, routes_, polities_);
    event_log_.Events().clear();
    current_turn_ = 0;
    BandSystem::InitializeBands(world_, params_, count, bands_);
}

void Simulation::AdvanceOneTurn() {
    BandSystem::UpdateBands(world_, params_, current_turn_, bands_, settlements_, event_log_);
    SettlementSystem::UpdateSettlements(world_, params_, current_turn_, settlements_, polities_, event_log_);
    PolitySystem::UpdatePolities(world_, current_turn_, settlements_, polities_, event_log_);
    TechnologySystem::UpdateTechnologies(world_, current_turn_, settlements_, polities_, event_log_);
    RouteSystem::UpdateRoutes(world_, current_turn_, settlements_, polities_, routes_, event_log_);
    ++current_turn_;
}

}  // namespace oikumene
