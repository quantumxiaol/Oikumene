#pragma once

#include <optional>
#include <vector>

#include <raylib.h>

#include "oikumene/render/camera_controller.hpp"
#include "oikumene/render/map_layer.hpp"
#include "oikumene/render/selection.hpp"
#include "oikumene/sim/band.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

class MapRenderer {
  public:
    void Draw(const World& world, const CameraController& camera, MapLayer layer,
              const std::optional<std::pair<int, int>>& hover_tile, const Selection& selection) const;

    void DrawEntities(const std::vector<Band>& bands, const std::vector<Settlement>& settlements,
                      const CameraController& camera, const Selection& selection) const;
};

} // namespace oikumene
