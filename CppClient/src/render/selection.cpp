#include "oikumene/render/selection.hpp"

namespace oikumene {

std::string ToString(SelectionKind kind) {
    switch (kind) {
        case SelectionKind::None:
            return "None";
        case SelectionKind::Tile:
            return "Tile";
        case SelectionKind::ImprovementTile:
            return "ImprovementTile";
        case SelectionKind::Band:
            return "Band";
        case SelectionKind::Settlement:
            return "Settlement";
    }
    return "Unknown";
}

Selection ClearSelection() {
    return {};
}

Selection SelectAtTile(const World& world,
                       const std::vector<Band>& bands,
                       const std::vector<Settlement>& settlements,
                       int x,
                       int y) {
    if (!world.InBounds(x, y)) {
        return ClearSelection();
    }

    for (const auto& settlement : settlements) {
        if (settlement.x == x && settlement.y == y) {
            return Selection{.kind = SelectionKind::Settlement, .id = settlement.id, .x = x, .y = y};
        }
    }

    for (const auto& band : bands) {
        if (band.active && band.x == x && band.y == y) {
            return Selection{.kind = SelectionKind::Band, .id = band.id, .x = x, .y = y};
        }
    }

    const auto& tile = world.At(x, y);
    if (tile.improvement != ImprovementKind::None) {
        return Selection{.kind = SelectionKind::ImprovementTile, .id = -1, .x = x, .y = y};
    }

    return Selection{.kind = SelectionKind::Tile, .id = -1, .x = x, .y = y};
}

bool IsSelectedTile(const Selection& selection, int x, int y) {
    return selection.kind != SelectionKind::None && selection.x == x && selection.y == y;
}

bool PointInAnyRectangle(Vector2 point, const std::vector<Rectangle>& rectangles) {
    for (const auto& rectangle : rectangles) {
        if (point.x >= rectangle.x && point.y >= rectangle.y && point.x <= rectangle.x + rectangle.width &&
            point.y <= rectangle.y + rectangle.height) {
            return true;
        }
    }
    return false;
}

}  // namespace oikumene
