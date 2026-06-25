#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

#include "oikumene/ai/strategic_report.hpp"
#include "oikumene/core/simulation.hpp"
#include "oikumene/io/simulation_json.hpp"
#include "oikumene/world/world_generation_report.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

struct Options {
    std::uint64_t seed = 42;
    int width = 80;
    int height = 56;
    int bands = 8;
    int turns = 200;
    int sample_every = 0;
    bool enable_routes = true;
    std::filesystem::path out = "../runs/sim_batch";
};

void PrintUsage() {
    std::cout << "usage: oikumene_sim_batch [--seed N] [--width N] [--height N] [--bands N] [--turns N]"
                 " [--sample-every N] [--disable-routes] [--out PATH]\n";
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
        if (arg == "--sample-every" && NeedValue(argc, i)) {
            options.sample_every = std::stoi(argv[++i]);
            continue;
        }
        if (arg == "--disable-routes") {
            options.enable_routes = false;
            continue;
        }
        if (arg == "--out" && NeedValue(argc, i)) {
            options.out = argv[++i];
            continue;
        }
        throw std::runtime_error("Unknown or incomplete argument: " + arg);
    }

    if (options.width <= 0 || options.height <= 0 || options.bands <= 0 || options.turns < 0 ||
        options.sample_every < 0) {
        throw std::runtime_error(
            "--width/--height/--bands must be positive and --turns/--sample-every must be non-negative");
    }
    return options;
}

oikumene::SimulationRunMetadata ToMetadata(const Options& options) {
    return oikumene::SimulationRunMetadata{
        .seed = options.seed,
        .width = options.width,
        .height = options.height,
        .turns = options.turns,
        .initial_bands = options.bands,
        .routes_enabled = options.enable_routes,
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
        output << oikumene::ToJson(event).dump() << '\n';
    }
}

void WriteStateSample(std::ofstream& output, const oikumene::Simulation& sim) {
    output << oikumene::StateSampleToJson(sim).dump() << '\n';
}

} // namespace

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
        sim_params.enable_routes = options.enable_routes;

        auto world = oikumene::WorldGenerator::Generate(world_params);
        const auto report = oikumene::BuildWorldGenerationReport(world);

        oikumene::Simulation sim(std::move(world), sim_params);
        sim.InitializeBands(options.bands);

        std::ofstream states;
        if (options.sample_every > 0) {
            states.open(options.out / "states.jsonl");
            if (!states) {
                throw std::runtime_error("failed to open " + (options.out / "states.jsonl").string());
            }
            WriteStateSample(states, sim);
        }

        for (int i = 0; i < options.turns; ++i) {
            sim.AdvanceOneTurn();
            if (options.sample_every > 0 && sim.CurrentTurn() % options.sample_every == 0) {
                WriteStateSample(states, sim);
            }
        }
        if (options.sample_every > 0 && sim.CurrentTurn() % options.sample_every != 0) {
            WriteStateSample(states, sim);
        }

        WriteJson(options.out / "world_report.json", oikumene::ToJson(report));
        WriteJson(options.out / "final_state.json", oikumene::FinalStateToJson(sim));
        WriteJson(options.out / "summary.json", oikumene::SummaryToJson(ToMetadata(options), sim));
        WriteJson(options.out / "strategic_reports.json", oikumene::StrategicReportsToJson(sim));
        WriteJson(options.out / "decision_batch.json", oikumene::BuildDecisionBatchRequest(sim, sim.CurrentTurn() / 5));
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
