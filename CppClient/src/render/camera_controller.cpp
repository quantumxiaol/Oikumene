#include "oikumene/render/camera_controller.hpp"

#include <algorithm>
#include <cmath>

namespace oikumene {

void CameraController::Update() {
    const float dt = GetFrameTime();
    const float speed = 520.0F * dt;

    if (IsKeyDown(KEY_LEFT)) {
        offset_.x += speed;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        offset_.x -= speed;
    }
    if (IsKeyDown(KEY_UP)) {
        offset_.y += speed;
    }
    if (IsKeyDown(KEY_DOWN)) {
        offset_.y -= speed;
    }

    const float wheel = GetMouseWheelMove();
    if (std::abs(wheel) > 0.01F) {
        const Vector2 mouse = GetMousePosition();
        const float before_x = (mouse.x - offset_.x) / TileSize();
        const float before_y = (mouse.y - offset_.y) / TileSize();
        zoom_ = std::clamp(zoom_ + wheel * 0.10F, 0.35F, 4.0F);
        offset_.x = mouse.x - before_x * TileSize();
        offset_.y = mouse.y - before_y * TileSize();
    }
}

void CameraController::FitToWorld(const World& world, int screen_width, int screen_height) {
    constexpr float margin = 42.0F;
    const float available_width = std::max(100.0F, static_cast<float>(screen_width) - margin * 2.0F);
    const float available_height = std::max(100.0F, static_cast<float>(screen_height) - margin * 2.0F);
    const float zoom_x = available_width / (static_cast<float>(world.Width()) * base_tile_size_);
    const float zoom_y = available_height / (static_cast<float>(world.Height()) * base_tile_size_);
    zoom_ = std::clamp(std::min(zoom_x, zoom_y), 0.35F, 4.0F);

    const float map_width = static_cast<float>(world.Width()) * TileSize();
    const float map_height = static_cast<float>(world.Height()) * TileSize();
    offset_.x = (static_cast<float>(screen_width) - map_width) * 0.50F;
    offset_.y = (static_cast<float>(screen_height) - map_height) * 0.50F;
}

Vector2 CameraController::TileToScreen(int x, int y) const {
    return Vector2{offset_.x + static_cast<float>(x) * TileSize(), offset_.y + static_cast<float>(y) * TileSize()};
}

bool CameraController::ScreenToTile(Vector2 screen, const World& world, int& x, int& y) const {
    x = static_cast<int>(std::floor((screen.x - offset_.x) / TileSize()));
    y = static_cast<int>(std::floor((screen.y - offset_.y) / TileSize()));
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

}  // namespace oikumene
