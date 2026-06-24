#pragma once

#include <cstdint>
#include <string>

namespace oikumene {

enum class Biome : std::uint8_t {
    Ocean,
    Coast,
    Lake,
    River,
    Desert,
    Grassland,
    Forest,
    Rainforest,
    Wetland,
    Tundra,
    Snow,
    Hill,
    Mountain,
};

[[nodiscard]] std::string ToString(Biome biome);

} // namespace oikumene
