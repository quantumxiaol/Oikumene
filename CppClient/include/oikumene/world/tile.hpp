#pragma once

#include "oikumene/world/biome.hpp"
#include "oikumene/world/resource.hpp"

namespace oikumene {

struct Tile {
    int x = 0;
    int y = 0;

    float elevation = 0.0F;
    float temperature = 0.0F;
    float rainfall = 0.0F;
    float fertility = 0.0F;
    float movement_cost = 1.0F;
    float defense_bonus = 0.0F;
    float settlement_score = 0.0F;

    Biome biome = Biome::Ocean;
    ResourceKind resource = ResourceKind::None;
    float resource_amount = 0.0F;

    bool is_ocean = false;
    bool is_lake = false;
    bool has_river = false;
    bool is_coast = false;

    int owner_polity_id = -1;
    int settlement_id = -1;
};

}  // namespace oikumene
