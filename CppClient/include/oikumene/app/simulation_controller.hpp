#pragma once

#include "oikumene/core/simulation.hpp"

namespace oikumene {

class SimulationController {
public:
    void SetRunning(bool running);
    void ToggleRunning();
    void StepTurns(Simulation& sim, int turns);
    int Update(float delta_seconds, Simulation& sim);

    void SetTurnsPerSecond(float turns_per_second);
    [[nodiscard]] bool IsRunning() const;
    [[nodiscard]] float TurnsPerSecond() const;
    [[nodiscard]] float Accumulator() const;

private:
    bool running_ = false;
    float turns_per_second_ = 4.0F;
    float accumulator_ = 0.0F;
};

}  // namespace oikumene
