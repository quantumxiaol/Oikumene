#pragma once

namespace oikumene {

struct SimulationParams {
    int initial_band_count = 8;
    int band_scan_radius = 5;
    int settlement_exclusion_radius = 6;
    float settlement_threshold = 0.62F;
    float band_food_consumption_per_person = 0.05F;
    float settlement_food_consumption_per_person = 0.08F;
    int settlement_work_radius = 3;
    int population_per_work_slot = 45;
    int camp_improvement_build_interval = 6;
    int village_improvement_build_interval = 5;
    float settlement_growth_rate = 0.013F;
};

}  // namespace oikumene
