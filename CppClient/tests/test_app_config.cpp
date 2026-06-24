#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "oikumene/app/app_config.hpp"
#include "oikumene/app/command_line.hpp"
#include "oikumene/core/simulation.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

void TestAppConfigJsonRoundTrip() {
    using namespace oikumene;

    AppConfig config;
    config.window.width = 1440;
    config.window.height = 900;
    config.window.fullscreen = true;
    config.ui.show_debug_panel = false;
    config.simulation.default_seed = 1234;
    config.simulation.world_width = 96;
    config.simulation.world_height = 64;
    config.simulation.initial_bands = 12;
    config.simulation.enable_routes = false;
    config.simulation.turns_per_second = 8.0F;

    const auto restored = AppConfigFromJson(ToJson(config));
    assert(restored.window.width == 1440);
    assert(restored.window.height == 900);
    assert(restored.window.fullscreen);
    assert(!restored.ui.show_debug_panel);
    assert(restored.simulation.default_seed == 1234);
    assert(restored.simulation.world_width == 96);
    assert(restored.simulation.world_height == 64);
    assert(restored.simulation.initial_bands == 12);
    assert(!restored.simulation.enable_routes);
    assert(restored.simulation.turns_per_second == 8.0F);
}

void TestAppConfigLoadSaveRoundTrip() {
    using namespace oikumene;

    const auto path = std::filesystem::temp_directory_path() / "oikumene_app_config_test" / "settings.json";
    std::filesystem::remove_all(path.parent_path());

    AppConfig config;
    config.window.width = 1600;
    config.window.height = 1000;
    config.simulation.default_seed = 9876;
    config.simulation.initial_bands = 10;

    assert(SaveAppConfig(path, config));
    const auto restored = LoadAppConfig(path);
    assert(restored.window.width == 1600);
    assert(restored.window.height == 1000);
    assert(restored.simulation.default_seed == 9876);
    assert(restored.simulation.initial_bands == 10);

    std::filesystem::remove_all(path.parent_path());
}

void TestCommandLineOverridesConfig() {
    using namespace oikumene;

    AppConfig base;
    base.simulation.default_seed = 1;
    const std::vector<std::string> args = {
        "--seed", "42", "--width", "90", "--height", "60", "--bands", "9",
        "--window", "1024x768", "--turns-per-second", "12.5", "--auto-run", "--disable-routes",
    };
    const auto parsed = ParseCommandLine(args, base);
    assert(parsed.error.empty());
    assert(!parsed.show_help);
    assert(parsed.config.simulation.default_seed == 42);
    assert(parsed.config.simulation.world_width == 90);
    assert(parsed.config.simulation.world_height == 60);
    assert(parsed.config.simulation.initial_bands == 9);
    assert(parsed.config.window.width == 1024);
    assert(parsed.config.window.height == 768);
    assert(parsed.config.simulation.turns_per_second == 12.5F);
    assert(parsed.config.simulation.auto_run);
    assert(!parsed.config.simulation.enable_routes);
}

void TestCommandLineSeedOverridesConfig() {
    using namespace oikumene;

    AppConfig base;
    base.simulation.default_seed = 777;
    const auto parsed = ParseCommandLine({"--seed", "42"}, base);
    assert(parsed.error.empty());
    assert(parsed.config.simulation.default_seed == 42);
}

void TestStartupWorldUsesCommandLineSeed() {
    using namespace oikumene;

    AppConfig base;
    base.simulation.default_seed = 777;
    const auto parsed = ParseCommandLine({"--seed", "42"}, base);
    assert(parsed.error.empty());

    WorldGenerationParams params;
    params.seed = parsed.config.simulation.default_seed;
    params.width = parsed.config.simulation.world_width;
    params.height = parsed.config.simulation.world_height;
    Simulation sim(WorldGenerator::Generate(params), SimulationParams{});
    assert(sim.GetWorld().Seed() == 42);
}

void TestCommandLineRejectsInvalidValues() {
    using namespace oikumene;

    const auto parsed_width = ParseCommandLine({"--width", "0"});
    assert(!parsed_width.error.empty());

    const auto parsed_window = ParseCommandLine({"--window", "bad"});
    assert(!parsed_window.error.empty());

    const auto parsed_unknown = ParseCommandLine({"--unknown"});
    assert(!parsed_unknown.error.empty());
}

}  // namespace

int main() {
    TestAppConfigJsonRoundTrip();
    TestAppConfigLoadSaveRoundTrip();
    TestCommandLineOverridesConfig();
    TestCommandLineSeedOverridesConfig();
    TestStartupWorldUsesCommandLineSeed();
    TestCommandLineRejectsInvalidValues();

    std::cout << "oikumene_app_config_tests passed\n";
    return 0;
}
