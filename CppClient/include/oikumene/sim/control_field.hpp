#pragma once

#include <vector>

#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

struct ControlFieldParams {
    float min_control_strength = 24.0F;
    float contested_margin = 4.0F;
    float max_path_cost = 52.0F;
    float path_cost_multiplier = 1.0F;
    float coastal_path_cost_multiplier = 1.0F;
};

struct ControlFieldStats {
    int controlled_land_tiles = 0;
    int contested_tiles = 0;
    int land_tiles = 0;

    [[nodiscard]] float ControlledLandRatio() const;
};

[[nodiscard]] float TerrainControlCost(const Tile& tile);
[[nodiscard]] float TerrainPathCost(const World& world,
                                    int start_x,
                                    int start_y,
                                    int end_x,
                                    int end_y,
                                    float max_cost,
                                    float path_cost_multiplier = 1.0F,
                                    float coastal_path_cost_multiplier = 1.0F,
                                    PolityId route_polity_id = kInvalidPolityId);

[[nodiscard]] ControlFieldStats RecomputeControlField(World& world,
                                                      const std::vector<Settlement>& settlements,
                                                      std::vector<Polity>& polities,
                                                      const ControlFieldParams& params = {});

}  // namespace oikumene
