#pragma once

#include <array>
#include <cstdint>
#include <utility>

#include "oikumene/world/resource.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene::world_generator {

[[nodiscard]] float Clamp01(float value);
[[nodiscard]] float Smooth(float value);
[[nodiscard]] float Lerp(float a, float b, float t);
[[nodiscard]] std::uint64_t Mix(std::uint64_t value);
[[nodiscard]] int IndexOf(const World& world, int x, int y);
[[nodiscard]] std::array<std::pair<int, int>, 8> NeighborOffsets8();
[[nodiscard]] bool IsLand(const Tile& tile);
[[nodiscard]] bool IsMineral(ResourceKind resource);

} // namespace oikumene::world_generator
