#include "oikumene/app/simulation_controller.hpp"

#include <algorithm>

namespace oikumene {

void SimulationController::SetRunning(bool running) {
    running_ = running;
}

void SimulationController::ToggleRunning() {
    running_ = !running_;
}

void SimulationController::StepTurns(Simulation& sim, int turns) {
    const int clamped_turns = std::max(0, turns);
    for (int i = 0; i < clamped_turns; ++i) {
        sim.AdvanceOneTurn();
    }
}

int SimulationController::Update(float delta_seconds, Simulation& sim) {
    if (!running_) {
        return 0;
    }

    accumulator_ += std::max(0.0F, delta_seconds) * turns_per_second_;
    const int turns = std::min(25, static_cast<int>(accumulator_));
    if (turns <= 0) {
        return 0;
    }

    accumulator_ -= static_cast<float>(turns);
    StepTurns(sim, turns);
    return turns;
}

void SimulationController::SetTurnsPerSecond(float turns_per_second) {
    turns_per_second_ = std::clamp(turns_per_second, 0.25F, 64.0F);
}

bool SimulationController::IsRunning() const {
    return running_;
}

float SimulationController::TurnsPerSecond() const {
    return turns_per_second_;
}

float SimulationController::Accumulator() const {
    return accumulator_;
}

} // namespace oikumene
