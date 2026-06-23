#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "oikumene/core/simulation.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

struct Options {
    std::uint64_t start_seed = 0;
    int count = 20;
    int width = 80;
    int height = 56;
    int bands = 8;
    int turns = 200;
    std::filesystem::path out = "../runs/sim_balance";
};

struct Metrics {
    std::uint64_t seed = 0;
    int settlements = 0;
    int villages = 0;
    int camps = 0;
    int active_bands = 0;
    int total_population = 0;
    int max_settlement_population = 0;
    int farms = 0;
    int lumber_camps = 0;
    int pastures = 0;
    int shallow_mines = 0;
    int worked_tiles = 0;
    int polities = 0;
    int contested_tiles = 0;
    int largest_polity_population = 0;
    int famine_events = 0;
    int farm_events = 0;
    int lumber_events = 0;
    int pasture_events = 0;
    float total_food_output = 0.0F;
    float total_food_consumption = 0.0F;
    float total_wood_output = 0.0F;
    float average_carrying_capacity = 0.0F;
    float food_output_consumption_ratio = 0.0F;
    float farm_share_of_worked_tiles = 0.0F;
    float controlled_land_ratio = 0.0F;
    float average_member_settlements_per_polity = 0.0F;
    float polity_formation_turn_mean = 0.0F;
};

void PrintUsage() {
    std::cout << "usage: oikumene_sim_balance_batch [--start-seed N] [--count N] [--width N] [--height N]\n"
                 "                                  [--bands N] [--turns N] [--out PATH]\n";
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

int CountEvents(const oikumene::Simulation& sim, oikumene::EventType type) {
    int count = 0;
    for (const auto& event : sim.Events().Events()) {
        count += event.type == type ? 1 : 0;
    }
    return count;
}

int CountImprovement(const oikumene::Simulation& sim, oikumene::ImprovementKind kind) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += tile.improvement == kind ? 1 : 0;
    }
    return count;
}

int CountLandTiles(const oikumene::Simulation& sim) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += !tile.is_ocean && !tile.is_lake ? 1 : 0;
    }
    return count;
}

Metrics RunOne(const Options& options, std::uint64_t seed) {
    oikumene::WorldGenerationParams world_params;
    world_params.seed = seed;
    world_params.width = options.width;
    world_params.height = options.height;

    oikumene::SimulationParams sim_params;
    sim_params.initial_band_count = options.bands;

    oikumene::Simulation sim(oikumene::WorldGenerator::Generate(world_params), sim_params);
    sim.InitializeBands(options.bands);
    for (int i = 0; i < options.turns; ++i) {
        sim.AdvanceOneTurn();
    }

    Metrics metrics;
    metrics.seed = seed;
    metrics.settlements = static_cast<int>(sim.Settlements().size());
    for (const auto& band : sim.Bands()) {
        metrics.active_bands += band.active ? 1 : 0;
        metrics.total_population += band.active ? band.population : 0;
    }
    float carrying_capacity_sum = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        metrics.villages += settlement.level == oikumene::SettlementLevel::Village ? 1 : 0;
        metrics.total_population += settlement.population;
        metrics.max_settlement_population = std::max(metrics.max_settlement_population, settlement.population);
        metrics.total_food_output += settlement.local_food_output_last_turn;
        metrics.total_food_consumption += settlement.food_consumption_last_turn;
        metrics.total_wood_output += settlement.local_wood_output_last_turn;
        carrying_capacity_sum += settlement.carrying_capacity;
    }
    metrics.camps = metrics.settlements - metrics.villages;
    metrics.average_carrying_capacity =
        metrics.settlements <= 0 ? 0.0F : carrying_capacity_sum / static_cast<float>(metrics.settlements);

    metrics.farms = CountImprovement(sim, oikumene::ImprovementKind::Farm);
    metrics.lumber_camps = CountImprovement(sim, oikumene::ImprovementKind::LumberCamp);
    metrics.pastures = CountImprovement(sim, oikumene::ImprovementKind::Pasture);
    metrics.shallow_mines = CountImprovement(sim, oikumene::ImprovementKind::ShallowMine);
    int controlled_land_tiles = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        metrics.worked_tiles += tile.worked_by_settlement_id >= 0 ? 1 : 0;
        controlled_land_tiles += tile.controller_polity_id != oikumene::kInvalidPolityId && !tile.is_ocean && !tile.is_lake ? 1 : 0;
        metrics.contested_tiles += tile.is_contested ? 1 : 0;
    }
    const int land_tiles = CountLandTiles(sim);
    metrics.controlled_land_ratio =
        land_tiles <= 0 ? 0.0F : static_cast<float>(controlled_land_tiles) / static_cast<float>(land_tiles);
    metrics.polities = static_cast<int>(sim.Polities().size());
    int member_sum = 0;
    float formation_turn_sum = 0.0F;
    for (const auto& polity : sim.Polities()) {
        metrics.largest_polity_population = std::max(metrics.largest_polity_population, polity.population);
        member_sum += static_cast<int>(polity.member_settlement_ids.size());
        formation_turn_sum += static_cast<float>(polity.founded_turn);
    }
    metrics.average_member_settlements_per_polity =
        metrics.polities <= 0 ? 0.0F : static_cast<float>(member_sum) / static_cast<float>(metrics.polities);
    metrics.polity_formation_turn_mean =
        metrics.polities <= 0 ? 0.0F : formation_turn_sum / static_cast<float>(metrics.polities);
    metrics.famine_events = CountEvents(sim, oikumene::EventType::Famine);
    metrics.farm_events = CountEvents(sim, oikumene::EventType::FarmBuilt);
    metrics.lumber_events = CountEvents(sim, oikumene::EventType::LumberCampBuilt);
    metrics.pasture_events = CountEvents(sim, oikumene::EventType::PastureBuilt);
    metrics.food_output_consumption_ratio =
        metrics.total_food_output / std::max(1.0F, metrics.total_food_consumption);
    metrics.farm_share_of_worked_tiles =
        metrics.worked_tiles <= 0 ? 0.0F : static_cast<float>(metrics.farms) / static_cast<float>(metrics.worked_tiles);
    return metrics;
}

nlohmann::json ToJson(const Metrics& metrics) {
    return nlohmann::json{
        {"seed", metrics.seed},
        {"settlements", metrics.settlements},
        {"villages", metrics.villages},
        {"camps", metrics.camps},
        {"active_bands", metrics.active_bands},
        {"total_population", metrics.total_population},
        {"max_settlement_population", metrics.max_settlement_population},
        {"farm_count", metrics.farms},
        {"lumbercamp_count", metrics.lumber_camps},
        {"pasture_count", metrics.pastures},
        {"shallow_mine_count", metrics.shallow_mines},
        {"worked_tile_count", metrics.worked_tiles},
        {"polities", metrics.polities},
        {"controlled_land_ratio", metrics.controlled_land_ratio},
        {"contested_tiles", metrics.contested_tiles},
        {"largest_polity_population", metrics.largest_polity_population},
        {"average_member_settlements_per_polity", metrics.average_member_settlements_per_polity},
        {"polity_formation_turn_mean", metrics.polity_formation_turn_mean},
        {"total_food_output_last_turn", metrics.total_food_output},
        {"total_food_consumption_last_turn", metrics.total_food_consumption},
        {"total_wood_output_last_turn", metrics.total_wood_output},
        {"average_carrying_capacity", metrics.average_carrying_capacity},
        {"food_output_consumption_ratio", metrics.food_output_consumption_ratio},
        {"farm_share_of_worked_tiles", metrics.farm_share_of_worked_tiles},
        {"famine_events", metrics.famine_events},
        {"farm_built_events", metrics.farm_events},
        {"lumbercamp_built_events", metrics.lumber_events},
        {"pasture_built_events", metrics.pasture_events},
    };
}

void WriteCsvHeader(std::ofstream& output) {
    output << "seed,settlements,villages,camps,active_bands,total_population,max_settlement_population,"
              "farm_count,lumbercamp_count,pasture_count,shallow_mine_count,worked_tile_count,"
              "polities,controlled_land_ratio,contested_tiles,largest_polity_population,"
              "average_member_settlements_per_polity,polity_formation_turn_mean,"
              "total_food_output_last_turn,total_food_consumption_last_turn,total_wood_output_last_turn,"
              "average_carrying_capacity,food_output_consumption_ratio,farm_share_of_worked_tiles,"
              "famine_events,farm_built_events,lumbercamp_built_events,pasture_built_events\n";
}

void WriteCsvRow(std::ofstream& output, const Metrics& metrics) {
    output << metrics.seed << ',' << metrics.settlements << ',' << metrics.villages << ',' << metrics.camps << ','
           << metrics.active_bands << ',' << metrics.total_population << ',' << metrics.max_settlement_population
           << ',' << metrics.farms << ',' << metrics.lumber_camps << ',' << metrics.pastures << ','
           << metrics.shallow_mines << ',' << metrics.worked_tiles << ',' << metrics.polities << ','
           << metrics.controlled_land_ratio << ',' << metrics.contested_tiles << ','
           << metrics.largest_polity_population << ',' << metrics.average_member_settlements_per_polity << ','
           << metrics.polity_formation_turn_mean << ',' << metrics.total_food_output << ','
           << metrics.total_food_consumption << ',' << metrics.total_wood_output << ','
           << metrics.average_carrying_capacity << ',' << metrics.food_output_consumption_ratio << ','
           << metrics.farm_share_of_worked_tiles << ',' << metrics.famine_events << ',' << metrics.farm_events << ','
           << metrics.lumber_events << ',' << metrics.pasture_events << '\n';
}

nlohmann::json Aggregate(const std::vector<Metrics>& metrics) {
    auto mean = [&](auto getter) {
        float total = 0.0F;
        for (const auto& item : metrics) {
            total += static_cast<float>(getter(item));
        }
        return metrics.empty() ? 0.0F : total / static_cast<float>(metrics.size());
    };
    auto min_value = [&](auto getter) {
        if (metrics.empty()) {
            return 0.0F;
        }
        float value = static_cast<float>(getter(metrics.front()));
        for (const auto& item : metrics) {
            value = std::min(value, static_cast<float>(getter(item)));
        }
        return value;
    };
    auto max_value = [&](auto getter) {
        if (metrics.empty()) {
            return 0.0F;
        }
        float value = static_cast<float>(getter(metrics.front()));
        for (const auto& item : metrics) {
            value = std::max(value, static_cast<float>(getter(item)));
        }
        return value;
    };

    return nlohmann::json{
        {"count", metrics.size()},
        {"mean_total_population", mean([](const Metrics& item) { return item.total_population; })},
        {"min_total_population", min_value([](const Metrics& item) { return item.total_population; })},
        {"max_total_population", max_value([](const Metrics& item) { return item.total_population; })},
        {"mean_max_settlement_population", mean([](const Metrics& item) { return item.max_settlement_population; })},
        {"mean_settlements", mean([](const Metrics& item) { return item.settlements; })},
        {"mean_villages", mean([](const Metrics& item) { return item.villages; })},
        {"mean_farms", mean([](const Metrics& item) { return item.farms; })},
        {"mean_lumbercamps", mean([](const Metrics& item) { return item.lumber_camps; })},
        {"mean_pastures", mean([](const Metrics& item) { return item.pastures; })},
        {"mean_shallow_mines", mean([](const Metrics& item) { return item.shallow_mines; })},
        {"mean_worked_tiles", mean([](const Metrics& item) { return item.worked_tiles; })},
        {"mean_polities", mean([](const Metrics& item) { return item.polities; })},
        {"mean_controlled_land_ratio", mean([](const Metrics& item) { return item.controlled_land_ratio; })},
        {"mean_contested_tiles", mean([](const Metrics& item) { return item.contested_tiles; })},
        {"mean_largest_polity_population", mean([](const Metrics& item) { return item.largest_polity_population; })},
        {"mean_member_settlements_per_polity",
         mean([](const Metrics& item) { return item.average_member_settlements_per_polity; })},
        {"mean_polity_formation_turn", mean([](const Metrics& item) { return item.polity_formation_turn_mean; })},
        {"mean_food_output", mean([](const Metrics& item) { return item.total_food_output; })},
        {"mean_food_consumption", mean([](const Metrics& item) { return item.total_food_consumption; })},
        {"mean_food_output_consumption_ratio",
         mean([](const Metrics& item) { return item.food_output_consumption_ratio; })},
        {"mean_farm_share_of_worked_tiles", mean([](const Metrics& item) { return item.farm_share_of_worked_tiles; })},
        {"mean_wood_output", mean([](const Metrics& item) { return item.total_wood_output; })},
        {"mean_carrying_capacity", mean([](const Metrics& item) { return item.average_carrying_capacity; })},
        {"mean_famine_events", mean([](const Metrics& item) { return item.famine_events; })},
        {"mean_farm_built_events", mean([](const Metrics& item) { return item.farm_events; })},
        {"mean_lumbercamp_built_events", mean([](const Metrics& item) { return item.lumber_events; })},
        {"mean_pasture_built_events", mean([](const Metrics& item) { return item.pasture_events; })},
    };
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const Options options = ParseArgs(argc, argv);
        std::filesystem::create_directories(options.out);

        std::vector<Metrics> all_metrics;
        all_metrics.reserve(static_cast<std::size_t>(options.count));

        std::ofstream csv(options.out / "summary.csv");
        if (!csv) {
            std::cerr << "failed to open " << options.out / "summary.csv" << '\n';
            return 1;
        }
        WriteCsvHeader(csv);

        nlohmann::json rows = nlohmann::json::array();
        for (int i = 0; i < options.count; ++i) {
            const auto seed = options.start_seed + static_cast<std::uint64_t>(i);
            auto metrics = RunOne(options, seed);
            WriteCsvRow(csv, metrics);
            rows.push_back(ToJson(metrics));
            all_metrics.push_back(metrics);
        }

        std::ofstream json(options.out / "summary.json");
        json << nlohmann::json{{"aggregate", Aggregate(all_metrics)}, {"rows", rows}}.dump(2) << '\n';

        std::cout << "wrote " << options.count << " sim balance reports to " << options.out << '\n';
        std::cout << "mean population: " << Aggregate(all_metrics)["mean_total_population"] << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        PrintUsage();
        return 1;
    }
}
