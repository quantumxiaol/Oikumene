#include <chrono>
#include <string>

#include <raylib.h>

#include "oikumene/ai/remote_decision_provider.hpp"
#include "oikumene/core/simulation.hpp"

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

}  // namespace

int main() {
    constexpr int screen_width = 1040;
    constexpr int screen_height = 640;

    InitWindow(screen_width, screen_height, "Oikumene - The Habitable World");
    SetTargetFPS(60);

    oikumene::Simulation simulation;
    oikumene::RemoteDecisionProvider remote_provider("127.0.0.1", 8000, std::chrono::milliseconds(900));
    auto health = remote_provider.CheckHealth();

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            health = remote_provider.CheckHealth();
        }
        if (IsKeyPressed(KEY_SPACE)) {
            simulation.AdvanceOneTurn();
        }

        BeginDrawing();
        ClearBackground(Color{18, 22, 26, 255});

        DrawText("Oikumene", 40, 38, 42, RAYWHITE);
        DrawText("The Habitable World / 人居界", 42, 86, 20, Color{180, 190, 198, 255});

        DrawRectangle(40, 136, 960, 1, Color{68, 76, 84, 255});

        DrawText("Phase 0 scaffold", 42, 170, 22, Color{220, 225, 230, 255});
        DrawText(simulation.StatusSummary().c_str(), 42, 204, 20, Color{180, 190, 198, 255});

        DrawText("Python Agent Server:", 42, 254, 22, Color{220, 225, 230, 255});
        DrawText(StatusText(health).c_str(), 292, 254, 22, StatusColor(health.online));

        const std::string service = health.service.empty() ? "service: unknown" : "service: " + health.service;
        DrawText(service.c_str(), 42, 290, 18, Color{160, 170, 180, 255});
        DrawText("Press R to retry health check", 42, 340, 18, Color{160, 170, 180, 255});
        DrawText("Press Space to advance the placeholder simulation turn", 42, 368, 18, Color{160, 170, 180, 255});

        DrawRectangleLines(40, 430, 960, 120, Color{68, 76, 84, 255});
        DrawText("Next phases: world generation, settlement simulation, control fields, trade/war ROI.", 64, 468, 19,
                 Color{210, 216, 222, 255});
        DrawText("LLM/Python will remain advisory; C++ will own and validate world state.", 64, 500, 19,
                 Color{210, 216, 222, 255});

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
