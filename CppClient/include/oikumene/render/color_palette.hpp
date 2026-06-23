#pragma once

#include <raylib.h>

#include "oikumene/core/types.hpp"
#include "oikumene/render/map_layer.hpp"
#include "oikumene/world/tile.hpp"

namespace oikumene {

[[nodiscard]] Color ColorForTile(const Tile& tile, MapLayer layer);
[[nodiscard]] Color ColorForBiome(Biome biome);
[[nodiscard]] Color ColorForResource(ResourceKind resource);
[[nodiscard]] Color ColorForPolity(PolityId polity_id);

}  // namespace oikumene
