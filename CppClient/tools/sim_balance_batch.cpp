#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "sim_balance_metrics.hpp"

namespace {

struct Options {
    std::uint64_t start_seed = 0;
    int count = 20;
    int width = 80;
    int height = 56;
    int bands = 8;
    int turns = 200;
    bool enable_routes = true;
    std::filesystem::path out = "../runs/sim_balance";
};

void PrintUsage() {
    std::cout << "usage: oikumene_sim_balance_batch [--start-seed N] [--count N] [--width N] [--height N]\n"
                 "                                  [--bands N] [--turns N] [--disable-routes] [--out PATH]\n";
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
        if (arg == "--start-seed" && NeedValue(argc, i)) {
            options.start_seed = static_cast<std::uint64_t>(std::stoull(argv[++i]));
            continue;
        }
        if (arg == "--count" && NeedValue(argc, i)) {
            options.count = std::stoi(argv[++i]);
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

    if (options.count <= 0 || options.width <= 0 || options.height <= 0 || options.bands <= 0 || options.turns < 0) {
        throw std::runtime_error("--count/--width/--height/--bands must be positive and --turns must be non-negative");
    }
    return options;
}

oikumene::tools::SimBalanceRunConfig ToRunConfig(const Options& options) {
    oikumene::tools::SimBalanceRunConfig config;
    config.width = options.width;
    config.height = options.height;
    config.bands = options.bands;
    config.turns = options.turns;
    config.enable_routes = options.enable_routes;
    return config;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Options options = ParseArgs(argc, argv);
        const auto config = ToRunConfig(options);
        std::filesystem::create_directories(options.out);

        std::vector<oikumene::tools::SimBalanceMetrics> all_metrics;
        all_metrics.reserve(static_cast<std::size_t>(options.count));

        std::ofstream csv(options.out / "summary.csv");
        if (!csv) {
            std::cerr << "failed to open " << options.out / "summary.csv" << '\n';
            return 1;
        }
        oikumene::tools::WriteCsvHeader(csv);

        nlohmann::json rows = nlohmann::json::array();
        for (int i = 0; i < options.count; ++i) {
            const auto seed = options.start_seed + static_cast<std::uint64_t>(i);
            auto metrics = oikumene::tools::RunOne(config, seed);
            oikumene::tools::WriteCsvRow(csv, metrics);
            rows.push_back(oikumene::tools::ToJson(metrics));
            all_metrics.push_back(metrics);
        }

        std::ofstream json(options.out / "summary.json");
        json << nlohmann::json{{"aggregate", oikumene::tools::Aggregate(all_metrics)}, {"rows", rows}}.dump(2) << '\n';

        std::cout << "wrote " << options.count << " sim balance reports to " << options.out << '\n';
        std::cout << "mean population: " << oikumene::tools::Aggregate(all_metrics)["mean_total_population"] << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        PrintUsage();
        return 1;
    }
}
