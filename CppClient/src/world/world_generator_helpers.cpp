#include "world_generator_helpers.hpp"

#include <algorithm>
#include <cmath>

#include "oikumene/world/world_generator.hpp"

namespace oikumene::world_generator {

float Clamp01(float value) {
    return std::clamp(value, 0.0F, 1.0F);
}

float Smooth(float value) {
    value = Clamp01(value);
    return value * value * (3.0F - 2.0F * value);
}

float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

std::uint64_t Mix(std::uint64_t value) {
    value ^= value >> 30U;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27U;
    value *= 0x94d049bb133111ebULL;
    value ^= value >> 31U;
    return value;
}

int IndexOf(const World& world, int x, int y) {
    return y * world.Width() + x;
}

std::array<std::pair<int, int>, 8> NeighborOffsets8() {
    return {{{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}}};
}

bool IsLand(const Tile& tile) {
    return !tile.is_ocean && !tile.is_lake;
}

bool IsMineral(ResourceKind resource) {
    return resource == ResourceKind::Copper || resource == ResourceKind::Tin || resource == ResourceKind::ShallowIron ||
           resource == ResourceKind::ShallowCoal || resource == ResourceKind::Gold ||
           resource == ResourceKind::Silver || resource == ResourceKind::MeteoricIron;
}

} // namespace oikumene::world_generator

namespace oikumene {

float WorldGenerator::Noise2D(float x, float y, int salt, float frequency) const {
    const float fx = x * frequency;
    const float fy = y * frequency;
    const int x0 = static_cast<int>(std::floor(fx));
    const int y0 = static_cast<int>(std::floor(fy));
    const int x1 = x0 + 1;
    const int y1 = y0 + 1;
    const float tx = world_generator::Smooth(fx - static_cast<float>(x0));
    const float ty = world_generator::Smooth(fy - static_cast<float>(y0));

    const float a = Random01(x0, y0, salt);
    const float b = Random01(x1, y0, salt);
    const float c = Random01(x0, y1, salt);
    const float d = Random01(x1, y1, salt);
    return world_generator::Lerp(world_generator::Lerp(a, b, tx), world_generator::Lerp(c, d, tx), ty);
}

float WorldGenerator::Random01(int x, int y, int salt) const {
    std::uint64_t value = params_.seed;
    value ^= static_cast<std::uint64_t>(x + 0x9e3779b9) * 0xbf58476d1ce4e5b9ULL;
    value ^= static_cast<std::uint64_t>(y + 0x85ebca6b) * 0x94d049bb133111ebULL;
    value ^= static_cast<std::uint64_t>(salt + 0xc2b2ae35) * 0x27d4eb2f165667c5ULL;
    value = world_generator::Mix(value);
    return static_cast<float>((value >> 11U) & 0x1fffffULL) / static_cast<float>(0x1fffffULL);
}

bool WorldGenerator::IsAdjacentToWater(int x, int y) const {
    for (const auto& [dx, dy] : world_generator::NeighborOffsets8()) {
        const int nx = x + dx;
        const int ny = y + dy;
        if (!world_.InBounds(nx, ny)) {
            continue;
        }
        const auto& neighbor = world_.At(nx, ny);
        if (neighbor.is_ocean || neighbor.is_lake || neighbor.has_river) {
            return true;
        }
    }
    return false;
}

bool WorldGenerator::HasNeighborBiome(int x, int y, Biome biome) const {
    for (const auto& [dx, dy] : world_generator::NeighborOffsets8()) {
        const int nx = x + dx;
        const int ny = y + dy;
        if (world_.InBounds(nx, ny) && world_.At(nx, ny).biome == biome) {
            return true;
        }
    }
    return false;
}

} // namespace oikumene
