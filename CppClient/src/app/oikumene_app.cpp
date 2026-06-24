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
#include "oikumene/sim/tech_effects.hpp"
#include "oikumene/ui/legend_panel.hpp"
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

Rectangle HudBounds() {
    return Rectangle{14.0F, 14.0F, 496.0F, 174.0F};
}

Rectangle DebugPanelBounds() {
    return Rectangle{18.0F, 154.0F, 460.0F, 532.0F};
}

Rectangle HelpPanelBounds() {
    return Rectangle{static_cast<float>(GetScreenWidth() - 390), 18.0F, 364.0F, 342.0F};
}

Rectangle LegendPanelBounds() {
    constexpr int width = 920;
    constexpr int height = 660;
    return Rectangle{std::max(18.0F, (static_cast<float>(GetScreenWidth()) - static_cast<float>(width)) * 0.5F),
                     std::max(18.0F, (static_cast<float>(GetScreenHeight()) - static_cast<float>(height)) * 0.5F),
                     std::min(static_cast<float>(width), static_cast<float>(GetScreenWidth() - 36)),
                     std::min(static_cast<float>(height), static_cast<float>(GetScreenHeight() - 36))};
}

Rectangle EventLogPanelBounds() {
    constexpr int width = 430;
    constexpr int height = 314;
    return Rectangle{static_cast<float>(GetScreenWidth() - width - 18),
                     static_cast<float>(GetScreenHeight() - height - 18), static_cast<float>(width),
                     static_cast<float>(height)};
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
    const Color fill = active ? Color{52, 112, 86, 230} : hovered ? Color{54, 62, 70, 230} : Color{32, 38, 44, 220};
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

const Polity* PolityById(const std::vector<Polity>& polities, PolityId id) {
    for (const auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

const Route* RouteById(const std::vector<Route>& routes, int id) {
    for (const auto& route : routes) {
        if (route.id == id) {
            return &route;
        }
    }
    return nullptr;
}

int ContestedTileCount(const World& world) {
    int count = 0;
    for (const auto& tile : world.Tiles()) {
        count += tile.is_contested ? 1 : 0;
    }
    return count;
}

int RouteTileCount(const World& world) {
    int count = 0;
    for (const auto& tile : world.Tiles()) {
        count += tile.has_route ? 1 : 0;
    }
    return count;
}

float ControlledLandRatio(const World& world) {
    int land = 0;
    int controlled = 0;
    for (const auto& tile : world.Tiles()) {
        if (tile.is_ocean || tile.is_lake) {
            continue;
        }
        ++land;
        controlled += tile.controller_polity_id != kInvalidPolityId ? 1 : 0;
    }
    return land <= 0 ? 0.0F : static_cast<float>(controlled) / static_cast<float>(land);
}

int LargestPolityPopulation(const std::vector<Polity>& polities) {
    int largest = 0;
    for (const auto& polity : polities) {
        largest = std::max(largest, polity.population);
    }
    return largest;
}

float AverageUnlockedTechs(const std::vector<Polity>& polities) {
    if (polities.empty()) {
        return 0.0F;
    }
    int unlocked = 0;
    for (const auto& polity : polities) {
        unlocked += static_cast<int>(polity.research.unlocked.size());
    }
    return static_cast<float>(unlocked) / static_cast<float>(polities.size());
}

float TechUnlockRate(const std::vector<Polity>& polities, TechId tech) {
    if (polities.empty()) {
        return 0.0F;
    }
    int count = 0;
    for (const auto& polity : polities) {
        count += HasTech(polity.research, tech) ? 1 : 0;
    }
    return static_cast<float>(count) / static_cast<float>(polities.size());
}

std::string TechList(const ResearchState& research, std::size_t limit = 4) {
    if (research.unlocked.empty()) {
        return "None";
    }
    std::string text;
    const std::size_t count = std::min(limit, research.unlocked.size());
    for (std::size_t i = 0; i < count; ++i) {
        if (!text.empty()) {
            text += ", ";
        }
        text += ToString(research.unlocked[i]);
    }
    if (research.unlocked.size() > limit) {
        text += ", ...";
    }
    return text;
}

std::string EffectsSummary(const TechEffects& effects) {
    const auto descriptions = DescribeActiveEffects(effects);
    if (descriptions.empty()) {
        return "None";
    }
    std::string text;
    for (std::size_t i = 0; i < descriptions.size() && i < 4; ++i) {
        if (!text.empty()) {
            text += ", ";
        }
        text += descriptions[i];
    }
    if (descriptions.size() > 4) {
        text += ", ...";
    }
    return text;
}

std::vector<Rectangle> UiCaptureRectangles(const AppState& state) {
    std::vector<Rectangle> rectangles;
    rectangles.push_back(HudBounds());
    rectangles.push_back(PlaybackBarBounds());
    if (state.show_debug_panel) {
        rectangles.push_back(DebugPanelBounds());
    }
    if (state.show_help_panel) {
        rectangles.push_back(HelpPanelBounds());
    }
    if (state.show_legend_panel) {
        rectangles.push_back(LegendPanelBounds());
    }
    if (state.show_event_log_panel) {
        rectangles.push_back(EventLogPanelBounds());
    }
    return rectangles;
}

bool UiCapturesMouse(const AppState& state, Vector2 mouse) {
    return PointInAnyRectangle(mouse, UiCaptureRectangles(state));
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

void SelectAtHover(AppState& state) {
    if (!state.hover_tile.has_value()) {
        ApplySelection(state, ClearSelection());
        state.selected_band_id = -1;
        state.selected_settlement_id = -1;
        return;
    }

    const auto [x, y] = *state.hover_tile;
    if (const auto* settlement = SettlementAt(state.simulation.Settlements(), x, y)) {
        ApplySelection(state, Selection{.kind = SelectionKind::Settlement, .id = settlement->id, .x = x, .y = y});
        return;
    }
    if (const auto* band = ActiveBandAt(state.simulation.Bands(), x, y)) {
        ApplySelection(state, Selection{.kind = SelectionKind::Band, .id = band->id, .x = x, .y = y});
        return;
    }
    ApplySelection(state, SelectAtTile(state.simulation.GetWorld(), state.simulation.Bands(),
                                       state.simulation.Settlements(), x, y));
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
    if (IsKeyPressed(KEY_EIGHT)) {
        state.current_layer = MapLayer::PolityControl;
    }
    if (IsKeyPressed(KEY_NINE)) {
        state.current_layer = MapLayer::RouteNetwork;
    }
}

void HandleInput(AppState& state) {
    state.camera.SetViewport(
        Rectangle{0.0F, 0.0F, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())});
    const Vector2 mouse = GetMousePosition();
    const bool mouse_captured = UiCapturesMouse(state, mouse);
    state.camera.Update(!mouse_captured, true);
    HandleLayerHotkeys(state);

    if (IsKeyPressed(KEY_R)) {
        BuildSimulation(state, NextSeed(state.generation_params.seed));
        state.camera.FitWorld(state.simulation.GetWorld().Width(), state.simulation.GetWorld().Height());
        state.status_message = "Generated seed " + std::to_string(state.generation_params.seed);
    }
    if (IsKeyPressed(KEY_B)) {
        state.simulation.InitializeBands(state.config.simulation.initial_bands);
        ApplySelection(state, ClearSelection());
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
        if (CenterOnSelection(state)) {
            state.status_message = "Centered selection";
        } else {
            state.camera.FitWorld(state.simulation.GetWorld().Width(), state.simulation.GetWorld().Height());
            state.status_message = "Fit world";
        }
    }
    if (IsKeyPressed(KEY_HOME) || IsKeyPressed(KEY_F)) {
        state.camera.FitWorld(state.simulation.GetWorld().Width(), state.simulation.GetWorld().Height());
        state.status_message = "Fit world";
    }
    if (IsKeyPressed(KEY_F1)) {
        state.show_help_panel = !state.show_help_panel;
    }
    if (IsKeyPressed(KEY_F2)) {
        state.show_legend_panel = !state.show_legend_panel;
    }
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !mouse_captured) {
        SelectAtMouse(state, mouse);
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
    const Vector2 mouse = GetMousePosition();
    if (UiCapturesMouse(state, mouse)) {
        return;
    }
    if (state.camera.ScreenToTile(mouse, state.simulation.GetWorld(), state.hover_x, state.hover_y)) {
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
        std::string line =
            Truncate("T" + std::to_string(event.turn) + " " + ToString(event.type) + ": " + event.summary, 60);
        DrawText(line.c_str(), x + 16, line_y, 15, Color{202, 211, 218, 255});
        line_y += 21;
    }
}

void DrawHud(const AppState& state) {
    const Rectangle bounds = HudBounds();
    DrawPanelBackground(static_cast<int>(bounds.x), static_cast<int>(bounds.y), static_cast<int>(bounds.width),
                        static_cast<int>(bounds.height));

    DrawText("Oikumene", 30, 28, 22, RAYWHITE);
    DrawText(
        ("Layer " + ToString(state.current_layer) + "  Seed " + std::to_string(state.generation_params.seed)).c_str(),
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
              "  Space step  N +10  Tab details  F2 legend  Home fit")
                 .c_str(),
             30, 106, 15, Color{152, 164, 174, 255});
    DrawText(("Polities " + std::to_string(state.simulation.Polities().size()) + "  Control " +
              Fixed(ControlledLandRatio(state.simulation.GetWorld()) * 100.0F, 1) + "%  Contested " +
              std::to_string(ContestedTileCount(state.simulation.GetWorld())) + "  Largest pop " +
              std::to_string(LargestPolityPopulation(state.simulation.Polities())))
                 .c_str(),
             30, 130, 15, Color{184, 194, 202, 255});
    DrawText((std::string(state.simulation_params.enable_routes ? "Routes " : "Routes off ") +
              std::to_string(state.simulation.Routes().size()) + "/" +
              std::to_string(RouteTileCount(state.simulation.GetWorld())) + "  Tech avg " +
              Fixed(AverageUnlockedTechs(state.simulation.Polities()), 1) + "  Mining " +
              Fixed(TechUnlockRate(state.simulation.Polities(), TechId::Mining) * 100.0F, 0) + "%  Roads " +
              Fixed(TechUnlockRate(state.simulation.Polities(), TechId::Roads) * 100.0F, 0) + "%")
                 .c_str(),
             30, 154, 15, Color{184, 194, 202, 255});
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
        ApplySelection(state, ClearSelection());
        state.status_message = "Reset bands on current world";
    }
}

void DrawInspectorDetails(const AppState& state, int& y) {
    std::optional<std::pair<int, int>> inspected_tile;
    if (state.selection.kind == SelectionKind::Tile || state.selection.kind == SelectionKind::ImprovementTile ||
        state.selection.kind == SelectionKind::RouteTile) {
        inspected_tile = std::pair<int, int>{state.selection.x, state.selection.y};
    } else if (state.hover_tile.has_value()) {
        inspected_tile = state.hover_tile;
    }

    DrawText(("Selection: " + ToString(state.selection.kind)).c_str(), 34, y, 17, Color{220, 225, 230, 255});
    y += 24;

    if (inspected_tile.has_value()) {
        const auto [tile_x, tile_y] = *inspected_tile;
        const auto& tile = state.simulation.GetWorld().At(tile_x, tile_y);
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
        DrawText(
            ("Improve " + ToString(tile.improvement) + "  WorkedBy " + std::to_string(tile.worked_by_settlement_id))
                .c_str(),
            34, y, 17, Color{202, 211, 218, 255});
        y += 22;
        DrawText(("Soil " + Fixed(tile.soil_quality) + "  Forest " + Fixed(tile.forest_cover)).c_str(), 34, y, 17,
                 Color{202, 211, 218, 255});
        y += 26;
        DrawText(("Controller " + std::to_string(tile.controller_polity_id) + "  Control " +
                  Fixed(tile.control_strength, 1) + "  Contested " + std::string(tile.is_contested ? "yes" : "no"))
                     .c_str(),
                 34, y, 17, Color{202, 211, 218, 255});
        y += 26;
        if (tile.has_route) {
            DrawText(("Route " + std::to_string(tile.route_id) + "  " + ToString(tile.route_kind) + "  Polity " +
                      std::to_string(tile.route_polity_id) + "  Quality " + Fixed(tile.route_quality, 2))
                         .c_str(),
                     34, y, 17, Color{224, 202, 136, 255});
            y += 22;
            if (const auto* route = RouteById(state.simulation.Routes(), tile.route_id)) {
                DrawText(("Purpose " + ToString(route->purpose) + "  Maint " + Fixed(route->maintenance, 2) + "  ROI " +
                          Fixed(route->roi, 2))
                             .c_str(),
                         34, y, 17, Color{224, 202, 136, 255});
                y += 22;
                DrawText(Truncate("Reason: " + route->reason, 58).c_str(), 34, y, 16, Color{224, 202, 136, 255});
                y += 24;
            }
        }
    }

    const int selected_band_id = state.selection.kind == SelectionKind::Band ? state.selection.id : -1;
    if (const auto* band = BandById(state.simulation.Bands(), selected_band_id)) {
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
        DrawText(Truncate("Reason: " + band->last_decision_reason, 58).c_str(), 34, y, 16, Color{220, 226, 210, 255});
        y += 26;
    }

    const int selected_settlement_id = state.selection.kind == SelectionKind::Settlement ? state.selection.id : -1;
    if (const auto* settlement = SettlementById(state.simulation.Settlements(), selected_settlement_id)) {
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
        y += 22;
        DrawText(
            ("Polity " + std::to_string(settlement->polity_id) + (settlement->is_capital ? "  Capital" : "")).c_str(),
            34, y, 17, Color{238, 218, 144, 255});
        y += 22;
        if (const auto* polity = PolityById(state.simulation.Polities(), settlement->polity_id)) {
            DrawText((polity->name + " " + ToString(polity->level)).c_str(), 34, y, 17, Color{238, 218, 144, 255});
            y += 22;
            DrawText(("Members " + std::to_string(polity->member_settlement_ids.size()) + "  Control tiles " +
                      std::to_string(polity->controlled_tile_count))
                         .c_str(),
                     34, y, 17, Color{238, 218, 144, 255});
            y += 22;
            DrawText(("Admin range " + Fixed(polity->admin_range, 1) + "  Food " + Fixed(polity->food, 0) + "  Wood " +
                      Fixed(polity->wood, 0))
                         .c_str(),
                     34, y, 17, Color{238, 218, 144, 255});
            y += 22;
            DrawText(("Admin " + Fixed(polity->admin_load, 1) + "/" + Fixed(polity->admin_capacity, 1) + "  Over " +
                      Fixed(polity->overextension * 100.0F, 0) + "%  Stability " +
                      Fixed(polity->stability * 100.0F, 0) + "%")
                         .c_str(),
                     34, y, 17, Color{238, 218, 144, 255});
            y += 22;
            DrawText(("Budget food " + Fixed(polity->budget.food_income, 1) + "  wood " +
                      Fixed(polity->budget.wood_income, 1) + "  wealth " + Fixed(polity->budget.wealth_surplus, 1))
                         .c_str(),
                     34, y, 17, Color{238, 218, 144, 255});
            y += 22;
            const float current_cost = TechCost(polity->research.current);
            DrawText(("Research " + ToString(polity->research.current) + "  " + Fixed(polity->research.progress, 1) +
                      "/" + Fixed(current_cost, 0) + "  K+" + Fixed(polity->knowledge_income, 1))
                         .c_str(),
                     34, y, 17, Color{198, 228, 245, 255});
            y += 22;
            DrawText(Truncate("Techs: " + TechList(polity->research), 58).c_str(), 34, y, 17,
                     Color{198, 228, 245, 255});
            y += 22;
            DrawText(Truncate("Effects: " + EffectsSummary(ComputeTechEffects(polity->research)), 58).c_str(), 34, y,
                     17, Color{198, 228, 245, 255});
            y += 22;
            DrawText(
                ("Tools " + Fixed(polity->tool_efficiency, 2) + "  Military " + Fixed(polity->military_potential, 1))
                    .c_str(),
                34, y, 17, Color{198, 228, 245, 255});
            y += 22;
            DrawText(("Routes " + std::to_string(polity->route_ids.size()) + "  Maint " +
                      Fixed(polity->route_maintenance, 2) + "  Conn villages " +
                      std::to_string(polity->connected_settlements) + "  Mines " +
                      std::to_string(polity->connected_mines))
                         .c_str(),
                     34, y, 17, Color{224, 202, 136, 255});
        }
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
              Fixed(state.controller.TurnsPerSecond(), 1) +
              "  Events: " + std::to_string(state.simulation.Events().Size()))
                 .c_str(),
             34, 332, 18, Color{184, 194, 202, 255});

    DrawText(("Land " + Fixed(state.report.land_ratio * 100.0F, 1) + "%  Ocean " +
              Fixed((1.0F - state.report.land_ratio) * 100.0F, 1) + "%  Rivers " +
              std::to_string(state.report.river_tiles))
                 .c_str(),
             34, 368, 16, Color{202, 211, 218, 255});
    DrawText(("Forest " + Fixed(state.report.forest_ratio * 100.0F, 1) + "%  Desert " +
              Fixed(state.report.desert_ratio * 100.0F, 1) + "%  Top settle " +
              Fixed(state.report.top_settlement_score))
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
    DrawText("WASD/arrows pan  right/middle drag  wheel zoom", 34, 468, 15, Color{152, 164, 174, 255});
    DrawText("Tab details  E events  C selected  Home/F fit  P shot", 34, 490, 15, Color{152, 164, 174, 255});
    if (!state.status_message.empty()) {
        DrawText(state.status_message.c_str(), 34, 512, 15, Color{180, 202, 225, 255});
    }

    int y = 542;
    DrawInspectorDetails(state, y);
}

void DrawHelpPanel() {
    DrawPanelBackground(GetScreenWidth() - 390, 18, 364, 342);
    DrawText("Help", GetScreenWidth() - 370, 36, 22, RAYWHITE);
    DrawText("1-9      switch map layers", GetScreenWidth() - 370, 72, 16, Color{202, 211, 218, 255});
    DrawText("R        generate next seed", GetScreenWidth() - 370, 96, 16, Color{202, 211, 218, 255});
    DrawText("B        reset bands on current world", GetScreenWidth() - 370, 120, 16, Color{202, 211, 218, 255});
    DrawText("Space    step one turn", GetScreenWidth() - 370, 144, 16, Color{202, 211, 218, 255});
    DrawText("N        step 10 turns", GetScreenWidth() - 370, 168, 16, Color{202, 211, 218, 255});
    DrawText("Shift+N  step 100 turns", GetScreenWidth() - 370, 192, 16, Color{202, 211, 218, 255});
    DrawText("A        toggle auto-run", GetScreenWidth() - 370, 216, 16, Color{202, 211, 218, 255});
    DrawText("E        toggle events panel", GetScreenWidth() - 370, 240, 16, Color{202, 211, 218, 255});
    DrawText("F2       toggle legend panel", GetScreenWidth() - 370, 264, 16, Color{202, 211, 218, 255});
    DrawText("WASD/arrows pan, right-drag map", GetScreenWidth() - 370, 288, 16, Color{202, 211, 218, 255});
    DrawText("C center selected, Home/F fit world", GetScreenWidth() - 370, 312, 16, Color{202, 211, 218, 255});
}

} // namespace

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
        UpdateHover(state);
        AdvanceAutoRun(state);

        BeginDrawing();
        ClearBackground(Color{18, 22, 26, 255});
        state.renderer.Draw(state.simulation.GetWorld(), state.camera, state.current_layer, state.hover_tile,
                            state.selection);
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

        if (state.pending_screenshot.has_value()) {
            TakeScreenshot(state.pending_screenshot->string().c_str());
            state.status_message = "Exported screenshot: " + state.pending_screenshot->string();
            state.pending_screenshot.reset();
        }
    }

    CloseWindow();
    return 0;
}

} // namespace oikumene
