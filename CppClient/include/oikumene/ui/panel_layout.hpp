#pragma once

#include <string>
#include <vector>

#include <raylib.h>

#include "oikumene/app/app_state.hpp"

namespace oikumene {

void DrawPanelBackground(int x, int y, int width, int height);
[[nodiscard]] Rectangle HudBounds();
[[nodiscard]] Rectangle DebugPanelBounds();
[[nodiscard]] Rectangle HelpPanelBounds();
[[nodiscard]] Rectangle LegendPanelBounds();
[[nodiscard]] Rectangle EventLogPanelBounds();
[[nodiscard]] Rectangle PlaybackBarBounds();
[[nodiscard]] Rectangle PlaybackButton(int index, float width);
bool DrawButton(Rectangle rect, const std::string& label, bool active = false);
[[nodiscard]] std::vector<Rectangle> UiCaptureRectangles(const AppState& state);
[[nodiscard]] bool UiCapturesMouse(const AppState& state, Vector2 mouse);

} // namespace oikumene
