#include <cassert>
#include <iostream>
#include <vector>

#include "oikumene/render/selection.hpp"

namespace {

oikumene::World MakeSelectionWorld() {
    oikumene::World world(5, 5, 7);
    for (auto& tile : world.Tiles()) {
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.biome = oikumene::Biome::Grassland;
    }
    return world;
}

void TestLeftClickSelectsSettlementBeforeTile() {
    using namespace oikumene;

    World world = MakeSelectionWorld();
    world.At(2, 2).improvement = ImprovementKind::Farm;

    Band band;
    band.id = 4;
    band.x = 2;
    band.y = 2;
    band.active = true;

    Settlement settlement;
    settlement.id = 9;
    settlement.x = 2;
    settlement.y = 2;

    const Selection selection = SelectAtTile(world, std::vector<Band>{band}, std::vector<Settlement>{settlement}, 2, 2);
    assert(selection.kind == SelectionKind::Settlement);
    assert(selection.id == 9);
}

void TestSelectionPriorityFallsBackThroughBandImprovementTile() {
    using namespace oikumene;

    World world = MakeSelectionWorld();
    world.At(2, 2).improvement = ImprovementKind::Farm;

    Band band;
    band.id = 4;
    band.x = 2;
    band.y = 2;
    band.active = true;

    Selection selection = SelectAtTile(world, std::vector<Band>{band}, {}, 2, 2);
    assert(selection.kind == SelectionKind::Band);
    assert(selection.id == 4);

    band.active = false;
    selection = SelectAtTile(world, std::vector<Band>{band}, {}, 2, 2);
    assert(selection.kind == SelectionKind::ImprovementTile);

    world.At(2, 2).improvement = ImprovementKind::None;
    selection = SelectAtTile(world, {}, {}, 2, 2);
    assert(selection.kind == SelectionKind::Tile);
}

void TestSelectionClearedAfterWorldRegenerate() {
    const oikumene::Selection selection = oikumene::ClearSelection();
    assert(selection.kind == oikumene::SelectionKind::None);
    assert(selection.id == -1);
}

void TestUiPanelConsumesMouseClick() {
    const std::vector<Rectangle> panels{
        Rectangle{10.0F, 10.0F, 100.0F, 80.0F},
        Rectangle{200.0F, 40.0F, 90.0F, 120.0F},
    };
    assert(oikumene::PointInAnyRectangle(Vector2{24.0F, 24.0F}, panels));
    assert(oikumene::PointInAnyRectangle(Vector2{250.0F, 140.0F}, panels));
    assert(!oikumene::PointInAnyRectangle(Vector2{150.0F, 24.0F}, panels));
}

}  // namespace

int main() {
    TestLeftClickSelectsSettlementBeforeTile();
    TestSelectionPriorityFallsBackThroughBandImprovementTile();
    TestSelectionClearedAfterWorldRegenerate();
    TestUiPanelConsumesMouseClick();

    std::cout << "oikumene_selection_tests passed\n";
    return 0;
}
