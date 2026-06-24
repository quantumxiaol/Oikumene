#include "oikumene/app/app_config.hpp"

#include <fstream>

namespace oikumene {
namespace {

template <typename T> T JsonValue(const nlohmann::json& json, const char* key, T fallback) {
    if (!json.is_object() || !json.contains(key)) {
        return fallback;
    }
    return json.at(key).get<T>();
}

} // namespace

nlohmann::json ToJson(const AppConfig& config) {
    return nlohmann::json{
        {"window",
         {
             {"width", config.window.width},
             {"height", config.window.height},
             {"fullscreen", config.window.fullscreen},
             {"borderless", config.window.borderless},
             {"vsync", config.window.vsync},
         }},
        {"ui",
         {
             {"scale", config.ui.scale},
             {"show_debug_panel", config.ui.show_debug_panel},
             {"show_help_panel", config.ui.show_help_panel},
         }},
        {"simulation",
         {
             {"default_seed", config.simulation.default_seed},
             {"world_width", config.simulation.world_width},
             {"world_height", config.simulation.world_height},
             {"initial_bands", config.simulation.initial_bands},
             {"auto_run", config.simulation.auto_run},
             {"enable_routes", config.simulation.enable_routes},
             {"turns_per_second", config.simulation.turns_per_second},
         }},
    };
}

AppConfig AppConfigFromJson(const nlohmann::json& json) {
    AppConfig config;
    const auto window = json.value("window", nlohmann::json::object());
    const auto ui = json.value("ui", nlohmann::json::object());
    const auto simulation = json.value("simulation", nlohmann::json::object());

    config.window.width = JsonValue(window, "width", config.window.width);
    config.window.height = JsonValue(window, "height", config.window.height);
    config.window.fullscreen = JsonValue(window, "fullscreen", config.window.fullscreen);
    config.window.borderless = JsonValue(window, "borderless", config.window.borderless);
    config.window.vsync = JsonValue(window, "vsync", config.window.vsync);

    config.ui.scale = JsonValue(ui, "scale", config.ui.scale);
    config.ui.show_debug_panel = JsonValue(ui, "show_debug_panel", config.ui.show_debug_panel);
    config.ui.show_help_panel = JsonValue(ui, "show_help_panel", config.ui.show_help_panel);

    config.simulation.default_seed = JsonValue(simulation, "default_seed", config.simulation.default_seed);
    config.simulation.world_width = JsonValue(simulation, "world_width", config.simulation.world_width);
    config.simulation.world_height = JsonValue(simulation, "world_height", config.simulation.world_height);
    config.simulation.initial_bands = JsonValue(simulation, "initial_bands", config.simulation.initial_bands);
    config.simulation.auto_run = JsonValue(simulation, "auto_run", config.simulation.auto_run);
    config.simulation.enable_routes = JsonValue(simulation, "enable_routes", config.simulation.enable_routes);
    config.simulation.turns_per_second = JsonValue(simulation, "turns_per_second", config.simulation.turns_per_second);

    return config;
}

AppConfig LoadAppConfig(const std::filesystem::path& path, const AppConfig& fallback) {
    std::ifstream input(path);
    if (!input) {
        return fallback;
    }

    try {
        return AppConfigFromJson(nlohmann::json::parse(input));
    } catch (const std::exception&) {
        return fallback;
    }
}

bool SaveAppConfig(const std::filesystem::path& path, const AppConfig& config) {
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
    }
    std::ofstream output(path);
    if (!output) {
        return false;
    }
    output << ToJson(config).dump(2) << '\n';
    return true;
}

} // namespace oikumene
