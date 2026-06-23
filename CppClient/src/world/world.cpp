#include "oikumene/world/world.hpp"

#include <stdexcept>

namespace oikumene {

World::World(int width, int height, std::uint64_t seed) : width_(width), height_(height), seed_(seed) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("World dimensions must be positive");
    }

    tiles_.resize(static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_));
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            auto& tile = At(x, y);
            tile.x = x;
            tile.y = y;
        }
    }
}

int World::Width() const {
    return width_;
}

int World::Height() const {
    return height_;
}

std::uint64_t World::Seed() const {
    return seed_;
}

bool World::InBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < width_ && y < height_;
}

Tile& World::At(int x, int y) {
    if (!InBounds(x, y)) {
        throw std::out_of_range("World tile coordinates out of bounds");
    }
    return tiles_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
}

const Tile& World::At(int x, int y) const {
    if (!InBounds(x, y)) {
        throw std::out_of_range("World tile coordinates out of bounds");
    }
    return tiles_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
}

std::vector<Tile>& World::Tiles() {
    return tiles_;
}

const std::vector<Tile>& World::Tiles() const {
    return tiles_;
}

}  // namespace oikumene
