#include <chrono>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#include <raylib.h>

#include "oikumene/ai/remote_decision_provider.hpp"
#include "oikumene/core/simulation.hpp"
#include "oikumene/render/camera_controller.hpp"
#include "oikumene/render/map_layer.hpp"
#include "oikumene/render/map_renderer.hpp"
#include "oikumene/world/biome.hpp"
#include "oikumene/world/resource.hpp"
#include "oikumene/world/world_generation_report.hpp"
#include "oikumene/world/world_generation_params.hpp"
#include "oikumene/world/world_generator.hpp"

namespace {

Color StatusColor(bool online) {
    return online ? Color{80, 210, 120, 255} : Color{230, 90, 75, 255};
}

std::string StatusText(const oikumene::HealthStatus& status) {
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

std::string LayerFilename(oikumene::MapLayer layer) {
    std::string name = oikumene::ToString(layer);
    for (auto& ch : name) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return "layer_" + name + ".png";
}

bool WriteReportJson(const oikumene::WorldGenerationReport& report, std::filesystem::path path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path);
    if (!output) {
        return false;
    }
    output << oikumene::ToJson(report).dump(2) << '\n';
    return true;
}

}  // namespace

int main() {
    constexpr int screen_width = 1040;
    constexpr int screen_height = 640;

    InitWindow(screen_width, screen_height, "Oikumene - The Habitable World");
    SetTargetFPS(60);

    oikumene::Simulation simulation;
    oikumene::RemoteDecisionProvider remote_provider("127.0.0.1", 8000, std::chrono::milliseconds(900));
    auto health = remote_provider.CheckHealth();

    oikumene::WorldGenerationParams generation_params;
    generation_params.seed = 42;
    auto world = oikumene::WorldGenerator::Generate(generation_params);
    auto report = oikumene::BuildWorldGenerationReport(world);

    oikumene::CameraController camera;
    oikumene::MapRenderer renderer;
    oikumene::MapLayer current_layer = oikumene::MapLayer::Biome;
    bool show_debug_panel = true;
    bool show_help_panel = false;
    std::string status_message;
    std::optional<std::filesystem::path> pending_screenshot;

    while (!WindowShouldClose()) {
        camera.Update();

        if (IsKeyPressed(KEY_R)) {
            generation_params.seed = NextSeed(generation_params.seed);
            world = oikumene::WorldGenerator::Generate(generation_params);
            report = oikumene::BuildWorldGenerationReport(world);
            status_message = "Generated seed " + std::to_string(generation_params.seed);
        }
        if (IsKeyPressed(KEY_H)) {
            health = remote_provider.CheckHealth();
            status_message = "Python health check: " + StatusText(health);
        }
        if (IsKeyPressed(KEY_SPACE)) {
            simulation.AdvanceOneTurn();
        }
        if (IsKeyPressed(KEY_TAB)) {
            show_debug_panel = !show_debug_panel;
        }
        if (IsKeyPressed(KEY_F1)) {
            show_help_panel = !show_help_panel;
        }
        if (IsKeyPressed(KEY_ONE)) {
            current_layer = oikumene::MapLayer::Biome;
        }
        if (IsKeyPressed(KEY_TWO)) {
            current_layer = oikumene::MapLayer::Elevation;
        }
        if (IsKeyPressed(KEY_THREE)) {
            current_layer = oikumene::MapLayer::Rainfall;
        }
        if (IsKeyPressed(KEY_FOUR)) {
            current_layer = oikumene::MapLayer::Temperature;
        }
        if (IsKeyPressed(KEY_FIVE)) {
            current_layer = oikumene::MapLayer::Fertility;
        }
        if (IsKeyPressed(KEY_SIX)) {
            current_layer = oikumene::MapLayer::Resources;
        }
        if (IsKeyPressed(KEY_SEVEN)) {
            current_layer = oikumene::MapLayer::SettlementScore;
        }
        if (IsKeyPressed(KEY_M)) {
            const auto path = WorldgenDirectory(generation_params.seed) / "report.json";
            status_message = WriteReportJson(report, path) ? "Exported report: " + path.string()
                                                           : "Failed to export report: " + path.string();
        }
        if (IsKeyPressed(KEY_P)) {
            const auto directory = WorldgenDirectory(generation_params.seed);
            std::filesystem::create_directories(directory);
            pending_screenshot = directory / LayerFilename(current_layer);
        }

        std::optional<std::pair<int, int>> hover_tile;
        int hover_x = 0;
        int hover_y = 0;
        if (camera.ScreenToTile(GetMousePosition(), world, hover_x, hover_y)) {
            hover_tile = std::pair<int, int>{hover_x, hover_y};
        }

        BeginDrawing();
        ClearBackground(Color{18, 22, 26, 255});

        renderer.Draw(world, camera, current_layer, hover_tile);

        if (show_debug_panel) {
            DrawPanelBackground(18, 18, 410, hover_tile.has_value() ? 430 : 326);
            DrawText("Oikumene / The Habitable World", 34, 34, 22, RAYWHITE);
            DrawText(("Layer: " + oikumene::ToString(current_layer)).c_str(), 34, 68, 18, Color{220, 225, 230, 255});
            DrawText(("Seed: " + std::to_string(generation_params.seed)).c_str(), 34, 94, 18,
                     Color{184, 194, 202, 255});
            DrawText(("World: " + std::to_string(world.Width()) + " x " + std::to_string(world.Height())).c_str(), 34,
                     120, 18, Color{184, 194, 202, 255});
            DrawText(("Python Agent: " + StatusText(health)).c_str(), 34, 146, 18, StatusColor(health.online));
            DrawText(("Sim: " + simulation.StatusSummary()).c_str(), 34, 172, 18, Color{184, 194, 202, 255});

            DrawText(("Land " + Fixed(report.land_ratio * 100.0F, 1) + "%  Ocean " +
                      Fixed((1.0F - report.land_ratio) * 100.0F, 1) + "%  Rivers " +
                      std::to_string(report.river_tiles))
                         .c_str(),
                     34, 208, 16, Color{202, 211, 218, 255});
            DrawText(("Forest " + Fixed(report.forest_ratio * 100.0F, 1) + "%  Desert " +
                      Fixed(report.desert_ratio * 100.0F, 1) + "%  Top settle " +
                      Fixed(report.top_settlement_score))
                         .c_str(),
                     34, 232, 16, Color{202, 211, 218, 255});
            DrawText(("Resources: wood " +
                      std::to_string(report.resource_counts[oikumene::ResourceKind::Wood]) + " horse " +
                      std::to_string(report.resource_counts[oikumene::ResourceKind::Horse]) + " copper " +
                      std::to_string(report.resource_counts[oikumene::ResourceKind::Copper]) + " iron " +
                      std::to_string(report.resource_counts[oikumene::ResourceKind::ShallowIron]))
                         .c_str(),
                     34, 256, 16, Color{202, 211, 218, 255});

            DrawText("Tab panel  F1 help  P screenshot  M report", 34, 286, 16, Color{152, 164, 174, 255});
            if (!status_message.empty()) {
                DrawText(status_message.c_str(), 34, 310, 15, Color{180, 202, 225, 255});
            }

            if (hover_tile.has_value()) {
                const auto& tile = world.At(hover_x, hover_y);
                int y = 342;
                DrawText(("Tile: " + std::to_string(tile.x) + ", " + std::to_string(tile.y)).c_str(), 34, y, 17,
                         RAYWHITE);
                y += 22;
                DrawText(("Biome: " + oikumene::ToString(tile.biome)).c_str(), 34, y, 17,
                         Color{202, 211, 218, 255});
                y += 22;
                DrawText(("Resource: " + oikumene::ToString(tile.resource) + " " + Fixed(tile.resource_amount)).c_str(),
                         34, y, 17, Color{202, 211, 218, 255});
                y += 22;
                DrawText(("Elevation " + Fixed(tile.elevation) + "  Temp " + Fixed(tile.temperature)).c_str(), 34, y,
                         17, Color{202, 211, 218, 255});
                y += 22;
                DrawText(("Rain " + Fixed(tile.rainfall) + "  Fertility " + Fixed(tile.fertility)).c_str(), 34, y, 17,
                         Color{202, 211, 218, 255});
                y += 22;
                DrawText(("River " + std::string(tile.has_river ? "yes " : "no ") + Fixed(tile.river_flow) +
                          "  Settle " + Fixed(tile.settlement_score))
                             .c_str(),
                         34, y, 17, Color{202, 211, 218, 255});
            }
        }

        if (show_help_panel) {
            DrawPanelBackground(GetScreenWidth() - 380, 18, 354, 196);
            DrawText("Help", GetScreenWidth() - 360, 36, 22, RAYWHITE);
            DrawText("1-7    switch map layers", GetScreenWidth() - 360, 72, 16, Color{202, 211, 218, 255});
            DrawText("R      generate next seed", GetScreenWidth() - 360, 96, 16, Color{202, 211, 218, 255});
            DrawText("WASD   pan camera", GetScreenWidth() - 360, 120, 16, Color{202, 211, 218, 255});
            DrawText("Wheel  zoom at cursor", GetScreenWidth() - 360, 144, 16, Color{202, 211, 218, 255});
            DrawText("P/M    export screenshot/report", GetScreenWidth() - 360, 168, 16, Color{202, 211, 218, 255});
            DrawText("Tab    toggle debug panel", GetScreenWidth() - 360, 192, 16, Color{202, 211, 218, 255});
        }

        EndDrawing();

        if (pending_screenshot.has_value()) {
            TakeScreenshot(pending_screenshot->string().c_str());
            status_message = "Exported screenshot: " + pending_screenshot->string();
            pending_screenshot.reset();
        }
    }

    CloseWindow();
    return 0;
}
