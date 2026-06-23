#pragma once

#include "oikumene/world/biome.hpp"
#include "oikumene/world/improvement.hpp"
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
    ImprovementKind improvement = ImprovementKind::None;
    int worked_by_settlement_id = -1;
    float soil_quality = 0.0F;
    float forest_cover = 0.0F;
    float improvement_progress = 0.0F;

    bool is_ocean = false;
    bool is_lake = false;
    bool has_river = false;
    bool is_coast = false;
    float river_flow = 0.0F;

    int owner_polity_id = -1;
    int settlement_id = -1;
};

}  // namespace oikumene
