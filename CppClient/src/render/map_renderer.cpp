#include "oikumene/render/map_renderer.hpp"

#include "oikumene/render/color_palette.hpp"

namespace oikumene {

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

        if (tile.has_river && layer != MapLayer::Resources) {
            DrawLineEx(Vector2{position.x + tile_size * 0.15F, position.y + tile_size * 0.50F},
                       Vector2{position.x + tile_size * 0.85F, position.y + tile_size * 0.50F},
                       std::max(1.0F, tile_size * 0.16F), Color{44, 135, 225, 255});
        }

        if (layer == MapLayer::Resources && tile.resource != ResourceKind::None) {
            DrawCircleV(Vector2{position.x + tile_size * 0.50F, position.y + tile_size * 0.50F},
                        std::max(2.0F, tile_size * 0.28F), ColorForResource(tile.resource));
        }
    }

    if (hover_tile.has_value()) {
        const auto [x, y] = *hover_tile;
        const Vector2 position = camera.TileToScreen(x, y);
        DrawRectangleLinesEx(Rectangle{position.x, position.y, tile_size, tile_size}, 2.0F, RAYWHITE);
    }
}

}  // namespace oikumene
