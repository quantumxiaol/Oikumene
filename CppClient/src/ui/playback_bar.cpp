#include "oikumene/ui/playback_bar.hpp"

#include "oikumene/app/input_controller.hpp"
#include "oikumene/render/selection.hpp"
#include "oikumene/ui/panel_layout.hpp"
#include "oikumene/ui/view_helpers.hpp"

namespace oikumene {

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

} // namespace oikumene
