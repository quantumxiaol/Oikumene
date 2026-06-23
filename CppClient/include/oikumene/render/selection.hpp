#pragma once

#include <string>
#include <vector>

#include <raylib.h>

#include "oikumene/sim/band.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

enum class SelectionKind {
    None,
    Tile,
    ImprovementTile,
    Band,
    Settlement,
};

struct Selection {
    SelectionKind kind = SelectionKind::None;
    int id = -1;
    int x = -1;
    int y = -1;
};

[[nodiscard]] std::string ToString(SelectionKind kind);
[[nodiscard]] Selection ClearSelection();
[[nodiscard]] Selection SelectAtTile(const World& world,
                                     const std::vector<Band>& bands,
                                     const std::vector<Settlement>& settlements,
                                     int x,
                                     int y);
[[nodiscard]] bool IsSelectedTile(const Selection& selection, int x, int y);
[[nodiscard]] bool PointInAnyRectangle(Vector2 point, const std::vector<Rectangle>& rectangles);

}  // namespace oikumene
