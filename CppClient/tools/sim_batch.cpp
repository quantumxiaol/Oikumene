#include <algorithm>
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
    int sample_every = 0;
    std::filesystem::path out = "../runs/sim_batch";
};

void PrintUsage() {
    std::cout << "usage: oikumene_sim_batch [--seed N] [--width N] [--height N] [--bands N] [--turns N]"
                 " [--sample-every N] [--out PATH]\n";
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
        if (arg == "--out" && NeedValue(argc, i)) {
            options.out = argv[++i];
            continue;
        }
        throw std::runtime_error("Unknown or incomplete argument: " + arg);
    }

    if (options.width <= 0 || options.height <= 0 || options.bands <= 0 || options.turns < 0 || options.sample_every < 0) {
        throw std::runtime_error(
            "--width/--height/--bands must be positive and --turns/--sample-every must be non-negative");
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
        {"last_decision_reason", band.last_decision_reason},
        {"current_tile_score", band.current_tile_score},
        {"best_seen_score", band.best_seen_score},
        {"forage_yield_last_turn", band.forage_yield_last_turn},
    };
}

nlohmann::json SettlementToJson(const oikumene::Settlement& settlement) {
    nlohmann::json worked_tiles = nlohmann::json::array();
    for (const int tile_index : settlement.worked_tile_indices) {
        worked_tiles.push_back(tile_index);
    }

    return nlohmann::json{
        {"id", settlement.id},
        {"x", settlement.x},
        {"y", settlement.y},
        {"founder_band_id", settlement.founder_band_id},
        {"polity_id", settlement.polity_id},
        {"is_capital", settlement.is_capital},
        {"population", settlement.population},
        {"level", oikumene::ToString(settlement.level)},
        {"stockpile", StockpileToJson(settlement.stockpile)},
        {"turns_since_founded", settlement.turns_since_founded},
        {"local_food_output_last_turn", settlement.local_food_output_last_turn},
        {"local_wood_output_last_turn", settlement.local_wood_output_last_turn},
        {"food_consumption_last_turn", settlement.food_consumption_last_turn},
        {"upgrade_readiness", settlement.upgrade_readiness},
        {"work_radius", settlement.work_radius},
        {"worked_tile_indices", worked_tiles},
        {"worked_tile_count", settlement.worked_tile_count},
        {"ore_output_last_turn", settlement.ore_output_last_turn},
        {"carrying_capacity", settlement.carrying_capacity},
        {"carrying_capacity_ratio", settlement.carrying_capacity_ratio},
    };
}

nlohmann::json PolityBudgetToJson(const oikumene::PolityBudget& budget) {
    return nlohmann::json{
        {"food_income", budget.food_income},
        {"wood_income", budget.wood_income},
        {"ore_income", budget.ore_income},
        {"wealth_income", budget.wealth_income},
        {"food_maintenance", budget.food_maintenance},
        {"wood_maintenance", budget.wood_maintenance},
        {"admin_maintenance", budget.admin_maintenance},
        {"control_maintenance", budget.control_maintenance},
        {"food_surplus", budget.food_surplus},
        {"wood_surplus", budget.wood_surplus},
        {"wealth_surplus", budget.wealth_surplus},
    };
}

nlohmann::json PolityToJson(const oikumene::Polity& polity) {
    return nlohmann::json{
        {"id", polity.id},
        {"name", polity.name},
        {"level", oikumene::ToString(polity.level)},
        {"capital_settlement_id", polity.capital_settlement_id},
        {"member_settlement_ids", polity.member_settlement_ids},
        {"population", polity.population},
        {"food", polity.food},
        {"wood", polity.wood},
        {"ore", polity.ore},
        {"wealth", polity.wealth},
        {"control_power", polity.control_power},
        {"admin_range", polity.admin_range},
        {"legitimacy", polity.legitimacy},
        {"stability", polity.stability},
        {"admin_load", polity.admin_load},
        {"admin_capacity", polity.admin_capacity},
        {"overextension", polity.overextension},
        {"controlled_tile_count", polity.controlled_tile_count},
        {"contested_tile_count", polity.contested_tile_count},
        {"founded_turn", polity.founded_turn},
        {"budget", PolityBudgetToJson(polity.budget)},
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

int CountCamps(const oikumene::Simulation& sim) {
    return static_cast<int>(sim.Settlements().size()) - CountVillages(sim);
}

int CountInactiveBands(const oikumene::Simulation& sim) {
    return static_cast<int>(sim.Bands().size()) - CountActiveBands(sim);
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

int CountWorkedTiles(const oikumene::Simulation& sim) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += tile.worked_by_settlement_id >= 0 ? 1 : 0;
    }
    return count;
}

float TotalFoodOutputLastTurn(const oikumene::Simulation& sim) {
    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += settlement.local_food_output_last_turn;
    }
    return total;
}

float TotalFoodConsumptionLastTurn(const oikumene::Simulation& sim) {
    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += settlement.food_consumption_last_turn;
    }
    return total;
}

float TotalWoodOutputLastTurn(const oikumene::Simulation& sim) {
    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += settlement.local_wood_output_last_turn;
    }
    return total;
}

float AverageCarryingCapacity(const oikumene::Simulation& sim) {
    if (sim.Settlements().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += settlement.carrying_capacity;
    }
    return total / static_cast<float>(sim.Settlements().size());
}

float ControlledLandRatio(const oikumene::Simulation& sim) {
    int land = 0;
    int controlled = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        if (tile.is_ocean || tile.is_lake) {
            continue;
        }
        ++land;
        controlled += tile.controller_polity_id != oikumene::kInvalidPolityId ? 1 : 0;
    }
    return land <= 0 ? 0.0F : static_cast<float>(controlled) / static_cast<float>(land);
}

int CountContestedTiles(const oikumene::Simulation& sim) {
    int count = 0;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        count += tile.is_contested ? 1 : 0;
    }
    return count;
}

float AverageAdminLoad(const oikumene::Simulation& sim) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& polity : sim.Polities()) {
        total += polity.admin_load;
    }
    return total / static_cast<float>(sim.Polities().size());
}

float AverageAdminCapacity(const oikumene::Simulation& sim) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& polity : sim.Polities()) {
        total += polity.admin_capacity;
    }
    return total / static_cast<float>(sim.Polities().size());
}

float AverageOverextension(const oikumene::Simulation& sim) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& polity : sim.Polities()) {
        total += polity.overextension;
    }
    return total / static_cast<float>(sim.Polities().size());
}

float AverageStability(const oikumene::Simulation& sim) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& polity : sim.Polities()) {
        total += polity.stability;
    }
    return total / static_cast<float>(sim.Polities().size());
}

float AverageSettlementScore(const oikumene::Simulation& sim) {
    if (sim.Settlements().empty()) {
        return 0.0F;
    }

    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += sim.GetWorld().At(settlement.x, settlement.y).settlement_score;
    }
    return total / static_cast<float>(sim.Settlements().size());
}

float AverageSettlementFertility(const oikumene::Simulation& sim) {
    if (sim.Settlements().empty()) {
        return 0.0F;
    }

    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += sim.GetWorld().At(settlement.x, settlement.y).fertility;
    }
    return total / static_cast<float>(sim.Settlements().size());
}

int MaxSettlementPopulation(const oikumene::Simulation& sim) {
    int maximum = 0;
    for (const auto& settlement : sim.Settlements()) {
        maximum = std::max(maximum, settlement.population);
    }
    return maximum;
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

nlohmann::json ImprovedTilesToJson(const oikumene::Simulation& sim) {
    nlohmann::json tiles = nlohmann::json::array();
    for (const auto& tile : sim.GetWorld().Tiles()) {
        if (tile.improvement == oikumene::ImprovementKind::None && tile.worked_by_settlement_id < 0) {
            continue;
        }
        tiles.push_back(nlohmann::json{
            {"x", tile.x},
            {"y", tile.y},
            {"improvement", oikumene::ToString(tile.improvement)},
            {"worked_by_settlement_id", tile.worked_by_settlement_id},
            {"soil_quality", tile.soil_quality},
            {"forest_cover", tile.forest_cover},
            {"resource", oikumene::ToString(tile.resource)},
        });
    }
    return tiles;
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
    nlohmann::json polities = nlohmann::json::array();
    for (const auto& polity : sim.Polities()) {
        polities.push_back(PolityToJson(polity));
    }

    return nlohmann::json{
        {"turn", sim.CurrentTurn()},
        {"status", sim.StatusSummary()},
        {"active_bands", CountActiveBands(sim)},
        {"settlements", settlements},
        {"polities", polities},
        {"bands", bands},
        {"improved_tiles", ImprovedTilesToJson(sim)},
        {"event_count", sim.Events().Size()},
    };
}

nlohmann::json SummaryToJson(const Options& options, const oikumene::Simulation& sim) {
    const int total_population = TotalPopulation(sim);
    return nlohmann::json{
        {"seed", options.seed},
        {"width", options.width},
        {"height", options.height},
        {"turns", options.turns},
        {"initial_bands", options.bands},
        {"active_bands", CountActiveBands(sim)},
        {"inactive_bands", CountInactiveBands(sim)},
        {"settlements", sim.Settlements().size()},
        {"camps", CountCamps(sim)},
        {"villages", CountVillages(sim)},
        {"total_population", total_population},
        {"population_total", total_population},
        {"average_settlement_score", AverageSettlementScore(sim)},
        {"average_settlement_fertility", AverageSettlementFertility(sim)},
        {"max_settlement_population", MaxSettlementPopulation(sim)},
        {"farm_count", CountImprovement(sim, oikumene::ImprovementKind::Farm)},
        {"lumbercamp_count", CountImprovement(sim, oikumene::ImprovementKind::LumberCamp)},
        {"pasture_count", CountImprovement(sim, oikumene::ImprovementKind::Pasture)},
        {"shallow_mine_count", CountImprovement(sim, oikumene::ImprovementKind::ShallowMine)},
        {"worked_tile_count", CountWorkedTiles(sim)},
        {"total_food_output_last_turn", TotalFoodOutputLastTurn(sim)},
        {"total_food_consumption_last_turn", TotalFoodConsumptionLastTurn(sim)},
        {"total_wood_output_last_turn", TotalWoodOutputLastTurn(sim)},
        {"average_carrying_capacity", AverageCarryingCapacity(sim)},
        {"polities", sim.Polities().size()},
        {"controlled_land_ratio", ControlledLandRatio(sim)},
        {"contested_tiles", CountContestedTiles(sim)},
        {"average_admin_load", AverageAdminLoad(sim)},
        {"average_admin_capacity", AverageAdminCapacity(sim)},
        {"average_overextension", AverageOverextension(sim)},
        {"average_stability", AverageStability(sim)},
        {"event_count", sim.Events().Size()},
        {"migration_events", CountEvents(sim, oikumene::EventType::BandMigrated)},
        {"settlement_founded_events", CountEvents(sim, oikumene::EventType::SettlementFounded)},
        {"population_growth_events", CountEvents(sim, oikumene::EventType::PopulationGrowth)},
        {"famine_events", CountEvents(sim, oikumene::EventType::Famine)},
        {"village_upgrade_events", CountEvents(sim, oikumene::EventType::SettlementUpgraded)},
        {"farm_built_events", CountEvents(sim, oikumene::EventType::FarmBuilt)},
        {"lumbercamp_built_events", CountEvents(sim, oikumene::EventType::LumberCampBuilt)},
        {"pasture_built_events", CountEvents(sim, oikumene::EventType::PastureBuilt)},
        {"carrying_capacity_reached_events", CountEvents(sim, oikumene::EventType::CarryingCapacityReached)},
    };
}

nlohmann::json StateSampleToJson(const oikumene::Simulation& sim) {
    return nlohmann::json{
        {"turn", sim.CurrentTurn()},
        {"active_bands", CountActiveBands(sim)},
        {"inactive_bands", CountInactiveBands(sim)},
        {"settlements", sim.Settlements().size()},
        {"camps", CountCamps(sim)},
        {"villages", CountVillages(sim)},
        {"total_population", TotalPopulation(sim)},
        {"event_count", sim.Events().Size()},
        {"average_settlement_score", AverageSettlementScore(sim)},
        {"average_settlement_fertility", AverageSettlementFertility(sim)},
        {"max_settlement_population", MaxSettlementPopulation(sim)},
        {"farm_count", CountImprovement(sim, oikumene::ImprovementKind::Farm)},
        {"lumbercamp_count", CountImprovement(sim, oikumene::ImprovementKind::LumberCamp)},
        {"pasture_count", CountImprovement(sim, oikumene::ImprovementKind::Pasture)},
        {"worked_tile_count", CountWorkedTiles(sim)},
        {"total_food_output_last_turn", TotalFoodOutputLastTurn(sim)},
        {"total_food_consumption_last_turn", TotalFoodConsumptionLastTurn(sim)},
        {"total_wood_output_last_turn", TotalWoodOutputLastTurn(sim)},
        {"average_carrying_capacity", AverageCarryingCapacity(sim)},
        {"polities", sim.Polities().size()},
        {"controlled_land_ratio", ControlledLandRatio(sim)},
        {"contested_tiles", CountContestedTiles(sim)},
        {"average_admin_load", AverageAdminLoad(sim)},
        {"average_admin_capacity", AverageAdminCapacity(sim)},
        {"average_overextension", AverageOverextension(sim)},
        {"average_stability", AverageStability(sim)},
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

void WriteStateSample(std::ofstream& output, const oikumene::Simulation& sim) {
    output << StateSampleToJson(sim).dump() << '\n';
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
