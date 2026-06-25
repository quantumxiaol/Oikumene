#include "oikumene/ui/panel_layout.hpp"

#include <algorithm>

#include "oikumene/render/selection.hpp"

namespace oikumene {

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

bool DrawButton(Rectangle rect, const std::string& label, bool active) {
    const Vector2 mouse = GetMousePosition();
    const bool hovered = CheckCollisionPointRec(mouse, rect);
    const Color fill = active ? Color{52, 112, 86, 230} : hovered ? Color{54, 62, 70, 230} : Color{32, 38, 44, 220};
    DrawRectangleRec(rect, fill);
    DrawRectangleLinesEx(rect, 1.0F, Color{96, 106, 116, 230});
    DrawText(label.c_str(), static_cast<int>(rect.x + 10.0F), static_cast<int>(rect.y + 6.0F), 14, RAYWHITE);
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
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

} // namespace oikumene
