#include "oikumene/render/map_renderer.hpp"

#include "oikumene/core/types.hpp"
#include "oikumene/render/color_palette.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace oikumene {
namespace {

void DrawResourceMarker(const Tile& tile, Vector2 position, float tile_size) {
    if (tile.resource == ResourceKind::None) {
        return;
    }

    const Vector2 center{position.x + tile_size * 0.50F, position.y + tile_size * 0.50F};
    const float radius = std::max(2.0F, tile_size * 0.24F);
    const Color color = ColorForResource(tile.resource);

    switch (tile.resource) {
    case ResourceKind::Horse:
        DrawTriangle(Vector2{center.x, center.y - radius}, Vector2{center.x - radius, center.y + radius},
                     Vector2{center.x + radius, center.y + radius}, color);
        break;
    case ResourceKind::Stone:
        DrawRectangleV(Vector2{center.x - radius, center.y - radius}, Vector2{radius * 2.0F, radius * 2.0F}, color);
        break;
    case ResourceKind::MeteoricIron: {
        std::array<Vector2, 5> points{};
        for (int i = 0; i < 5; ++i) {
            const float angle = -1.5708F + static_cast<float>(i) * 6.28318F / 5.0F;
            points[static_cast<std::size_t>(i)] =
                Vector2{center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius};
        }
        for (int i = 0; i < 5; ++i) {
            DrawLineEx(points[static_cast<std::size_t>(i)], points[static_cast<std::size_t>((i + 2) % 5)],
                       std::max(1.0F, tile_size * 0.10F), color);
        }
        break;
    }
    case ResourceKind::None:
        break;
    default:
        DrawCircleV(center, radius, color);
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius, Color{18, 22, 26, 220});
        break;
    }
}

void DrawImprovementOverlay(const Tile& tile, Vector2 position, float tile_size) {
    const Vector2 center{position.x + tile_size * 0.50F, position.y + tile_size * 0.50F};
    switch (tile.improvement) {
    case ImprovementKind::Farm: {
        const Color color{226, 202, 104, 210};
        DrawRectangleV(Vector2{position.x + tile_size * 0.22F, position.y + tile_size * 0.22F},
                       Vector2{tile_size * 0.56F, tile_size * 0.56F}, color);
        DrawLineEx(Vector2{position.x + tile_size * 0.28F, center.y}, Vector2{position.x + tile_size * 0.72F, center.y},
                   1.0F, Color{112, 93, 44, 210});
        DrawLineEx(Vector2{center.x, position.y + tile_size * 0.28F}, Vector2{center.x, position.y + tile_size * 0.72F},
                   1.0F, Color{112, 93, 44, 210});
        break;
    }
    case ImprovementKind::LumberCamp:
        DrawRectangleV(Vector2{center.x - tile_size * 0.16F, center.y - tile_size * 0.16F},
                       Vector2{tile_size * 0.32F, tile_size * 0.32F}, Color{128, 78, 42, 230});
        break;
    case ImprovementKind::Pasture:
        DrawCircleV(center, std::max(2.0F, tile_size * 0.20F), Color{166, 218, 124, 220});
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), tile_size * 0.24F,
                        Color{76, 108, 54, 220});
        break;
    case ImprovementKind::ShallowMine:
        DrawCircleV(center, std::max(2.0F, tile_size * 0.18F), Color{54, 56, 58, 235});
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), tile_size * 0.22F,
                        Color{206, 210, 214, 220});
        break;
    case ImprovementKind::ForagingGround:
        DrawCircleV(center, std::max(1.5F, tile_size * 0.12F), Color{80, 190, 108, 210});
        break;
    case ImprovementKind::Road:
    case ImprovementKind::None:
        break;
    }
}

void DrawControlOverlay(const Tile& tile, Vector2 position, float tile_size) {
    if (tile.controller_polity_id == kInvalidPolityId) {
        return;
    }

    Color control = ColorForPolity(tile.controller_polity_id);
    control.a = tile.is_contested ? 118 : 92;
    DrawRectangleV(position, Vector2{tile_size + 0.75F, tile_size + 0.75F}, control);

    if (tile.is_contested) {
        DrawLineEx(position, Vector2{position.x + tile_size, position.y + tile_size}, std::max(1.0F, tile_size * 0.10F),
                   Color{246, 226, 108, 210});
        DrawLineEx(Vector2{position.x + tile_size, position.y}, Vector2{position.x, position.y + tile_size},
                   std::max(1.0F, tile_size * 0.10F), Color{246, 226, 108, 210});
    }
}

Color RouteColorFor(const Tile& tile) {
    Color color =
        tile.route_polity_id == kInvalidPolityId ? Color{224, 202, 136, 245} : ColorForPolity(tile.route_polity_id);
    color.a = 245;
    if (tile.route_kind == RouteKind::Trail) {
        color = Color{196, 172, 122, 235};
    } else if (tile.route_kind == RouteKind::RiverRoute) {
        color = Color{84, 170, 232, 245};
    } else if (tile.route_kind == RouteKind::CoastalRoute) {
        color = Color{112, 206, 216, 245};
    }
    return color;
}

Color TradeColorFor(const TradeAgreement& trade) {
    const float mix = std::clamp(trade.route_efficiency, 0.0F, 1.0F);
    const auto green = static_cast<unsigned char>(180 + static_cast<int>(mix * 58.0F));
    const auto alpha = static_cast<unsigned char>(trade.weak_refresh_count > 0 ? 165 : 230);
    return Color{86, green, 154, alpha};
}

void DrawRouteOverlay(const Tile& tile, Vector2 position, float tile_size) {
    if (!tile.has_route) {
        return;
    }
    const float width = std::max(1.0F, tile_size * (tile.route_kind == RouteKind::Road ? 0.18F : 0.12F));
    const Color color = RouteColorFor(tile);
    const Vector2 start{position.x + tile_size * 0.10F, position.y + tile_size * 0.52F};
    const Vector2 end{position.x + tile_size * 0.90F, position.y + tile_size * 0.52F};
    DrawLineEx(start, end, width, color);
    if (tile.route_kind == RouteKind::Road) {
        DrawLineEx(Vector2{position.x + tile_size * 0.24F, position.y + tile_size * 0.38F},
                   Vector2{position.x + tile_size * 0.76F, position.y + tile_size * 0.38F},
                   std::max(1.0F, width * 0.52F), Color{232, 220, 176, 230});
    }
}

void DrawTradePath(const TradeAgreement& trade, const CameraController& camera) {
    if (!trade.active || trade.path.size() < 2) {
        return;
    }

    const float tile_size = camera.TileSize();
    const float width = std::max(1.5F, tile_size * (0.10F + std::clamp(trade.expected_profit, 0.0F, 2.0F) * 0.025F));
    const Color color = TradeColorFor(trade);
    for (std::size_t i = 1; i < trade.path.size(); ++i) {
        const auto& from = trade.path[i - 1];
        const auto& to = trade.path[i];
        const Vector2 start = camera.TileToScreen(from.x, from.y);
        const Vector2 end = camera.TileToScreen(to.x, to.y);
        DrawLineEx(Vector2{start.x + tile_size * 0.50F, start.y + tile_size * 0.50F},
                   Vector2{end.x + tile_size * 0.50F, end.y + tile_size * 0.50F}, width, color);
    }

    const auto& first = trade.path.front();
    const auto& last = trade.path.back();
    const Vector2 start = camera.TileToScreen(first.x, first.y);
    const Vector2 end = camera.TileToScreen(last.x, last.y);
    DrawCircleV(Vector2{start.x + tile_size * 0.50F, start.y + tile_size * 0.50F}, std::max(2.5F, tile_size * 0.22F),
                Color{238, 228, 142, 230});
    DrawCircleV(Vector2{end.x + tile_size * 0.50F, end.y + tile_size * 0.50F}, std::max(2.5F, tile_size * 0.22F),
                Color{238, 228, 142, 230});
}

} // namespace

void MapRenderer::Draw(const World& world, const std::vector<TradeAgreement>& trades, const CameraController& camera,
                       MapLayer layer, const std::optional<std::pair<int, int>>& hover_tile,
                       const Selection& selection) const {
    const float tile_size = camera.TileSize();
    const int screen_width = GetScreenWidth();
    const int screen_height = GetScreenHeight();

    for (const auto& tile : world.Tiles()) {
        const Vector2 position = camera.TileToScreen(tile.x, tile.y);
        if (position.x > static_cast<float>(screen_width) || position.y > static_cast<float>(screen_height) ||
            position.x + tile_size < 0.0F || position.y + tile_size < 0.0F) {
            continue;
        }

        const Color color = ColorForTile(tile, layer);
        DrawRectangleV(position, Vector2{tile_size + 0.75F, tile_size + 0.75F}, color);
        if (layer == MapLayer::PolityControl) {
            DrawControlOverlay(tile, position, tile_size);
        }
        if (layer == MapLayer::RouteNetwork) {
            DrawRouteOverlay(tile, position, tile_size);
        }

        if (tile.has_river) {
            const float flow_width = std::clamp(tile.river_flow * 0.035F, 0.12F, 0.34F);
            DrawLineEx(Vector2{position.x + tile_size * 0.15F, position.y + tile_size * 0.50F},
                       Vector2{position.x + tile_size * 0.85F, position.y + tile_size * 0.50F},
                       std::max(1.0F, tile_size * flow_width), Color{44, 135, 225, 255});
        }

        DrawImprovementOverlay(tile, position, tile_size);

        if (layer == MapLayer::Resources) {
            DrawResourceMarker(tile, position, tile_size);
        }
    }

    if (layer == MapLayer::TradeNetwork) {
        for (const auto& trade : trades) {
            DrawTradePath(trade, camera);
        }
    }

    if (hover_tile.has_value()) {
        const auto [x, y] = *hover_tile;
        const Vector2 position = camera.TileToScreen(x, y);
        DrawRectangleLinesEx(Rectangle{position.x, position.y, tile_size, tile_size}, 2.0F, RAYWHITE);
    }

    if (selection.kind == SelectionKind::Tile || selection.kind == SelectionKind::ImprovementTile ||
        selection.kind == SelectionKind::RouteTile) {
        const Vector2 position = camera.TileToScreen(selection.x, selection.y);
        DrawRectangleLinesEx(Rectangle{position.x - 1.0F, position.y - 1.0F, tile_size + 2.0F, tile_size + 2.0F}, 3.0F,
                             Color{255, 216, 96, 255});
    }
}

void MapRenderer::DrawEntities(const std::vector<Band>& bands, const std::vector<Settlement>& settlements,
                               const CameraController& camera, const Selection& selection) const {
    const float tile_size = camera.TileSize();

    for (const auto& settlement : settlements) {
        const Vector2 position = camera.TileToScreen(settlement.x, settlement.y);
        const Vector2 center{position.x + tile_size * 0.50F, position.y + tile_size * 0.50F};
        const float radius = std::max(3.0F, tile_size * (settlement.level == SettlementLevel::Village ? 0.42F : 0.34F));
        const Color fill =
            settlement.level == SettlementLevel::Village ? Color{246, 235, 185, 255} : Color{238, 218, 144, 255};
        DrawRectangleV(Vector2{center.x - radius, center.y - radius}, Vector2{radius * 2.0F, radius * 2.0F}, fill);
        DrawRectangleLinesEx(Rectangle{center.x - radius, center.y - radius, radius * 2.0F, radius * 2.0F}, 2.0F,
                             Color{65, 42, 25, 255});
        if (settlement.is_capital) {
            DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius * 1.75F,
                            Color{255, 216, 96, 255});
            DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius * 2.05F,
                            Color{65, 42, 25, 255});
        }
        if (selection.kind == SelectionKind::Settlement && selection.id == settlement.id) {
            DrawRectangleLinesEx(Rectangle{center.x - radius - 3.0F, center.y - radius - 3.0F, radius * 2.0F + 6.0F,
                                           radius * 2.0F + 6.0F},
                                 3.0F, Color{255, 216, 96, 255});
        }
        if (settlement.level == SettlementLevel::Village) {
            DrawTriangle(Vector2{center.x, center.y - radius * 1.55F}, Vector2{center.x - radius, center.y - radius},
                         Vector2{center.x + radius, center.y - radius}, Color{170, 74, 48, 255});
        }
    }

    for (const auto& band : bands) {
        if (!band.active) {
            continue;
        }
        const Vector2 position = camera.TileToScreen(band.x, band.y);
        const Vector2 center{position.x + tile_size * 0.50F, position.y + tile_size * 0.50F};
        const float radius = std::max(3.0F, tile_size * 0.32F);
        DrawTriangle(Vector2{center.x, center.y - radius}, Vector2{center.x - radius, center.y + radius},
                     Vector2{center.x + radius, center.y + radius}, Color{245, 245, 236, 255});
        DrawTriangleLines(Vector2{center.x, center.y - radius}, Vector2{center.x - radius, center.y + radius},
                          Vector2{center.x + radius, center.y + radius}, Color{26, 28, 30, 255});
        if (selection.kind == SelectionKind::Band && selection.id == band.id) {
            DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius * 1.65F,
                            Color{255, 216, 96, 255});
        }
    }
}

} // namespace oikumene
