#include "oikumene/ui/details_panel.hpp"

#include <algorithm>
#include <optional>

#include <raylib.h>

#include "oikumene/render/map_layer.hpp"
#include "oikumene/sim/tech_effects.hpp"
#include "oikumene/sim/technology.hpp"
#include "oikumene/ui/panel_layout.hpp"
#include "oikumene/ui/view_helpers.hpp"
#include "oikumene/world/biome.hpp"
#include "oikumene/world/improvement.hpp"
#include "oikumene/world/resource.hpp"
#include "oikumene/world/route_kind.hpp"

namespace oikumene {
namespace {

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
        const auto tile_trades = ActiveTradesAtTile(state.simulation.Trades(), tile.x, tile.y);
        if (!tile_trades.empty()) {
            DrawText(("Trade paths " + std::to_string(tile_trades.size())).c_str(), 34, y, 17,
                     Color{160, 218, 188, 255});
            y += 22;
            const std::size_t count = std::min<std::size_t>(tile_trades.size(), 3);
            for (std::size_t i = 0; i < count; ++i) {
                DrawText(Truncate(TradeAgreementLine(*tile_trades[i]), 58).c_str(), 34, y, 16,
                         Color{160, 218, 188, 255});
                y += 22;
                if (const auto* relation =
                        DiplomacyRelationForPair(state.simulation.DiplomacyRelations(), tile_trades[i]->polity_a_id,
                                                 tile_trades[i]->polity_b_id)) {
                    DrawText(Truncate("Diplomacy: " + DiplomacyRelationLine(*relation), 58).c_str(), 34, y, 16,
                             Color{178, 204, 232, 255});
                    y += 22;
                }
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
            y += 22;
            DrawText(("Trade " + std::to_string(polity->active_trade_count) + "  Profit " +
                      Fixed(polity->trade_profit, 2) + "  Avg route " + Fixed(polity->trade_route_cost, 1))
                         .c_str(),
                     34, y, 17, Color{160, 218, 188, 255});
            y += 22;
            DrawText(("Occupation load " + Fixed(polity->occupation_load, 2) + "  Unrest " +
                      Fixed(polity->occupation_unrest, 2) + "  Vassals " + std::to_string(polity->vassal_count))
                         .c_str(),
                     34, y, 17, Color{222, 150, 135, 255});
            y += 22;
            DrawText(("Vassal tribute +" + Fixed(polity->vassal_tribute_income, 2) + " / -" +
                      Fixed(polity->vassal_tribute_paid, 2) + "  Liberty " + Fixed(polity->vassal_liberty_desire, 2))
                         .c_str(),
                     34, y, 17, Color{214, 170, 156, 255});
            y += 22;
            DrawText(Truncate("Agreements: " + TradeIdsForPolity(state.simulation.Trades(), polity->id), 58).c_str(),
                     34, y, 16, Color{160, 218, 188, 255});
            y += 22;
            const auto treaties = VassalTreatiesForPolity(state.simulation.VassalTreaties(), polity->id);
            DrawText(("Vassal treaties " + std::to_string(treaties.size())).c_str(), 34, y, 16,
                     Color{214, 170, 156, 255});
            y += 22;
            const std::size_t treaty_count = std::min<std::size_t>(treaties.size(), 2);
            for (std::size_t i = 0; i < treaty_count; ++i) {
                DrawText(Truncate(VassalTreatyLine(*treaties[i]), 58).c_str(), 34, y, 16, Color{214, 170, 156, 255});
                y += 22;
            }
            const auto diplomacy = DiplomacyRelationsForPolity(state.simulation.DiplomacyRelations(), polity->id);
            DrawText(("Diplomacy relations " + std::to_string(diplomacy.size())).c_str(), 34, y, 16,
                     Color{178, 204, 232, 255});
            y += 22;
            const std::size_t diplomacy_count = std::min<std::size_t>(diplomacy.size(), 3);
            for (std::size_t i = 0; i < diplomacy_count; ++i) {
                DrawText(Truncate(DiplomacyRelationLine(*diplomacy[i]), 58).c_str(), 34, y, 16,
                         Color{178, 204, 232, 255});
                y += 22;
            }
            const auto war_pressures = WarPressuresForActor(state.simulation.WarPressures(), polity->id);
            DrawText(("War ROI candidates " + std::to_string(war_pressures.size())).c_str(), 34, y, 16,
                     Color{232, 178, 142, 255});
            y += 22;
            const std::size_t war_count = std::min<std::size_t>(war_pressures.size(), 3);
            for (std::size_t i = 0; i < war_count; ++i) {
                DrawText(Truncate(WarPressureLine(*war_pressures[i]), 58).c_str(), 34, y, 16,
                         Color{232, 178, 142, 255});
                y += 22;
            }
            const auto war_targets = WarTargetsForActor(state.simulation.WarTargets(), polity->id);
            DrawText(("War targets " + std::to_string(war_targets.size())).c_str(), 34, y, 16,
                     Color{246, 194, 126, 255});
            y += 22;
            const std::size_t target_count = std::min<std::size_t>(war_targets.size(), 3);
            for (std::size_t i = 0; i < target_count; ++i) {
                DrawText(Truncate(WarTargetLine(*war_targets[i]), 58).c_str(), 34, y, 16, Color{246, 194, 126, 255});
                y += 22;
            }
            const auto wars = WarsForPolity(state.simulation.Wars(), polity->id);
            DrawText(("Wars " + std::to_string(wars.size())).c_str(), 34, y, 16, Color{246, 150, 120, 255});
            y += 22;
            const std::size_t campaign_count = std::min<std::size_t>(wars.size(), 3);
            for (std::size_t i = 0; i < campaign_count; ++i) {
                DrawText(Truncate(WarCampaignLine(*wars[i]), 58).c_str(), 34, y, 16, Color{246, 150, 120, 255});
                y += 22;
            }
            const auto occupations = OccupationsForPolity(state.simulation.Occupations(), polity->id);
            DrawText(("Occupations " + std::to_string(occupations.size())).c_str(), 34, y, 16,
                     Color{222, 150, 135, 255});
            y += 22;
            const std::size_t occupation_count = std::min<std::size_t>(occupations.size(), 3);
            for (std::size_t i = 0; i < occupation_count; ++i) {
                DrawText(Truncate(OccupationLine(*occupations[i]), 58).c_str(), 34, y, 16, Color{222, 150, 135, 255});
                y += 22;
            }
        }
    }
}

} // namespace

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

} // namespace oikumene
