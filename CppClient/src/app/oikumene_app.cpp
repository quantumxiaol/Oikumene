#include "oikumene/app/oikumene_app.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <raylib.h>

#include "oikumene/ai/remote_decision_provider.hpp"
#include "oikumene/app/app_state.hpp"
#include "oikumene/render/map_layer.hpp"
#include "oikumene/sim/band_system.hpp"
#include "oikumene/sim/settlement_system.hpp"
#include "oikumene/world/biome.hpp"
#include "oikumene/world/resource.hpp"
#include "oikumene/world/world_generator.hpp"

namespace oikumene {
namespace {

Color StatusColor(bool online) {
    return online ? Color{80, 210, 120, 255} : Color{230, 90, 75, 255};
}

std::string StatusText(const HealthStatus& status) {
    if (status.online) {
        return "online (" + std::to_string(status.latency_ms) + " ms)";
    }
    if (!status.message.empty()) {
        return "offline - " + status.message;
    }
    return "offline";
}

void DrawPanelBackground(int x, int y, int width, int height) {
    DrawRectangle(x, y, width, height, Color{18, 22, 26, 226});
    DrawRectangleLines(x, y, width, height, Color{78, 86, 94, 255});
}

std::string Fixed(float value, int precision = 2) {
    std::ostringstream stream;
    stream.setf(std::ios::fixed);
    stream.precision(precision);
    stream << value;
    return stream.str();
}

std::uint64_t NextSeed(std::uint64_t seed) {
    seed ^= seed >> 12U;
    seed ^= seed << 25U;
    seed ^= seed >> 27U;
    return seed * 2685821657736338717ULL;
}

std::filesystem::path RunsDirectory() {
    const auto cwd = std::filesystem::current_path();
    if (cwd.filename() == "CppClient") {
        return cwd.parent_path() / "runs";
    }
    if (std::filesystem::exists(cwd / "CppClient")) {
        return cwd / "runs";
    }
    return cwd / "runs";
}

std::filesystem::path WorldgenDirectory(std::uint64_t seed) {
    return RunsDirectory() / ("worldgen_seed_" + std::to_string(seed));
}

std::string LayerFilename(MapLayer layer) {
    std::string name = ToString(layer);
    for (auto& ch : name) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return "layer_" + name + ".png";
}

bool WriteReportJson(const WorldGenerationReport& report, std::filesystem::path path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path);
    if (!output) {
        return false;
    }
    output << ToJson(report).dump(2) << '\n';
    return true;
}

int ActiveBandCount(const std::vector<Band>& bands) {
    int count = 0;
    for (const auto& band : bands) {
        count += band.active ? 1 : 0;
    }
    return count;
}

int VillageCount(const std::vector<Settlement>& settlements) {
    int count = 0;
    for (const auto& settlement : settlements) {
        count += settlement.level == SettlementLevel::Village ? 1 : 0;
    }
    return count;
}

const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id) {
    for (const auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

const Settlement* SettlementAt(const std::vector<Settlement>& settlements, int x, int y) {
    for (const auto& settlement : settlements) {
        if (settlement.x == x && settlement.y == y) {
            return &settlement;
        }
    }
    return nullptr;
}

const Band* BandById(const std::vector<Band>& bands, int id) {
    for (const auto& band : bands) {
        if (band.id == id) {
            return &band;
        }
    }
    return nullptr;
}

const Band* ActiveBandAt(const std::vector<Band>& bands, int x, int y) {
    for (const auto& band : bands) {
        if (band.active && band.x == x && band.y == y) {
            return &band;
        }
    }
    return nullptr;
}

void StepTurns(AppState& state, int turns) {
    for (int i = 0; i < turns; ++i) {
        state.simulation.AdvanceOneTurn();
    }
}

void BuildSimulation(AppState& state, std::uint64_t seed) {
    state.generation_params.seed = seed;
    state.generation_params.width = state.config.simulation.world_width;
    state.generation_params.height = state.config.simulation.world_height;
    state.simulation_params.initial_band_count = state.config.simulation.initial_bands;
    state.simulation = Simulation(WorldGenerator::Generate(state.generation_params), state.simulation_params);
    state.simulation.InitializeBands(state.config.simulation.initial_bands);
    state.report = BuildWorldGenerationReport(state.simulation.GetWorld());
    state.selected_band_id = -1;
    state.selected_settlement_id = -1;
    state.auto_run_accumulator = 0.0F;
}

void SelectAtHover(AppState& state) {
    if (!state.hover_tile.has_value()) {
        state.selected_band_id = -1;
        state.selected_settlement_id = -1;
        return;
    }

    const auto [x, y] = *state.hover_tile;
    if (const auto* settlement = SettlementAt(state.simulation.Settlements(), x, y)) {
        state.selected_settlement_id = settlement->id;
        state.selected_band_id = -1;
        return;
    }
    if (const auto* band = ActiveBandAt(state.simulation.Bands(), x, y)) {
        state.selected_band_id = band->id;
        state.selected_settlement_id = -1;
        return;
    }
}

void HandleLayerHotkeys(AppState& state) {
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
}

void HandleInput(AppState& state) {
    state.camera.Update();
    HandleLayerHotkeys(state);

    if (IsKeyPressed(KEY_R)) {
        BuildSimulation(state, NextSeed(state.generation_params.seed));
        state.status_message = "Generated seed " + std::to_string(state.generation_params.seed);
    }
    if (IsKeyPressed(KEY_B)) {
        state.simulation.InitializeBands(state.config.simulation.initial_bands);
        state.selected_band_id = -1;
        state.selected_settlement_id = -1;
        state.status_message = "Reset bands on current world";
    }
    if (IsKeyPressed(KEY_H)) {
        state.health = state.remote_provider.CheckHealth();
        state.status_message = "Python health check: " + StatusText(state.health);
    }
    if (IsKeyPressed(KEY_SPACE)) {
        StepTurns(state, 1);
    }
    if (IsKeyPressed(KEY_N)) {
        StepTurns(state, IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ? 100 : 10);
    }
    if (IsKeyPressed(KEY_A)) {
        state.auto_run = !state.auto_run;
    }
    if (IsKeyPressed(KEY_TAB)) {
        state.show_debug_panel = !state.show_debug_panel;
    }
    if (IsKeyPressed(KEY_F1)) {
        state.show_help_panel = !state.show_help_panel;
    }
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        SelectAtHover(state);
    }
    if (IsKeyPressed(KEY_M)) {
        const auto path = WorldgenDirectory(state.generation_params.seed) / "report.json";
        state.status_message = WriteReportJson(state.report, path) ? "Exported report: " + path.string()
                                                                   : "Failed to export report: " + path.string();
    }
    if (IsKeyPressed(KEY_P)) {
        const auto directory = WorldgenDirectory(state.generation_params.seed);
        std::filesystem::create_directories(directory);
        state.pending_screenshot = directory / LayerFilename(state.current_layer);
    }
}

void AdvanceAutoRun(AppState& state) {
    if (!state.auto_run) {
        return;
    }

    const float turns_per_second = std::max(0.25F, state.config.simulation.turns_per_second);
    state.auto_run_accumulator += GetFrameTime() * turns_per_second;
    const int turns = std::min(25, static_cast<int>(state.auto_run_accumulator));
    if (turns <= 0) {
        return;
    }
    state.auto_run_accumulator -= static_cast<float>(turns);
    StepTurns(state, turns);
}

void UpdateHover(AppState& state) {
    state.hover_tile.reset();
    if (state.camera.ScreenToTile(GetMousePosition(), state.simulation.GetWorld(), state.hover_x, state.hover_y)) {
        state.hover_tile = std::pair<int, int>{state.hover_x, state.hover_y};
    }
}

void DrawEventLogPanel(const AppState& state) {
    if (!state.show_event_log_panel) {
        return;
    }

    const int width = 430;
    const int height = 230;
    const int x = GetScreenWidth() - width - 18;
    const int y = GetScreenHeight() - height - 18;
    DrawPanelBackground(x, y, width, height);
    DrawText("Recent Events", x + 16, y + 16, 20, RAYWHITE);

    const auto& events = state.simulation.Events().Events();
    int line_y = y + 48;
    const int first = std::max(0, static_cast<int>(events.size()) - 8);
    for (int i = first; i < static_cast<int>(events.size()); ++i) {
        const auto& event = events[static_cast<std::size_t>(i)];
        std::string line = "T" + std::to_string(event.turn) + " " + ToString(event.type) + ": " + event.summary;
        if (line.size() > 58) {
            line = line.substr(0, 55) + "...";
        }
        DrawText(line.c_str(), x + 16, line_y, 15, Color{202, 211, 218, 255});
        line_y += 21;
    }
}

void DrawInspectorDetails(const AppState& state, int& y) {
    if (state.hover_tile.has_value()) {
        const auto& tile = state.simulation.GetWorld().At(state.hover_x, state.hover_y);
        DrawText(("Tile: " + std::to_string(tile.x) + ", " + std::to_string(tile.y)).c_str(), 34, y, 17, RAYWHITE);
        y += 22;
        DrawText(("Biome: " + ToString(tile.biome) + "  Resource: " + ToString(tile.resource)).c_str(), 34, y, 17,
                 Color{202, 211, 218, 255});
        y += 22;
        DrawText(("Elevation " + Fixed(tile.elevation) + "  Temp " + Fixed(tile.temperature)).c_str(), 34, y, 17,
                 Color{202, 211, 218, 255});
        y += 22;
        DrawText(("Rain " + Fixed(tile.rainfall) + "  Fertility " + Fixed(tile.fertility)).c_str(), 34, y, 17,
                 Color{202, 211, 218, 255});
        y += 22;
        DrawText(("River " + std::string(tile.has_river ? "yes " : "no ") + Fixed(tile.river_flow) + "  Settle " +
                  Fixed(tile.settlement_score))
                     .c_str(),
                 34, y, 17, Color{202, 211, 218, 255});
        y += 26;
    }

    if (const auto* band = BandById(state.simulation.Bands(), state.selected_band_id)) {
        DrawText(("Band " + std::to_string(band->id) + " " + ToString(band->state)).c_str(), 34, y, 17,
                 Color{245, 245, 236, 255});
        y += 22;
        DrawText(("Pop " + std::to_string(band->population) + "  Food " + Fixed(band->food)).c_str(), 34, y, 17,
                 Color{245, 245, 236, 255});
        y += 22;
        DrawText(("At " + std::to_string(band->x) + "," + std::to_string(band->y) + " Target " +
                  std::to_string(band->target_x) + "," + std::to_string(band->target_y))
                     .c_str(),
                 34, y, 17, Color{245, 245, 236, 255});
        y += 26;
    }

    if (const auto* settlement = SettlementById(state.simulation.Settlements(), state.selected_settlement_id)) {
        DrawText(("Settlement " + std::to_string(settlement->id) + " " + ToString(settlement->level)).c_str(), 34, y,
                 17, Color{238, 218, 144, 255});
        y += 22;
        DrawText(("Pop " + std::to_string(settlement->population) + "  Age " +
                  std::to_string(settlement->turns_since_founded))
                     .c_str(),
                 34, y, 17, Color{238, 218, 144, 255});
        y += 22;
        DrawText(("Food " + Fixed(settlement->stockpile.food) + "  Wood " + Fixed(settlement->stockpile.wood)).c_str(),
                 34, y, 17, Color{238, 218, 144, 255});
        y += 22;
        DrawText(("Local food " + Fixed(LocalFoodOutput(state.simulation.GetWorld(), *settlement, state.simulation_params)) +
                  "  wood " + Fixed(LocalWoodOutput(state.simulation.GetWorld(), *settlement, state.simulation_params)))
                     .c_str(),
                 34, y, 17, Color{238, 218, 144, 255});
    }
}

void DrawDebugPanel(const AppState& state) {
    if (!state.show_debug_panel) {
        return;
    }

    DrawPanelBackground(18, 18, 460, 560);
    DrawText("Oikumene / The Habitable World", 34, 34, 22, RAYWHITE);
    DrawText(("Layer: " + ToString(state.current_layer)).c_str(), 34, 68, 18, Color{220, 225, 230, 255});
    DrawText(("Seed: " + std::to_string(state.generation_params.seed)).c_str(), 34, 94, 18, Color{184, 194, 202, 255});
    DrawText(("World: " + std::to_string(state.simulation.GetWorld().Width()) + " x " +
              std::to_string(state.simulation.GetWorld().Height()))
                 .c_str(),
             34, 120, 18, Color{184, 194, 202, 255});
    DrawText(("Python Agent: " + StatusText(state.health)).c_str(), 34, 146, 18, StatusColor(state.health.online));
    DrawText(("Sim: " + state.simulation.StatusSummary()).c_str(), 34, 172, 18, Color{184, 194, 202, 255});
    DrawText(("Auto-run: " + std::string(state.auto_run ? "on" : "off") + "  TPS " +
              Fixed(state.config.simulation.turns_per_second, 1) + "  Events: " +
              std::to_string(state.simulation.Events().Size()))
                 .c_str(),
             34, 196, 18, Color{184, 194, 202, 255});

    DrawText(("Land " + Fixed(state.report.land_ratio * 100.0F, 1) + "%  Ocean " +
              Fixed((1.0F - state.report.land_ratio) * 100.0F, 1) + "%  Rivers " +
              std::to_string(state.report.river_tiles))
                 .c_str(),
             34, 232, 16, Color{202, 211, 218, 255});
    DrawText(("Forest " + Fixed(state.report.forest_ratio * 100.0F, 1) + "%  Desert " +
              Fixed(state.report.desert_ratio * 100.0F, 1) + "%  Top settle " + Fixed(state.report.top_settlement_score))
                 .c_str(),
             34, 256, 16, Color{202, 211, 218, 255});
    DrawText(("Bands " + std::to_string(ActiveBandCount(state.simulation.Bands())) + "/" +
              std::to_string(state.simulation.Bands().size()) + "  Camps " +
              std::to_string(static_cast<int>(state.simulation.Settlements().size()) -
                             VillageCount(state.simulation.Settlements())) +
              "  Villages " + std::to_string(VillageCount(state.simulation.Settlements())))
                 .c_str(),
             34, 280, 16, Color{202, 211, 218, 255});

    DrawText("Space step  N 10 turns  Shift+N 100  A auto  B reset", 34, 310, 15, Color{152, 164, 174, 255});
    DrawText("Tab panel  F1 help  F11 fullscreen  P screenshot  M report", 34, 332, 15, Color{152, 164, 174, 255});
    if (!state.status_message.empty()) {
        DrawText(state.status_message.c_str(), 34, 356, 15, Color{180, 202, 225, 255});
    }

    int y = 388;
    DrawInspectorDetails(state, y);
}

void DrawHelpPanel() {
    DrawPanelBackground(GetScreenWidth() - 390, 18, 364, 246);
    DrawText("Help", GetScreenWidth() - 370, 36, 22, RAYWHITE);
    DrawText("1-7      switch map layers", GetScreenWidth() - 370, 72, 16, Color{202, 211, 218, 255});
    DrawText("R        generate next seed", GetScreenWidth() - 370, 96, 16, Color{202, 211, 218, 255});
    DrawText("B        reset bands on current world", GetScreenWidth() - 370, 120, 16, Color{202, 211, 218, 255});
    DrawText("Space    step one turn", GetScreenWidth() - 370, 144, 16, Color{202, 211, 218, 255});
    DrawText("N        step 10 turns", GetScreenWidth() - 370, 168, 16, Color{202, 211, 218, 255});
    DrawText("Shift+N  step 100 turns", GetScreenWidth() - 370, 192, 16, Color{202, 211, 218, 255});
    DrawText("A        toggle auto-run", GetScreenWidth() - 370, 216, 16, Color{202, 211, 218, 255});
    DrawText("F11      toggle fullscreen", GetScreenWidth() - 370, 240, 16, Color{202, 211, 218, 255});
}

}  // namespace

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
    state.auto_run = config_.simulation.auto_run;
    BuildSimulation(state, config_.simulation.default_seed);
    state.health = state.remote_provider.CheckHealth();

    while (!WindowShouldClose()) {
        UpdateHover(state);
        HandleInput(state);
        AdvanceAutoRun(state);

        BeginDrawing();
        ClearBackground(Color{18, 22, 26, 255});
        state.renderer.Draw(state.simulation.GetWorld(), state.camera, state.current_layer, state.hover_tile);
        state.renderer.DrawEntities(state.simulation.Bands(), state.simulation.Settlements(), state.camera);
        DrawDebugPanel(state);
        if (state.show_help_panel) {
            DrawHelpPanel();
        }
        DrawEventLogPanel(state);
        EndDrawing();

        if (state.pending_screenshot.has_value()) {
            TakeScreenshot(state.pending_screenshot->string().c_str());
            state.status_message = "Exported screenshot: " + state.pending_screenshot->string();
            state.pending_screenshot.reset();
        }
    }

    CloseWindow();
    return 0;
}

}  // namespace oikumene
