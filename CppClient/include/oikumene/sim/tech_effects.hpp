#pragma once

#include <string>
#include <vector>

#include "oikumene/sim/technology.hpp"

namespace oikumene {

struct TechEffects {
    float farm_output_multiplier = 1.0F;
    float river_farm_output_multiplier = 1.0F;
    float pasture_output_multiplier = 1.0F;
    float horse_value_multiplier = 1.0F;
    float food_storage_multiplier = 1.0F;
    float famine_severity_multiplier = 1.0F;
    float carrying_capacity_multiplier = 1.0F;
    float admin_capacity_multiplier = 1.0F;
    float distance_admin_load_multiplier = 1.0F;
    float overextension_penalty_multiplier = 1.0F;
    float control_path_cost_multiplier = 1.0F;
    float coastal_control_cost_multiplier = 1.0F;
    float contested_stability_loss_multiplier = 1.0F;
    bool mining_enabled = false;
    bool roads_enabled = false;
    bool coastal_trade_enabled = false;
    bool bronze_working_enabled = false;
    bool fortification_enabled = false;
};

[[nodiscard]] TechEffects ComputeTechEffects(const ResearchState& research);
[[nodiscard]] std::vector<std::string> DescribeActiveEffects(const TechEffects& effects);

} // namespace oikumene
