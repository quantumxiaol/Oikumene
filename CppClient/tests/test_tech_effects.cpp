#include <cassert>
#include <iostream>

#include "oikumene/sim/tech_effects.hpp"

namespace {

oikumene::ResearchState ResearchWith(std::initializer_list<oikumene::TechId> techs) {
    oikumene::ResearchState research;
    research.unlocked.assign(techs.begin(), techs.end());
    return research;
}

void TestPotteryEffects() {
    const auto effects = oikumene::ComputeTechEffects(ResearchWith({oikumene::TechId::Pottery}));
    assert(effects.food_storage_multiplier > 1.0F);
    assert(effects.famine_severity_multiplier < 1.0F);
    assert(effects.carrying_capacity_multiplier > 1.0F);
}

void TestIrrigationEffects() {
    const auto effects =
        oikumene::ComputeTechEffects(ResearchWith({oikumene::TechId::Pottery, oikumene::TechId::Irrigation}));
    assert(effects.farm_output_multiplier > 1.0F);
    assert(effects.river_farm_output_multiplier > 1.0F);
}

void TestAnimalHusbandryEffects() {
    const auto effects = oikumene::ComputeTechEffects(ResearchWith({oikumene::TechId::AnimalHusbandry}));
    assert(effects.pasture_output_multiplier > 1.0F);
    assert(effects.horse_value_multiplier > 1.0F);
}

void TestMiningAndRoadsEffects() {
    const auto effects =
        oikumene::ComputeTechEffects(ResearchWith({oikumene::TechId::Mining, oikumene::TechId::Roads}));
    assert(effects.mining_enabled);
    assert(effects.roads_enabled);
    assert(effects.control_path_cost_multiplier < 1.0F);
    assert(effects.distance_admin_load_multiplier < 1.0F);
}

void TestAdministrationAndMilitaryEffects() {
    const auto effects = oikumene::ComputeTechEffects(
        ResearchWith({oikumene::TechId::Roads, oikumene::TechId::Administration, oikumene::TechId::Mining,
                      oikumene::TechId::BronzeWorking, oikumene::TechId::Fortification}));
    assert(effects.admin_capacity_multiplier > 1.0F);
    assert(effects.overextension_penalty_multiplier < 1.0F);
    assert(effects.bronze_working_enabled);
    assert(effects.fortification_enabled);
    assert(effects.contested_stability_loss_multiplier < 1.0F);
}

void TestSailingEffects() {
    const auto effects = oikumene::ComputeTechEffects(ResearchWith({oikumene::TechId::Sailing}));
    assert(effects.coastal_trade_enabled);
    assert(effects.coastal_control_cost_multiplier < 1.0F);
}

} // namespace

int main() {
    TestPotteryEffects();
    TestIrrigationEffects();
    TestAnimalHusbandryEffects();
    TestMiningAndRoadsEffects();
    TestAdministrationAndMilitaryEffects();
    TestSailingEffects();

    std::cout << "oikumene_tech_effects_tests passed\n";
    return 0;
}
