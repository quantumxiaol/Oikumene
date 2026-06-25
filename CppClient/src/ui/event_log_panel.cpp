#include "oikumene/ui/event_log_panel.hpp"

#include "oikumene/sim/event.hpp"
#include "oikumene/ui/panel_layout.hpp"
#include "oikumene/ui/view_helpers.hpp"

namespace oikumene {

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

} // namespace oikumene
