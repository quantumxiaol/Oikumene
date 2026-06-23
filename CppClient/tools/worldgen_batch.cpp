#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "oikumene/world/world_generation_report.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

struct Options {
    std::uint64_t start_seed = 0;
    int count = 100;
    std::filesystem::path out = "../runs/worldgen_batch";
};

void PrintUsage() {
    std::cout << "usage: oikumene_worldgen_batch [--start-seed N] [--count N] [--out PATH]\n";
}

Options ParseArgs(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            std::exit(0);
        }
        if (arg == "--start-seed" && i + 1 < argc) {
            options.start_seed = static_cast<std::uint64_t>(std::stoull(argv[++i]));
            continue;
        }
        if (arg == "--count" && i + 1 < argc) {
            options.count = std::stoi(argv[++i]);
            continue;
        }
        if (arg == "--out" && i + 1 < argc) {
            options.out = argv[++i];
            continue;
        }
        throw std::runtime_error("Unknown or incomplete argument: " + arg);
    }
    if (options.count <= 0) {
        throw std::runtime_error("--count must be positive");
    }
    return options;
}

int CountBiome(const oikumene::WorldGenerationReport& report, oikumene::Biome biome) {
    const auto it = report.biome_counts.find(biome);
    return it == report.biome_counts.end() ? 0 : it->second;
}

int CountResource(const oikumene::WorldGenerationReport& report, oikumene::ResourceKind resource) {
    const auto it = report.resource_counts.find(resource);
    return it == report.resource_counts.end() ? 0 : it->second;
}

float Ratio(int count, int total) {
    return total <= 0 ? 0.0F : static_cast<float>(count) / static_cast<float>(total);
}

bool SeedLooksHealthy(const oikumene::WorldGenerationReport& report) {
    const int base_minerals = CountResource(report, oikumene::ResourceKind::Copper) +
                              CountResource(report, oikumene::ResourceKind::Tin) +
                              CountResource(report, oikumene::ResourceKind::ShallowIron) +
                              CountResource(report, oikumene::ResourceKind::ShallowCoal);
    return report.land_tiles > 0 && report.ocean_tiles > 0 && report.river_tiles >= 20 &&
           report.top_settlement_score >= 0.50F && CountResource(report, oikumene::ResourceKind::Wood) >= 20 &&
           base_minerals >= 1;
}

void WriteCsvHeader(std::ofstream& output) {
    output << "seed,land_ratio,ocean_ratio,river_tiles,lake_tiles,forest_ratio,desert_ratio,grassland_ratio,"
              "mountain_ratio,top_settlement_score,high_score_settlement_ratio,wood_count,horse_count,copper_count,"
              "tin_count,iron_count,coal_count,gold_count,silver_count,salt_count\n";
}

void WriteCsvRow(std::ofstream& output, const oikumene::WorldGenerationReport& report) {
    const int total = report.width * report.height;
    const int grassland = CountBiome(report, oikumene::Biome::Grassland);
    const int mountain = CountBiome(report, oikumene::Biome::Mountain) + CountBiome(report, oikumene::Biome::Hill);
    output << report.seed << ',' << report.land_ratio << ',' << (1.0F - report.land_ratio) << ','
           << report.river_tiles << ',' << report.lake_tiles << ',' << report.forest_ratio << ','
           << report.desert_ratio << ',' << Ratio(grassland, total) << ',' << Ratio(mountain, total) << ','
           << report.top_settlement_score << ',' << report.high_score_settlement_ratio << ','
           << CountResource(report, oikumene::ResourceKind::Wood) << ','
           << CountResource(report, oikumene::ResourceKind::Horse) << ','
           << CountResource(report, oikumene::ResourceKind::Copper) << ','
           << CountResource(report, oikumene::ResourceKind::Tin) << ','
           << CountResource(report, oikumene::ResourceKind::ShallowIron) << ','
           << CountResource(report, oikumene::ResourceKind::ShallowCoal) << ','
           << CountResource(report, oikumene::ResourceKind::Gold) << ','
           << CountResource(report, oikumene::ResourceKind::Silver) << ','
           << CountResource(report, oikumene::ResourceKind::Salt) << '\n';
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const Options options = ParseArgs(argc, argv);
        std::filesystem::create_directories(options.out);

        std::ofstream csv(options.out / "summary.csv");
        std::ofstream failed(options.out / "failed_seeds.txt");
        if (!csv || !failed) {
            std::cerr << "failed to open output files in " << options.out << '\n';
            return 1;
        }

        WriteCsvHeader(csv);
        nlohmann::json summary = nlohmann::json::array();

        int failures = 0;
        for (int i = 0; i < options.count; ++i) {
            oikumene::WorldGenerationParams params;
            params.seed = options.start_seed + static_cast<std::uint64_t>(i);
            const auto world = oikumene::WorldGenerator::Generate(params);
            const auto report = oikumene::BuildWorldGenerationReport(world);

            WriteCsvRow(csv, report);
            summary.push_back(oikumene::ToJson(report));
            if (!SeedLooksHealthy(report)) {
                failed << report.seed << '\n';
                ++failures;
            }
        }

        std::ofstream json(options.out / "summary.json");
        json << summary.dump(2) << '\n';

        std::cout << "wrote " << options.count << " reports to " << options.out << '\n';
        std::cout << "failed health seeds: " << failures << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        PrintUsage();
        return 1;
    }
}
