#pragma once

#include <optional>

#include <raylib.h>

#include "oikumene/render/camera_controller.hpp"
#include "oikumene/render/map_layer.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

class MapRenderer {
public:
    void Draw(const World& world,
              const CameraController& camera,
              MapLayer layer,
              const std::optional<std::pair<int, int>>& hover_tile) const;
};

}  // namespace oikumene
