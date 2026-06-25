#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

#include "oikumene/ai/remote_decision_provider.hpp"
#include "oikumene/app/app_config.hpp"
#include "oikumene/app/simulation_controller.hpp"
#include "oikumene/core/simulation.hpp"
#include "oikumene/render/camera_controller.hpp"
#include "oikumene/render/map_layer.hpp"
#include "oikumene/render/map_renderer.hpp"
#include "oikumene/render/selection.hpp"
#include "oikumene/world/world_generation_params.hpp"
#include "oikumene/world/world_generation_report.hpp"

namespace oikumene {

struct AppState {
    AppConfig config;
    WorldGenerationParams generation_params;
    SimulationParams simulation_params;
    Simulation simulation;
    WorldGenerationReport report;

    RemoteDecisionProvider remote_provider{"127.0.0.1", 8000};
    HealthStatus health;
    CameraController camera;
    MapRenderer renderer;
    SimulationController controller;
    MapLayer current_layer = MapLayer::Biome;

    bool show_debug_panel = true;
    bool show_help_panel = false;
    bool show_event_log_panel = false;
    bool show_legend_panel = false;
    bool window_was_focused = true;
    std::string status_message;
    std::optional<std::filesystem::path> pending_screenshot;
    std::optional<std::pair<int, int>> hover_tile;
    Selection selection;
    int hover_x = 0;
    int hover_y = 0;
    int selected_band_id = -1;
    int selected_settlement_id = -1;
};

} // namespace oikumene
