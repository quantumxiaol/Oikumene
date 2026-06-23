#pragma once

#include <raylib.h>

#include "oikumene/world/world.hpp"

namespace oikumene {

class CameraController {
public:
    void SetViewport(Rectangle viewport);
    void Update(bool mouse_input_enabled = true, bool keyboard_input_enabled = true);
    void FitToWorld(const World& world, int screen_width, int screen_height);
    void FitWorld(int world_width, int world_height);
    void Pan(float dx_pixels, float dy_pixels);
    void ZoomAt(float factor, Vector2 screen_position);
    void CenterOnTile(int x, int y);

    [[nodiscard]] Vector2 TileToScreen(int x, int y) const;
    [[nodiscard]] Vector2 ScreenToWorld(Vector2 screen) const;
    [[nodiscard]] bool ScreenToTile(Vector2 screen, const World& world, int& x, int& y) const;
    [[nodiscard]] float TileSize() const;
    [[nodiscard]] float Zoom() const;
    [[nodiscard]] Vector2 Offset() const;
    [[nodiscard]] Rectangle Viewport() const;

private:
    Rectangle viewport_{0.0F, 0.0F, 1280.0F, 720.0F};
    Vector2 offset_{52.0F, 92.0F};
    float base_tile_size_ = 10.0F;
    float zoom_ = 1.0F;
};

}  // namespace oikumene
