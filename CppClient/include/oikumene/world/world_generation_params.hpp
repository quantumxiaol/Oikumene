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
    float river_flow_threshold = 3.6F;
    float lake_basin_threshold = 0.030F;
    int max_lake_tiles = 72;
};

}  // namespace oikumene
