#pragma once

#include <cstdint>
#include <vector>

#include "oikumene/world/tile.hpp"

namespace oikumene {

class World {
public:
    World(int width, int height, std::uint64_t seed);

    [[nodiscard]] int Width() const;
    [[nodiscard]] int Height() const;
    [[nodiscard]] std::uint64_t Seed() const;
    [[nodiscard]] bool InBounds(int x, int y) const;

    [[nodiscard]] Tile& At(int x, int y);
    [[nodiscard]] const Tile& At(int x, int y) const;

    [[nodiscard]] std::vector<Tile>& Tiles();
    [[nodiscard]] const std::vector<Tile>& Tiles() const;

private:
    int width_ = 0;
    int height_ = 0;
    std::uint64_t seed_ = 0;
    std::vector<Tile> tiles_;
};

}  // namespace oikumene
