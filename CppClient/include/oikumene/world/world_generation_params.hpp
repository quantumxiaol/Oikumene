#pragma once

#include <cstdint>

namespace oikumene {

struct WorldGenerationParams {
    int width = 80;
    int height = 56;
    std::uint64_t seed = 42;

    float sea_level = 0.46F;
    float mountain_level = 0.78F;
    float hill_level = 0.63F;
    int river_sources = 44;
    int river_min_length = 5;
    int river_max_steps = 180;
};

}  // namespace oikumene
