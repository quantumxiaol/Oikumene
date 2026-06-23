#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

#include "oikumene/core/simulation.hpp"
#include "oikumene/sim/event.hpp"
#include "oikumene/world/world_generation_report.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

struct Options {
    std::uint64_t seed = 42;
    int width = 80;
    int height = 56;
    int bands = 8;
    int turns = 200;
    std::filesystem::path out = "../runs/sim_batch";
};

void PrintUsage() {
    std::cout << "usage: oikumene_sim_batch [--seed N] [--width N] [--height N] [--bands N] [--turns N]"
                 " [--out PATH]\n";
}

bool NeedValue(int argc, int index) {
    return index + 1 < argc;
}

Options ParseArgs(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            std::exit(0);
        }
        if (arg == "--seed" && NeedValue(argc, i)) {
            options.seed = static_cast<std::uint64_t>(std::stoull(argv[++i]));
            continue;
        }
        if (arg == "--width" && NeedValue(argc, i)) {
            options.width = std::stoi(argv[++i]);
            continue;
        }
        if (arg == "--height" && NeedValue(argc, i)) {
            options.height = std::stoi(argv[++i]);
            continue;
        }
        if (arg == "--bands" && NeedValue(argc, i)) {
            options.bands = std::stoi(argv[++i]);
            continue;
        }
        if (arg == "--turns" && NeedValue(argc, i)) {
            options.turns = std::stoi(argv[++i]);
            continue;
        }
        if (arg == "--out" && NeedValue(argc, i)) {
            options.out = argv[++i];
            continue;
        }
        throw std::runtime_error("Unknown or incomplete argument: " + arg);
    }

    if (options.width <= 0 || options.height <= 0 || options.bands <= 0 || options.turns < 0) {
        throw std::runtime_error("--width/--height/--bands must be positive and --turns must be non-negative");
    }
    return options;
}

nlohmann::json StockpileToJson(const oikumene::Stockpile& stockpile) {
    return nlohmann::json{
        {"food", stockpile.food},
        {"wood", stockpile.wood},
        {"ore", stockpile.ore},
        {"metal", stockpile.metal},
        {"wealth", stockpile.wealth},
    };
}

nlohmann::json EventToJson(const oikumene::SimEvent& event) {
    return nlohmann::json{
        {"turn", event.turn},
        {"type", oikumene::ToString(event.type)},
        {"actor_id", event.actor_id},
        {"x", event.x},
        {"y", event.y},
        {"summary", event.summary},
    };
}

nlohmann::json BandToJson(const oikumene::Band& band) {
    return nlohmann::json{
        {"id", band.id},
        {"x", band.x},
        {"y", band.y},
        {"population", band.population},
        {"food", band.food},
        {"target_x", band.target_x},
        {"target_y", band.target_y},
        {"turns_alive", band.turns_alive},
        {"state", oikumene::ToString(band.state)},
        {"active", band.active},
    };
}

nlohmann::json SettlementToJson(const oikumene::Settlement& settlement) {
    return nlohmann::json{
        {"id", settlement.id},
        {"x", settlement.x},
        {"y", settlement.y},
        {"founder_band_id", settlement.founder_band_id},
        {"population", settlement.population},
        {"level", oikumene::ToString(settlement.level)},
        {"stockpile", StockpileToJson(settlement.stockpile)},
        {"turns_since_founded", settlement.turns_since_founded},
    };
}

int CountActiveBands(const oikumene::Simulation& sim) {
    int count = 0;
    for (const auto& band : sim.Bands()) {
        count += band.active ? 1 : 0;
    }
    return count;
}

int CountVillages(const oikumene::Simulation& sim) {
    int count = 0;
    for (const auto& settlement : sim.Settlements()) {
        count += settlement.level == oikumene::SettlementLevel::Village ? 1 : 0;
    }
    return count;
}

int CountEvents(const oikumene::Simulation& sim, oikumene::EventType type) {
    int count = 0;
    for (const auto& event : sim.Events().Events()) {
        count += event.type == type ? 1 : 0;
    }
    return count;
}

int TotalPopulation(const oikumene::Simulation& sim) {
    int total = 0;
    for (const auto& band : sim.Bands()) {
        if (band.active) {
            total += band.population;
        }
    }
    for (const auto& settlement : sim.Settlements()) {
        total += settlement.population;
    }
    return total;
}

nlohmann::json FinalStateToJson(const oikumene::Simulation& sim) {
    nlohmann::json bands = nlohmann::json::array();
    for (const auto& band : sim.Bands()) {
        bands.push_back(BandToJson(band));
    }

    nlohmann::json settlements = nlohmann::json::array();
    for (const auto& settlement : sim.Settlements()) {
        settlements.push_back(SettlementToJson(settlement));
    }

    return nlohmann::json{
        {"turn", sim.CurrentTurn()},
        {"status", sim.StatusSummary()},
        {"active_bands", CountActiveBands(sim)},
        {"settlements", settlements},
        {"bands", bands},
        {"event_count", sim.Events().Size()},
    };
}

nlohmann::json SummaryToJson(const Options& options, const oikumene::Simulation& sim) {
    return nlohmann::json{
        {"seed", options.seed},
        {"width", options.width},
        {"height", options.height},
        {"turns", options.turns},
        {"initial_bands", options.bands},
        {"active_bands", CountActiveBands(sim)},
        {"settlements", sim.Settlements().size()},
        {"villages", CountVillages(sim)},
        {"population_total", TotalPopulation(sim)},
        {"event_count", sim.Events().Size()},
        {"migration_events", CountEvents(sim, oikumene::EventType::BandMigrated)},
        {"settlement_founded_events", CountEvents(sim, oikumene::EventType::SettlementFounded)},
        {"population_growth_events", CountEvents(sim, oikumene::EventType::PopulationGrowth)},
        {"famine_events", CountEvents(sim, oikumene::EventType::Famine)},
        {"village_upgrade_events", CountEvents(sim, oikumene::EventType::SettlementUpgraded)},
    };
}

void WriteJson(const std::filesystem::path& path, const nlohmann::json& json) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("failed to open " + path.string());
    }
    output << json.dump(2) << '\n';
}

void WriteEventsJsonl(const std::filesystem::path& path, const oikumene::Simulation& sim) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("failed to open " + path.string());
    }
    for (const auto& event : sim.Events().Events()) {
        output << EventToJson(event).dump() << '\n';
    }
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const Options options = ParseArgs(argc, argv);
        std::filesystem::create_directories(options.out);

        oikumene::WorldGenerationParams world_params;
        world_params.seed = options.seed;
        world_params.width = options.width;
        world_params.height = options.height;

        oikumene::SimulationParams sim_params;
        sim_params.initial_band_count = options.bands;

        auto world = oikumene::WorldGenerator::Generate(world_params);
        const auto report = oikumene::BuildWorldGenerationReport(world);

        oikumene::Simulation sim(std::move(world), sim_params);
        sim.InitializeBands(options.bands);
        for (int i = 0; i < options.turns; ++i) {
            sim.AdvanceOneTurn();
        }

        WriteJson(options.out / "world_report.json", oikumene::ToJson(report));
        WriteJson(options.out / "final_state.json", FinalStateToJson(sim));
        WriteJson(options.out / "summary.json", SummaryToJson(options, sim));
        WriteEventsJsonl(options.out / "events.jsonl", sim);

        std::cout << "wrote sim batch to " << options.out << '\n';
        std::cout << sim.StatusSummary() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        PrintUsage();
        return 1;
    }
}
