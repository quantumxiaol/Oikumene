#include "oikumene/ui/help_panel.hpp"

#include <raylib.h>

#include "oikumene/ui/panel_layout.hpp"

namespace oikumene {

void DrawHelpPanel() {
    DrawPanelBackground(GetScreenWidth() - 390, 18, 364, 342);
    DrawText("Help", GetScreenWidth() - 370, 36, 22, RAYWHITE);
    DrawText("1-9,0    switch map layers", GetScreenWidth() - 370, 72, 16, Color{202, 211, 218, 255});
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

} // namespace oikumene
