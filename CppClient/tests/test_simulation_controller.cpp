#include <cassert>
#include <iostream>

#include "oikumene/app/simulation_controller.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

oikumene::Simulation MakeSimulation() {
    oikumene::WorldGenerationParams params;
    params.seed = 42;
    auto world = oikumene::WorldGenerator::Generate(params);
    oikumene::Simulation sim(std::move(world), oikumene::SimulationParams{});
    sim.InitializeBands(8);
    return sim;
}

void TestPausedDoesNotAdvance() {
    using namespace oikumene;

    auto sim = MakeSimulation();
    SimulationController controller;
    controller.SetRunning(false);
    const auto before = sim.CurrentTurn();
    const int advanced = controller.Update(10.0F, sim);
    assert(advanced == 0);
    assert(sim.CurrentTurn() == before);
}

void TestStepOneAdvancesExactlyOneTurn() {
    using namespace oikumene;

    auto sim = MakeSimulation();
    SimulationController controller;
    controller.StepTurns(sim, 1);
    assert(sim.CurrentTurn() == 1);
}

void TestStepTenAdvancesExactlyTenTurns() {
    using namespace oikumene;

    auto sim = MakeSimulation();
    SimulationController controller;
    controller.StepTurns(sim, 10);
    assert(sim.CurrentTurn() == 10);
}

void TestAutoRunAdvancesAccordingToTPS() {
    using namespace oikumene;

    auto sim = MakeSimulation();
    SimulationController controller;
    controller.SetTurnsPerSecond(4.0F);
    controller.SetRunning(true);
    const int advanced = controller.Update(1.0F, sim);
    assert(advanced == 4);
    assert(sim.CurrentTurn() == 4);
}

void TestChangingTPSChangesAdvanceRate() {
    using namespace oikumene;

    auto sim = MakeSimulation();
    SimulationController controller;
    controller.SetRunning(true);
    controller.SetTurnsPerSecond(2.0F);
    assert(controller.Update(1.0F, sim) == 2);
    controller.SetTurnsPerSecond(8.0F);
    assert(controller.Update(1.0F, sim) == 8);
    assert(sim.CurrentTurn() == 10);
}

}  // namespace

int main() {
    TestPausedDoesNotAdvance();
    TestStepOneAdvancesExactlyOneTurn();
    TestStepTenAdvancesExactlyTenTurns();
    TestAutoRunAdvancesAccordingToTPS();
    TestChangingTPSChangesAdvanceRate();

    std::cout << "oikumene_simulation_controller_tests passed\n";
    return 0;
}
