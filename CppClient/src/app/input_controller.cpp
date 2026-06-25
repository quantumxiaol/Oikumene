#include "oikumene/app/input_controller.hpp"

#include "oikumene/app/input_policy.hpp"
#include "oikumene/io/export_service.hpp"
#include "oikumene/render/map_layer.hpp"
#include "oikumene/ui/panel_layout.hpp"
#include "oikumene/ui/view_helpers.hpp"
#include "oikumene/world/world_generator.hpp"

namespace oikumene {
namespace {

std::uint64_t NextSeed(std::uint64_t seed) {
    seed ^= seed >> 12U;
    seed ^= seed << 25U;
    seed ^= seed >> 27U;
    return seed * 2685821657736338717ULL;
}

ShortcutModifiers CurrentShortcutModifiers() {
    return ShortcutModifiers{
        .command = IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER),
        .control = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL),
        .alt = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT),
        .shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT),
    };
}

bool GameKeyPressed(int key, const ShortcutModifiers& modifiers) {
    return AppHotkeysAllowed(modifiers) && IsKeyPressed(key);
}

void SelectAtMouse(AppState& state, Vector2 mouse) {
    int x = 0;
    int y = 0;
    if (!state.camera.ScreenToTile(mouse, state.simulation.GetWorld(), x, y)) {
        ApplySelection(state, ClearSelection());
        return;
    }
    ApplySelection(state, SelectAtTile(state.simulation.GetWorld(), state.simulation.Bands(),
                                       state.simulation.Settlements(), x, y));
}

bool CenterOnSelection(AppState& state) {
    if (state.selection.kind == SelectionKind::Settlement) {
        if (const auto* settlement = SettlementById(state.simulation.Settlements(), state.selection.id)) {
            state.camera.CenterOnTile(settlement->x, settlement->y);
            return true;
        }
    }
    if (state.selection.kind == SelectionKind::Band) {
        if (const auto* band = BandById(state.simulation.Bands(), state.selection.id)) {
            state.camera.CenterOnTile(band->x, band->y);
            return true;
        }
    }
    if (state.selection.kind == SelectionKind::Tile || state.selection.kind == SelectionKind::ImprovementTile ||
        state.selection.kind == SelectionKind::RouteTile) {
        state.camera.CenterOnTile(state.selection.x, state.selection.y);
        return true;
    }
    return false;
}

void HandleLayerHotkeys(AppState& state) {
    if (!LayerHotkeysAllowed(CurrentShortcutModifiers())) {
        return;
    }

    if (IsKeyPressed(KEY_ONE)) {
        state.current_layer = MapLayer::Biome;
    }
    if (IsKeyPressed(KEY_TWO)) {
        state.current_layer = MapLayer::Elevation;
    }
    if (IsKeyPressed(KEY_THREE)) {
        state.current_layer = MapLayer::Rainfall;
    }
    if (IsKeyPressed(KEY_FOUR)) {
        state.current_layer = MapLayer::Temperature;
    }
    if (IsKeyPressed(KEY_FIVE)) {
        state.current_layer = MapLayer::Fertility;
    }
    if (IsKeyPressed(KEY_SIX)) {
        state.current_layer = MapLayer::Resources;
    }
    if (IsKeyPressed(KEY_SEVEN)) {
        state.current_layer = MapLayer::SettlementScore;
    }
    if (IsKeyPressed(KEY_EIGHT)) {
        state.current_layer = MapLayer::PolityControl;
    }
    if (IsKeyPressed(KEY_NINE)) {
        state.current_layer = MapLayer::RouteNetwork;
    }
    if (IsKeyPressed(KEY_ZERO)) {
        state.current_layer = MapLayer::TradeNetwork;
    }
}

} // namespace

void StepTurns(AppState& state, int turns) {
    state.controller.StepTurns(state.simulation, turns);
}

void BuildSimulation(AppState& state, std::uint64_t seed) {
    state.generation_params.seed = seed;
    state.generation_params.width = state.config.simulation.world_width;
    state.generation_params.height = state.config.simulation.world_height;
    state.simulation_params.initial_band_count = state.config.simulation.initial_bands;
    state.simulation_params.enable_routes = state.config.simulation.enable_routes;
    state.simulation = Simulation(WorldGenerator::Generate(state.generation_params), state.simulation_params);
    state.simulation.InitializeBands(state.config.simulation.initial_bands);
    state.report = BuildWorldGenerationReport(state.simulation.GetWorld());
    state.selection = ClearSelection();
    state.selected_band_id = -1;
    state.selected_settlement_id = -1;
    state.controller.SetRunning(false);
}

void ApplySelection(AppState& state, Selection selection) {
    state.selection = selection;
    state.selected_band_id = selection.kind == SelectionKind::Band ? selection.id : -1;
    state.selected_settlement_id = selection.kind == SelectionKind::Settlement ? selection.id : -1;
}

void HandleInput(AppState& state) {
    state.camera.SetViewport(
        Rectangle{0.0F, 0.0F, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())});
    const Vector2 mouse = GetMousePosition();
    const bool mouse_captured = UiCapturesMouse(state, mouse);
    state.camera.Update(!mouse_captured, true);
    HandleLayerHotkeys(state);
    const auto modifiers = CurrentShortcutModifiers();

    if (GameKeyPressed(KEY_R, modifiers)) {
        BuildSimulation(state, NextSeed(state.generation_params.seed));
        state.camera.FitWorld(state.simulation.GetWorld().Width(), state.simulation.GetWorld().Height());
        state.status_message = "Generated seed " + std::to_string(state.generation_params.seed);
    }
    if (GameKeyPressed(KEY_B, modifiers)) {
        state.simulation.InitializeBands(state.config.simulation.initial_bands);
        ApplySelection(state, ClearSelection());
        state.selected_band_id = -1;
        state.selected_settlement_id = -1;
        state.status_message = "Reset bands on current world";
    }
    if (GameKeyPressed(KEY_H, modifiers)) {
        state.health = state.remote_provider.CheckHealth();
        state.status_message = "Python health check: " + StatusText(state.health);
    }
    if (GameKeyPressed(KEY_SPACE, modifiers)) {
        StepTurns(state, 1);
    }
    if (GameKeyPressed(KEY_N, modifiers)) {
        StepTurns(state, UsesLargeStep(modifiers) ? 100 : 10);
    }
    if (GameKeyPressed(KEY_A, modifiers)) {
        state.controller.ToggleRunning();
    }
    if (GameKeyPressed(KEY_MINUS, modifiers)) {
        state.controller.SetTurnsPerSecond(state.controller.TurnsPerSecond() - 1.0F);
    }
    if (GameKeyPressed(KEY_EQUAL, modifiers)) {
        state.controller.SetTurnsPerSecond(state.controller.TurnsPerSecond() + 1.0F);
    }
    if (GameKeyPressed(KEY_TAB, modifiers)) {
        state.show_debug_panel = !state.show_debug_panel;
    }
    if (GameKeyPressed(KEY_E, modifiers)) {
        state.show_event_log_panel = !state.show_event_log_panel;
    }
    if (GameKeyPressed(KEY_C, modifiers)) {
        if (CenterOnSelection(state)) {
            state.status_message = "Centered selection";
        } else {
            state.camera.FitWorld(state.simulation.GetWorld().Width(), state.simulation.GetWorld().Height());
            state.status_message = "Fit world";
        }
    }
    if (IsKeyPressed(KEY_HOME) || IsKeyPressed(KEY_F)) {
        if (AppHotkeysAllowed(modifiers)) {
            state.camera.FitWorld(state.simulation.GetWorld().Width(), state.simulation.GetWorld().Height());
            state.status_message = "Fit world";
        }
    }
    if (GameKeyPressed(KEY_F1, modifiers)) {
        state.show_help_panel = !state.show_help_panel;
    }
    if (GameKeyPressed(KEY_F2, modifiers)) {
        state.show_legend_panel = !state.show_legend_panel;
    }
    if (GameKeyPressed(KEY_F11, modifiers)) {
        ToggleFullscreen();
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !mouse_captured) {
        SelectAtMouse(state, mouse);
    }
    if (GameKeyPressed(KEY_M, modifiers)) {
        const auto path = WorldgenDirectory(state.generation_params.seed) / "report.json";
        state.status_message = WriteReportJson(state.report, path) ? "Exported report: " + path.string()
                                                                   : "Failed to export report: " + path.string();
    }
    if (GameKeyPressed(KEY_P, modifiers)) {
        if (ShouldPauseForScreenshot(state.config.ui.pause_on_screenshot)) {
            state.controller.SetRunning(false);
        }
        const auto directory = WorldgenDirectory(state.generation_params.seed);
        std::filesystem::create_directories(directory);
        state.pending_screenshot = directory / LayerFilename(state.current_layer);
    }
}

void AdvanceAutoRun(AppState& state) {
    state.controller.Update(GetFrameTime(), state.simulation);
}

void HandleFocusPause(AppState& state) {
    const bool focused = IsWindowFocused();
    if (ShouldPauseForFocusLoss(state.config.ui.pause_on_focus_loss, state.window_was_focused, focused)) {
        state.controller.SetRunning(false);
        state.status_message = "Paused: window focus lost";
    }
    state.window_was_focused = focused;
}

void UpdateHover(AppState& state) {
    state.hover_tile.reset();
    const Vector2 mouse = GetMousePosition();
    if (UiCapturesMouse(state, mouse)) {
        return;
    }
    if (state.camera.ScreenToTile(mouse, state.simulation.GetWorld(), state.hover_x, state.hover_y)) {
        state.hover_tile = std::pair<int, int>{state.hover_x, state.hover_y};
    }
}

} // namespace oikumene
