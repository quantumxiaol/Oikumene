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
    DrawRectangle(x, y, width, height, Color{18, 22, 26, 190});
    DrawRectangleLines(x, y, width, height, Color{78, 86, 94, 205});
}

Rectangle PlaybackBarBounds() {
    return Rectangle{16.0F, static_cast<float>(GetScreenHeight() - 58), 718.0F, 42.0F};
}

Rectangle PlaybackButton(int index, float width) {
    const Rectangle bar = PlaybackBarBounds();
    float x = bar.x + 12.0F;
    constexpr float gap = 8.0F;
    const float widths[] = {98.0F, 66.0F, 66.0F, 72.0F, 64.0F, 92.0F, 64.0F, 108.0F};
    for (int i = 0; i < index; ++i) {
        x += widths[i] + gap;
    }
    return Rectangle{x, bar.y + 8.0F, width, 26.0F};
}

bool DrawButton(Rectangle rect, const std::string& label, bool active = false) {
    const Vector2 mouse = GetMousePosition();
    const bool hovered = CheckCollisionPointRec(mouse, rect);
    const Color fill = active ? Color{52, 112, 86, 230}
                              : hovered ? Color{54, 62, 70, 230} : Color{32, 38, 44, 220};
    DrawRectangleRec(rect, fill);
    DrawRectangleLinesEx(rect, 1.0F, Color{96, 106, 116, 230});
    DrawText(label.c_str(), static_cast<int>(rect.x + 10.0F), static_cast<int>(rect.y + 6.0F), 14, RAYWHITE);
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
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

int TotalPopulation(const std::vector<Band>& bands, const std::vector<Settlement>& settlements) {
    int total = 0;
    for (const auto& band : bands) {
        if (band.active) {
            total += band.population;
        }
    }
    for (const auto& settlement : settlements) {
        total += settlement.population;
    }
    return total;
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

std::string Truncate(std::string text, std::size_t limit) {
    if (text.size() <= limit) {
        return text;
    }
    return text.substr(0, limit > 3 ? limit - 3 : limit) + "...";
}

void StepTurns(AppState& state, int turns) {
    state.controller.StepTurns(state.simulation, turns);
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
    state.controller.SetRunning(false);
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
        state.camera.FitToWorld(state.simulation.GetWorld(), GetScreenWidth(), GetScreenHeight());
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
        state.controller.ToggleRunning();
    }
    if (IsKeyPressed(KEY_MINUS)) {
        state.controller.SetTurnsPerSecond(state.controller.TurnsPerSecond() - 1.0F);
    }
    if (IsKeyPressed(KEY_EQUAL)) {
        state.controller.SetTurnsPerSecond(state.controller.TurnsPerSecond() + 1.0F);
    }
    if (IsKeyPressed(KEY_TAB)) {
        state.show_debug_panel = !state.show_debug_panel;
    }
    if (IsKeyPressed(KEY_E)) {
        state.show_event_log_panel = !state.show_event_log_panel;
    }
    if (IsKeyPressed(KEY_C)) {
        state.camera.FitToWorld(state.simulation.GetWorld(), GetScreenWidth(), GetScreenHeight());
        state.status_message = "Centered map";
    }
    if (IsKeyPressed(KEY_F1)) {
        state.show_help_panel = !state.show_help_panel;
    }
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(GetMousePosition(), PlaybackBarBounds())) {
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
    state.controller.Update(GetFrameTime(), state.simulation);
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
    const int height = 314;
    const int x = GetScreenWidth() - width - 18;
    const int y = GetScreenHeight() - height - 18;
    DrawPanelBackground(x, y, width, height);
    DrawText("Recent Events", x + 16, y + 16, 20, RAYWHITE);

    const auto& events = state.simulation.Events().Events();
    if (events.empty()) {
        DrawText("No events yet. Press Space/N or A to advance.", x + 16, y + 52, 15, Color{202, 211, 218, 255});
        return;
    }

    int line_y = y + 48;
    const int first = std::max(0, static_cast<int>(events.size()) - 12);
    for (int i = first; i < static_cast<int>(events.size()); ++i) {
        const auto& event = events[static_cast<std::size_t>(i)];
        std::string line = Truncate("T" + std::to_string(event.turn) + " " + ToString(event.type) + ": " + event.summary, 60);
        DrawText(line.c_str(), x + 16, line_y, 15, Color{202, 211, 218, 255});
        line_y += 21;
    }
}

void DrawHud(const AppState& state) {
    const int width = 438;
    const int height = 126;
    DrawPanelBackground(14, 14, width, height);

    DrawText("Oikumene", 30, 28, 22, RAYWHITE);
    DrawText(("Layer " + ToString(state.current_layer) + "  Seed " + std::to_string(state.generation_params.seed)).c_str(),
             30, 58, 16, Color{220, 225, 230, 255});
    DrawText(("Turn " + std::to_string(state.simulation.CurrentTurn()) + "  Bands " +
              std::to_string(ActiveBandCount(state.simulation.Bands())) + "/" +
              std::to_string(state.simulation.Bands().size()) + "  Camps " +
              std::to_string(static_cast<int>(state.simulation.Settlements().size()) -
                             VillageCount(state.simulation.Settlements())) +
              "  Villages " + std::to_string(VillageCount(state.simulation.Settlements())) + "  Pop " +
              std::to_string(TotalPopulation(state.simulation.Bands(), state.simulation.Settlements())))
                 .c_str(),
             30, 82, 16, Color{202, 211, 218, 255});
    DrawText(("A " + std::string(state.controller.IsRunning() ? "run" : "pause") +
              "  Space step  N +10  Tab details  E events  C center")
                 .c_str(),
             30, 106, 15, Color{152, 164, 174, 255});
}

void DrawPlaybackBar(AppState& state) {
    const Rectangle bar = PlaybackBarBounds();
    DrawPanelBackground(static_cast<int>(bar.x), static_cast<int>(bar.y), static_cast<int>(bar.width),
                        static_cast<int>(bar.height));

    if (DrawButton(PlaybackButton(0, 98.0F), state.controller.IsRunning() ? "Pause" : "Play",
                   state.controller.IsRunning())) {
        state.controller.ToggleRunning();
    }
    if (DrawButton(PlaybackButton(1, 66.0F), "Step")) {
        StepTurns(state, 1);
    }
    if (DrawButton(PlaybackButton(2, 66.0F), "+10")) {
        StepTurns(state, 10);
    }
    if (DrawButton(PlaybackButton(3, 72.0F), "+100")) {
        StepTurns(state, 100);
    }
    if (DrawButton(PlaybackButton(4, 64.0F), "TPS -")) {
        state.controller.SetTurnsPerSecond(state.controller.TurnsPerSecond() - 1.0F);
    }
    DrawButton(PlaybackButton(5, 92.0F), "TPS " + Fixed(state.controller.TurnsPerSecond(), 1));
    if (DrawButton(PlaybackButton(6, 64.0F), "TPS +")) {
        state.controller.SetTurnsPerSecond(state.controller.TurnsPerSecond() + 1.0F);
    }
    if (DrawButton(PlaybackButton(7, 108.0F), "Reset Bands")) {
        state.simulation.InitializeBands(state.config.simulation.initial_bands);
        state.selected_band_id = -1;
        state.selected_settlement_id = -1;
        state.status_message = "Reset bands on current world";
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
        y += 22;
        DrawText(("Improve " + ToString(tile.improvement) + "  WorkedBy " +
                  std::to_string(tile.worked_by_settlement_id))
                     .c_str(),
                 34, y, 17, Color{202, 211, 218, 255});
        y += 22;
        DrawText(("Soil " + Fixed(tile.soil_quality) + "  Forest " + Fixed(tile.forest_cover)).c_str(), 34, y, 17,
                 Color{202, 211, 218, 255});
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
        y += 22;
        DrawText(("Current score " + Fixed(band->current_tile_score) + "  Best seen " + Fixed(band->best_seen_score))
                     .c_str(),
                 34, y, 17, Color{245, 245, 236, 255});
        y += 22;
        DrawText(("Forage last turn " + Fixed(band->forage_yield_last_turn)).c_str(), 34, y, 17,
                 Color{245, 245, 236, 255});
        y += 22;
        DrawText(Truncate("Reason: " + band->last_decision_reason, 58).c_str(), 34, y, 16,
                 Color{220, 226, 210, 255});
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
        DrawText(("Food out " + Fixed(settlement->local_food_output_last_turn) + "  Wood out " +
                  Fixed(settlement->local_wood_output_last_turn))
                     .c_str(),
                 34, y, 17, Color{238, 218, 144, 255});
        y += 22;
        DrawText(("Consumption " + Fixed(settlement->food_consumption_last_turn) + "  Upgrade " +
                  Fixed(settlement->upgrade_readiness * 100.0F, 0) + "%")
                     .c_str(),
                 34, y, 17, Color{238, 218, 144, 255});
        y += 22;
        DrawText(("Ore out " + Fixed(settlement->ore_output_last_turn) + "  Worked " +
                  std::to_string(settlement->worked_tile_count))
                     .c_str(),
                 34, y, 17, Color{238, 218, 144, 255});
        y += 22;
        DrawText(("Capacity " + Fixed(settlement->carrying_capacity, 0) + "  Ratio " +
                  Fixed(settlement->carrying_capacity_ratio * 100.0F, 0) + "%")
                     .c_str(),
                 34, y, 17, Color{238, 218, 144, 255});
    }
}

void DrawDebugPanel(const AppState& state) {
    if (!state.show_debug_panel) {
        return;
    }

    DrawPanelBackground(18, 154, 460, 532);
    DrawText("Details", 34, 170, 22, RAYWHITE);
    DrawText(("Layer: " + ToString(state.current_layer)).c_str(), 34, 204, 18, Color{220, 225, 230, 255});
    DrawText(("Seed: " + std::to_string(state.generation_params.seed)).c_str(), 34, 230, 18, Color{184, 194, 202, 255});
    DrawText(("World: " + std::to_string(state.simulation.GetWorld().Width()) + " x " +
              std::to_string(state.simulation.GetWorld().Height()))
                 .c_str(),
             34, 256, 18, Color{184, 194, 202, 255});
    DrawText(("Python Agent: " + StatusText(state.health)).c_str(), 34, 282, 18, StatusColor(state.health.online));
    DrawText(("Sim: " + state.simulation.StatusSummary()).c_str(), 34, 308, 18, Color{184, 194, 202, 255});
    DrawText(("Auto-run: " + std::string(state.controller.IsRunning() ? "on" : "off") + "  TPS " +
              Fixed(state.controller.TurnsPerSecond(), 1) + "  Events: " +
              std::to_string(state.simulation.Events().Size()))
                 .c_str(),
             34, 332, 18, Color{184, 194, 202, 255});

    DrawText(("Land " + Fixed(state.report.land_ratio * 100.0F, 1) + "%  Ocean " +
              Fixed((1.0F - state.report.land_ratio) * 100.0F, 1) + "%  Rivers " +
              std::to_string(state.report.river_tiles))
                 .c_str(),
             34, 368, 16, Color{202, 211, 218, 255});
    DrawText(("Forest " + Fixed(state.report.forest_ratio * 100.0F, 1) + "%  Desert " +
              Fixed(state.report.desert_ratio * 100.0F, 1) + "%  Top settle " + Fixed(state.report.top_settlement_score))
                 .c_str(),
             34, 392, 16, Color{202, 211, 218, 255});
    DrawText(("Bands " + std::to_string(ActiveBandCount(state.simulation.Bands())) + "/" +
              std::to_string(state.simulation.Bands().size()) + "  Camps " +
              std::to_string(static_cast<int>(state.simulation.Settlements().size()) -
                             VillageCount(state.simulation.Settlements())) +
              "  Villages " + std::to_string(VillageCount(state.simulation.Settlements())) + "  Pop " +
              std::to_string(TotalPopulation(state.simulation.Bands(), state.simulation.Settlements())))
                 .c_str(),
             34, 416, 16, Color{202, 211, 218, 255});

    DrawText("Space step  N 10 turns  Shift+N 100  A auto  B reset", 34, 446, 15, Color{152, 164, 174, 255});
    DrawText("Tab details  E events  C center  F1 help  P screenshot", 34, 468, 15, Color{152, 164, 174, 255});
    if (!state.status_message.empty()) {
        DrawText(state.status_message.c_str(), 34, 492, 15, Color{180, 202, 225, 255});
    }

    int y = 522;
    DrawInspectorDetails(state, y);
}

void DrawHelpPanel() {
    DrawPanelBackground(GetScreenWidth() - 390, 18, 364, 278);
    DrawText("Help", GetScreenWidth() - 370, 36, 22, RAYWHITE);
    DrawText("1-7      switch map layers", GetScreenWidth() - 370, 72, 16, Color{202, 211, 218, 255});
    DrawText("R        generate next seed", GetScreenWidth() - 370, 96, 16, Color{202, 211, 218, 255});
    DrawText("B        reset bands on current world", GetScreenWidth() - 370, 120, 16, Color{202, 211, 218, 255});
    DrawText("Space    step one turn", GetScreenWidth() - 370, 144, 16, Color{202, 211, 218, 255});
    DrawText("N        step 10 turns", GetScreenWidth() - 370, 168, 16, Color{202, 211, 218, 255});
    DrawText("Shift+N  step 100 turns", GetScreenWidth() - 370, 192, 16, Color{202, 211, 218, 255});
    DrawText("A        toggle auto-run", GetScreenWidth() - 370, 216, 16, Color{202, 211, 218, 255});
    DrawText("E        toggle events panel", GetScreenWidth() - 370, 240, 16, Color{202, 211, 218, 255});
    DrawText("C        center map, arrows pan", GetScreenWidth() - 370, 264, 16, Color{202, 211, 218, 255});
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
    state.controller.SetTurnsPerSecond(config_.simulation.turns_per_second);
    state.controller.SetRunning(config_.simulation.auto_run);
    BuildSimulation(state, config_.simulation.default_seed);
    state.controller.SetTurnsPerSecond(config_.simulation.turns_per_second);
    state.controller.SetRunning(config_.simulation.auto_run);
    state.camera.FitToWorld(state.simulation.GetWorld(), GetScreenWidth(), GetScreenHeight());
    state.health = state.remote_provider.CheckHealth();

    while (!WindowShouldClose()) {
        UpdateHover(state);
        HandleInput(state);
        AdvanceAutoRun(state);

        BeginDrawing();
        ClearBackground(Color{18, 22, 26, 255});
        state.renderer.Draw(state.simulation.GetWorld(), state.camera, state.current_layer, state.hover_tile);
        state.renderer.DrawEntities(state.simulation.Bands(), state.simulation.Settlements(), state.camera);
        DrawHud(state);
        DrawPlaybackBar(state);
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
