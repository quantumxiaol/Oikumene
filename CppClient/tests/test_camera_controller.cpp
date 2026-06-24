#include <cassert>
#include <cmath>
#include <iostream>

#include "oikumene/render/camera_controller.hpp"
#include "oikumene/world/world.hpp"

namespace {

bool NearlyEqual(float lhs, float rhs, float tolerance = 0.001F) {
    return std::abs(lhs - rhs) <= tolerance;
}

void TestScreenToTileAndBackRoundTrip() {
    oikumene::World world(80, 56, 42);
    oikumene::CameraController camera;
    camera.SetViewport(Rectangle{0.0F, 0.0F, 800.0F, 600.0F});
    camera.FitWorld(world.Width(), world.Height());

    const Vector2 screen = camera.TileToScreen(10, 12);
    int x = -1;
    int y = -1;
    assert(camera.ScreenToTile(Vector2{screen.x + camera.TileSize() * 0.5F, screen.y + camera.TileSize() * 0.5F}, world,
                               x, y));
    assert(x == 10);
    assert(y == 12);
}

void TestPanChangesCameraOffset() {
    oikumene::CameraController camera;
    camera.SetViewport(Rectangle{0.0F, 0.0F, 800.0F, 600.0F});
    const Vector2 before = camera.Offset();
    camera.Pan(25.0F, -13.0F);
    const Vector2 after = camera.Offset();
    assert(NearlyEqual(after.x, before.x + 25.0F));
    assert(NearlyEqual(after.y, before.y - 13.0F));
}

void TestZoomAtMouseKeepsWorldPointStable() {
    oikumene::CameraController camera;
    camera.SetViewport(Rectangle{0.0F, 0.0F, 800.0F, 600.0F});
    camera.FitWorld(80, 56);
    const Vector2 mouse{321.0F, 244.0F};
    const Vector2 before = camera.ScreenToWorld(mouse);
    camera.ZoomAt(1.7F, mouse);
    const Vector2 after = camera.ScreenToWorld(mouse);
    assert(NearlyEqual(before.x, after.x));
    assert(NearlyEqual(before.y, after.y));
}

void TestFitWorldShowsAllTiles() {
    oikumene::CameraController camera;
    camera.SetViewport(Rectangle{0.0F, 0.0F, 800.0F, 600.0F});
    camera.FitWorld(80, 56);

    const Vector2 top_left = camera.TileToScreen(0, 0);
    const Vector2 bottom_right = camera.TileToScreen(80, 56);
    assert(top_left.x >= -0.01F);
    assert(top_left.y >= -0.01F);
    assert(bottom_right.x <= 800.01F);
    assert(bottom_right.y <= 600.01F);
}

void TestResizePreservesCameraCenter() {
    oikumene::CameraController camera;
    camera.SetViewport(Rectangle{0.0F, 0.0F, 800.0F, 600.0F});
    camera.FitWorld(80, 56);
    const Vector2 before = camera.ScreenToWorld(Vector2{400.0F, 300.0F});

    camera.SetViewport(Rectangle{0.0F, 0.0F, 1200.0F, 900.0F});
    const Vector2 after = camera.ScreenToWorld(Vector2{600.0F, 450.0F});

    assert(NearlyEqual(before.x, after.x));
    assert(NearlyEqual(before.y, after.y));
}

} // namespace

int main() {
    TestScreenToTileAndBackRoundTrip();
    TestPanChangesCameraOffset();
    TestZoomAtMouseKeepsWorldPointStable();
    TestFitWorldShowsAllTiles();
    TestResizePreservesCameraCenter();

    std::cout << "oikumene_camera_controller_tests passed\n";
    return 0;
}
