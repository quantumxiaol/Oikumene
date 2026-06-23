#include "oikumene/core/simulation.hpp"

#include <sstream>

namespace oikumene {

Simulation::Simulation() = default;

Turn Simulation::CurrentTurn() const {
    return current_turn_;
}

std::string Simulation::StatusSummary() const {
    std::ostringstream stream;
    stream << "Turn " << current_turn_ << " - Phase 0 scaffold";
    return stream.str();
}

void Simulation::AdvanceOneTurn() {
    ++current_turn_;
}

}  // namespace oikumene
