#include "oikumene/render/map_renderer.hpp"

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

}  // namespace

void MapRenderer::Draw(const World& world,
                       const CameraController& camera,
                       MapLayer layer,
                       const std::optional<std::pair<int, int>>& hover_tile) const {
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

        if (tile.has_river) {
            const float flow_width = std::clamp(tile.river_flow * 0.035F, 0.12F, 0.34F);
            DrawLineEx(Vector2{position.x + tile_size * 0.15F, position.y + tile_size * 0.50F},
                       Vector2{position.x + tile_size * 0.85F, position.y + tile_size * 0.50F},
                       std::max(1.0F, tile_size * flow_width), Color{44, 135, 225, 255});
        }

        if (layer == MapLayer::Resources) {
            DrawResourceMarker(tile, position, tile_size);
        }
    }

    if (hover_tile.has_value()) {
        const auto [x, y] = *hover_tile;
        const Vector2 position = camera.TileToScreen(x, y);
        DrawRectangleLinesEx(Rectangle{position.x, position.y, tile_size, tile_size}, 2.0F, RAYWHITE);
    }
}

void MapRenderer::DrawEntities(const std::vector<Band>& bands,
                               const std::vector<Settlement>& settlements,
                               const CameraController& camera) const {
    const float tile_size = camera.TileSize();

    for (const auto& settlement : settlements) {
        const Vector2 position = camera.TileToScreen(settlement.x, settlement.y);
        const Vector2 center{position.x + tile_size * 0.50F, position.y + tile_size * 0.50F};
        const float radius = std::max(3.0F, tile_size * (settlement.level == SettlementLevel::Village ? 0.42F : 0.34F));
        const Color fill = settlement.level == SettlementLevel::Village ? Color{246, 235, 185, 255}
                                                                        : Color{238, 218, 144, 255};
        DrawRectangleV(Vector2{center.x - radius, center.y - radius}, Vector2{radius * 2.0F, radius * 2.0F}, fill);
        DrawRectangleLinesEx(Rectangle{center.x - radius, center.y - radius, radius * 2.0F, radius * 2.0F}, 2.0F,
                             Color{65, 42, 25, 255});
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
    }
}

}  // namespace oikumene
