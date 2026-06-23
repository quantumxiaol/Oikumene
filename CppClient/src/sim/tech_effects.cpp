#include "oikumene/sim/tech_effects.hpp"

namespace oikumene {

TechEffects ComputeTechEffects(const ResearchState& research) {
    TechEffects effects;
    if (HasTech(research, TechId::Pottery)) {
        effects.food_storage_multiplier *= 1.30F;
        effects.famine_severity_multiplier *= 0.80F;
        effects.carrying_capacity_multiplier *= 1.05F;
    }
    if (HasTech(research, TechId::Irrigation)) {
        effects.farm_output_multiplier *= 1.20F;
        effects.river_farm_output_multiplier *= 1.15F;
        effects.carrying_capacity_multiplier *= 1.06F;
    }
    if (HasTech(research, TechId::AnimalHusbandry)) {
        effects.pasture_output_multiplier *= 1.20F;
        effects.horse_value_multiplier *= 1.50F;
    }
    if (HasTech(research, TechId::Mining)) {
        effects.mining_enabled = true;
    }
    if (HasTech(research, TechId::Roads)) {
        effects.roads_enabled = true;
        effects.control_path_cost_multiplier *= 0.85F;
        effects.distance_admin_load_multiplier *= 0.85F;
    }
    if (HasTech(research, TechId::Administration)) {
        effects.admin_capacity_multiplier *= 1.35F;
        effects.overextension_penalty_multiplier *= 0.80F;
    }
    if (HasTech(research, TechId::BronzeWorking)) {
        effects.bronze_working_enabled = true;
    }
    if (HasTech(research, TechId::Fortification)) {
        effects.fortification_enabled = true;
        effects.contested_stability_loss_multiplier *= 0.90F;
    }
    if (HasTech(research, TechId::Sailing)) {
        effects.coastal_trade_enabled = true;
        effects.coastal_control_cost_multiplier *= 0.90F;
    }
    return effects;
}

std::vector<std::string> DescribeActiveEffects(const TechEffects& effects) {
    std::vector<std::string> descriptions;
    if (effects.farm_output_multiplier > 1.0F) {
        descriptions.push_back("Farm x" + std::to_string(effects.farm_output_multiplier).substr(0, 4));
    }
    if (effects.pasture_output_multiplier > 1.0F) {
        descriptions.push_back("Pasture x" + std::to_string(effects.pasture_output_multiplier).substr(0, 4));
    }
    if (effects.mining_enabled) {
        descriptions.push_back("Mining");
    }
    if (effects.admin_capacity_multiplier > 1.0F) {
        descriptions.push_back("Admin x" + std::to_string(effects.admin_capacity_multiplier).substr(0, 4));
    }
    if (effects.roads_enabled) {
        descriptions.push_back("Roads");
    }
    if (effects.bronze_working_enabled) {
        descriptions.push_back("Bronze");
    }
    if (effects.fortification_enabled) {
        descriptions.push_back("Fortified");
    }
    if (effects.coastal_trade_enabled) {
        descriptions.push_back("Sailing");
    }
    return descriptions;
}

}  // namespace oikumene
