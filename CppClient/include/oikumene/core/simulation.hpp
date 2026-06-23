#pragma once

#include <string>
#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/band.hpp"
#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/route.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/simulation_params.hpp"
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
    EventLog event_log_;
};

}  // namespace oikumene
