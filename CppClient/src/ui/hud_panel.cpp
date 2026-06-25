#include "oikumene/ui/hud_panel.hpp"

#include "oikumene/render/map_layer.hpp"
#include "oikumene/sim/technology.hpp"
#include "oikumene/ui/panel_layout.hpp"
#include "oikumene/ui/view_helpers.hpp"

namespace oikumene {

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
    DrawText(
        (std::string(state.simulation_params.enable_routes ? "Routes " : "Routes off ") +
         std::to_string(state.simulation.Routes().size()) + "/" +
         std::to_string(RouteTileCount(state.simulation.GetWorld())) + "  Tech avg " +
         Fixed(AverageUnlockedTechs(state.simulation.Polities()), 1) + "  Mining " +
         Fixed(TechUnlockRate(state.simulation.Polities(), TechId::Mining) * 100.0F, 0) + "%  Roads " +
         Fixed(TechUnlockRate(state.simulation.Polities(), TechId::Roads) * 100.0F, 0) + "%  Trade " +
         std::to_string(ActiveTradeCount(state.simulation.Trades())) + " +" +
         Fixed(TotalTradeProfit(state.simulation.Trades()), 1) + "  Dip F/C/D/B " +
         std::to_string(DiplomacyPostureCount(state.simulation.DiplomacyRelations(), DiplomaticPosture::Friendly)) +
         "/" +
         std::to_string(DiplomacyPostureCount(state.simulation.DiplomacyRelations(), DiplomaticPosture::Competitive)) +
         "/" +
         std::to_string(DiplomacyPostureCount(state.simulation.DiplomacyRelations(), DiplomaticPosture::Dependent)) +
         "/" +
         std::to_string(DiplomacyPostureCount(state.simulation.DiplomacyRelations(), DiplomaticPosture::BlockadeRisk)))
            .c_str(),
        30, 154, 15, Color{184, 194, 202, 255});
}

} // namespace oikumene
