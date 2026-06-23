#pragma once

#include <string>

#include "oikumene/core/types.hpp"

namespace oikumene {

class Simulation {
public:
    Simulation();

    [[nodiscard]] Turn CurrentTurn() const;
    [[nodiscard]] std::string StatusSummary() const;

    void AdvanceOneTurn();

private:
    Turn current_turn_ = 0;
};

}  // namespace oikumene
