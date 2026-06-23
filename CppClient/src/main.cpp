#include <chrono>
#include <cstdint>
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

    oikumene::CameraController camera;
    oikumene::MapRenderer renderer;
    oikumene::MapLayer current_layer = oikumene::MapLayer::Biome;

    while (!WindowShouldClose()) {
        camera.Update();

        if (IsKeyPressed(KEY_R)) {
            generation_params.seed = NextSeed(generation_params.seed);
            world = oikumene::WorldGenerator::Generate(generation_params);
        }
        if (IsKeyPressed(KEY_H)) {
            health = remote_provider.CheckHealth();
        }
        if (IsKeyPressed(KEY_SPACE)) {
            simulation.AdvanceOneTurn();
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

        std::optional<std::pair<int, int>> hover_tile;
        int hover_x = 0;
        int hover_y = 0;
        if (camera.ScreenToTile(GetMousePosition(), world, hover_x, hover_y)) {
            hover_tile = std::pair<int, int>{hover_x, hover_y};
        }

        BeginDrawing();
        ClearBackground(Color{18, 22, 26, 255});

        renderer.Draw(world, camera, current_layer, hover_tile);

        DrawPanelBackground(18, 18, 372, hover_tile.has_value() ? 318 : 206);
        DrawText("Oikumene / The Habitable World", 34, 34, 22, RAYWHITE);
        DrawText(("Layer: " + oikumene::ToString(current_layer)).c_str(), 34, 68, 18, Color{220, 225, 230, 255});
        DrawText(("Seed: " + std::to_string(generation_params.seed)).c_str(), 34, 94, 18, Color{184, 194, 202, 255});
        DrawText(("World: " + std::to_string(world.Width()) + " x " + std::to_string(world.Height())).c_str(), 34,
                 120, 18, Color{184, 194, 202, 255});
        DrawText(("Python Agent: " + StatusText(health)).c_str(), 34, 146, 18, StatusColor(health.online));
        DrawText(("Sim: " + simulation.StatusSummary()).c_str(), 34, 172, 18, Color{184, 194, 202, 255});

        DrawText("1-7 layers  R new seed  H health  WASD pan  wheel zoom", 34, 198, 16, Color{152, 164, 174, 255});

        if (hover_tile.has_value()) {
            const auto& tile = world.At(hover_x, hover_y);
            int y = 232;
            DrawText(("Tile: " + std::to_string(tile.x) + ", " + std::to_string(tile.y)).c_str(), 34, y, 17,
                     RAYWHITE);
            y += 22;
            DrawText(("Biome: " + oikumene::ToString(tile.biome)).c_str(), 34, y, 17, Color{202, 211, 218, 255});
            y += 22;
            DrawText(("Resource: " + oikumene::ToString(tile.resource)).c_str(), 34, y, 17,
                     Color{202, 211, 218, 255});
            y += 22;
            DrawText(("Elevation " + Fixed(tile.elevation) + "  Temp " + Fixed(tile.temperature)).c_str(), 34, y, 17,
                     Color{202, 211, 218, 255});
            y += 22;
            DrawText(("Rain " + Fixed(tile.rainfall) + "  Fertility " + Fixed(tile.fertility)).c_str(), 34, y, 17,
                     Color{202, 211, 218, 255});
            y += 22;
            DrawText(("Move " + Fixed(tile.movement_cost) + "  Settle " + Fixed(tile.settlement_score)).c_str(), 34,
                     y, 17, Color{202, 211, 218, 255});
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
