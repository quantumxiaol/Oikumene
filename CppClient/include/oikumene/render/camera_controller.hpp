#pragma once

#include <raylib.h>

#include "oikumene/world/world.hpp"

namespace oikumene {

class CameraController {
public:
    void Update();
    void FitToWorld(const World& world, int screen_width, int screen_height);

    [[nodiscard]] Vector2 TileToScreen(int x, int y) const;
    [[nodiscard]] bool ScreenToTile(Vector2 screen, const World& world, int& x, int& y) const;
    [[nodiscard]] float TileSize() const;
    [[nodiscard]] float Zoom() const;
    [[nodiscard]] Vector2 Offset() const;

private:
    Vector2 offset_{52.0F, 92.0F};
    float base_tile_size_ = 10.0F;
    float zoom_ = 1.0F;
};

}  // namespace oikumene
