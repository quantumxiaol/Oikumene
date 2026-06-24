#pragma once

#include <cstdint>
#include <filesystem>

#include <nlohmann/json.hpp>

namespace oikumene {

struct WindowConfig {
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool borderless = false;
    bool vsync = true;
};

struct UiConfig {
    float scale = 1.0F;
    bool show_debug_panel = false;
    bool show_help_panel = false;
};

struct SimulationConfig {
    std::uint64_t default_seed = 42;
    int world_width = 80;
    int world_height = 56;
    int initial_bands = 8;
    bool auto_run = false;
    bool enable_routes = true;
    float turns_per_second = 4.0F;
};

struct AppConfig {
    WindowConfig window;
    UiConfig ui;
    SimulationConfig simulation;
};

[[nodiscard]] nlohmann::json ToJson(const AppConfig& config);
[[nodiscard]] AppConfig AppConfigFromJson(const nlohmann::json& json);
[[nodiscard]] AppConfig LoadAppConfig(const std::filesystem::path& path, const AppConfig& fallback = {});
bool SaveAppConfig(const std::filesystem::path& path, const AppConfig& config);

}  // namespace oikumene
