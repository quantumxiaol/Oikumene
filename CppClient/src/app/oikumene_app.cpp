#include "oikumene/app/oikumene_app.hpp"

#include <raylib.h>

#include "oikumene/app/app_state.hpp"
#include "oikumene/app/input_controller.hpp"
#include "oikumene/io/export_service.hpp"
#include "oikumene/ui/details_panel.hpp"
#include "oikumene/ui/event_log_panel.hpp"
#include "oikumene/ui/help_panel.hpp"
#include "oikumene/ui/hud_panel.hpp"
#include "oikumene/ui/legend_panel.hpp"
#include "oikumene/ui/panel_layout.hpp"
#include "oikumene/ui/playback_bar.hpp"

namespace oikumene {

OikumeneApp::OikumeneApp(AppConfig config) : config_(config) {}

int OikumeneApp::Run() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    if (config_.window.vsync) {
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    }

    InitWindow(config_.window.width, config_.window.height, "Oikumene - The Habitable World");
    if (config_.window.fullscreen) {
        ToggleFullscreen();
    }
    SetTargetFPS(60);

    AppState state;
    state.config = config_;
    state.show_debug_panel = config_.ui.show_debug_panel;
    state.show_help_panel = config_.ui.show_help_panel;
    state.controller.SetTurnsPerSecond(config_.simulation.turns_per_second);
    state.controller.SetRunning(config_.simulation.auto_run);
    BuildSimulation(state, config_.simulation.default_seed);
    state.controller.SetTurnsPerSecond(config_.simulation.turns_per_second);
    state.controller.SetRunning(config_.simulation.auto_run);
    state.camera.FitToWorld(state.simulation.GetWorld(), GetScreenWidth(), GetScreenHeight());
    state.health = state.remote_provider.CheckHealth();

    while (!WindowShouldClose()) {
        HandleInput(state);
        HandleFocusPause(state);
        UpdateHover(state);
        AdvanceAutoRun(state);

        BeginDrawing();
        ClearBackground(Color{18, 22, 26, 255});
        state.renderer.Draw(state.simulation.GetWorld(), state.simulation.Trades(), state.camera, state.current_layer,
                            state.hover_tile, state.selection);
        state.renderer.DrawEntities(state.simulation.Bands(), state.simulation.Settlements(), state.camera,
                                    state.selection);
        DrawHud(state);
        DrawPlaybackBar(state);
        DrawDebugPanel(state);
        if (state.show_help_panel) {
            DrawHelpPanel();
        }
        DrawEventLogPanel(state);
        if (state.show_legend_panel) {
            DrawLegendPanel(LegendPanelBounds());
        }
        EndDrawing();

        CapturePendingScreenshot(state);
    }

    CloseWindow();
    return 0;
}

} // namespace oikumene
