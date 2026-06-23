#include "oikumene/render/camera_controller.hpp"

#include <algorithm>
#include <cmath>

namespace oikumene {

void CameraController::SetViewport(Rectangle viewport) {
    const Vector2 old_center{viewport_.x + viewport_.width * 0.50F, viewport_.y + viewport_.height * 0.50F};
    const Vector2 world_center = ScreenToWorld(old_center);
    viewport_ = viewport;
    const Vector2 new_center{viewport_.x + viewport_.width * 0.50F, viewport_.y + viewport_.height * 0.50F};
    offset_.x = new_center.x - world_center.x * TileSize();
    offset_.y = new_center.y - world_center.y * TileSize();
}

void CameraController::Update(bool mouse_input_enabled, bool keyboard_input_enabled) {
    const float dt = GetFrameTime();
    const float speed = 520.0F * dt;

    if (keyboard_input_enabled) {
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            Pan(speed, 0.0F);
        }
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            Pan(-speed, 0.0F);
        }
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
            Pan(0.0F, speed);
        }
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
            Pan(0.0F, -speed);
        }
    }

    if (mouse_input_enabled) {
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            const Vector2 delta = GetMouseDelta();
            Pan(delta.x, delta.y);
        }

        const float wheel = GetMouseWheelMove();
        if (std::abs(wheel) > 0.01F) {
            const float factor = wheel > 0.0F ? 1.10F : 1.0F / 1.10F;
            ZoomAt(factor, GetMousePosition());
        }
    }
}

void CameraController::FitToWorld(const World& world, int screen_width, int screen_height) {
    SetViewport(Rectangle{0.0F, 0.0F, static_cast<float>(screen_width), static_cast<float>(screen_height)});
    FitWorld(world.Width(), world.Height());
}

void CameraController::FitWorld(int world_width, int world_height) {
    constexpr float margin = 42.0F;
    const float available_width = std::max(100.0F, viewport_.width - margin * 2.0F);
    const float available_height = std::max(100.0F, viewport_.height - margin * 2.0F);
    const float zoom_x = available_width / (static_cast<float>(world_width) * base_tile_size_);
    const float zoom_y = available_height / (static_cast<float>(world_height) * base_tile_size_);
    zoom_ = std::clamp(std::min(zoom_x, zoom_y), 0.35F, 4.0F);

    const float map_width = static_cast<float>(world_width) * TileSize();
    const float map_height = static_cast<float>(world_height) * TileSize();
    offset_.x = viewport_.x + (viewport_.width - map_width) * 0.50F;
    offset_.y = viewport_.y + (viewport_.height - map_height) * 0.50F;
}

void CameraController::Pan(float dx_pixels, float dy_pixels) {
    offset_.x += dx_pixels;
    offset_.y += dy_pixels;
}

void CameraController::ZoomAt(float factor, Vector2 screen_position) {
    const Vector2 world = ScreenToWorld(screen_position);
    zoom_ = std::clamp(zoom_ * factor, 0.35F, 4.0F);
    offset_.x = screen_position.x - world.x * TileSize();
    offset_.y = screen_position.y - world.y * TileSize();
}

void CameraController::CenterOnTile(int x, int y) {
    const float tile_size = TileSize();
    offset_.x = viewport_.x + viewport_.width * 0.50F - (static_cast<float>(x) + 0.50F) * tile_size;
    offset_.y = viewport_.y + viewport_.height * 0.50F - (static_cast<float>(y) + 0.50F) * tile_size;
}

Vector2 CameraController::TileToScreen(int x, int y) const {
    return Vector2{offset_.x + static_cast<float>(x) * TileSize(), offset_.y + static_cast<float>(y) * TileSize()};
}

Vector2 CameraController::ScreenToWorld(Vector2 screen) const {
    return Vector2{(screen.x - offset_.x) / TileSize(), (screen.y - offset_.y) / TileSize()};
}

bool CameraController::ScreenToTile(Vector2 screen, const World& world, int& x, int& y) const {
    const Vector2 world_position = ScreenToWorld(screen);
    x = static_cast<int>(std::floor(world_position.x));
    y = static_cast<int>(std::floor(world_position.y));
    return world.InBounds(x, y);
}

float CameraController::TileSize() const {
    return base_tile_size_ * zoom_;
}

float CameraController::Zoom() const {
    return zoom_;
}

Vector2 CameraController::Offset() const {
    return offset_;
}

Rectangle CameraController::Viewport() const {
    return viewport_;
}

}  // namespace oikumene
