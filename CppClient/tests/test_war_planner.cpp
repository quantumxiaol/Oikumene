#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "oikumene/sim/war_planner.hpp"

namespace {

oikumene::Polity MakeWarPolity(oikumene::PolityId id, float military) {
    oikumene::Polity polity;
    polity.id = id;
    polity.population = 360;
    polity.stability = 0.82F;
    polity.military_potential = military;
    return polity;
}

oikumene::DiplomacyRelation MakeRelation() {
    oikumene::DiplomacyRelation relation;
    relation.id = 7;
    relation.polity_a_id = 0;
    relation.polity_b_id = 1;
    relation.has_active_trade = true;
    relation.trade_id = 3;
    relation.trade_profit = 1.0F;
    relation.trade_complementarity = 2.0F;
    relation.trade_route_efficiency = 0.8F;
    relation.border_tension = 0.2F;
    relation.economic_overlap = 0.1F;
    relation.friendship = 0.5F;
    relation.competition = 0.2F;
    relation.blockade_tendency = 0.2F;
    return relation;
}

const oikumene::WarPressure* PressureFor(const std::vector<oikumene::WarPressure>& pressures, oikumene::PolityId actor,
                                         oikumene::PolityId target) {
    const auto it = std::find_if(pressures.begin(), pressures.end(), [&](const oikumene::WarPressure& pressure) {
        return pressure.actor_polity_id == actor && pressure.target_polity_id == target;
    });
    return it == pressures.end() ? nullptr : &(*it);
}

float TopDeclarationPressure(const std::vector<oikumene::WarPressure>& pressures) {
    float top = 0.0F;
    for (const auto& pressure : pressures) {
        top = std::max(top, pressure.declaration_pressure);
    }
    return top;
}

void TestFriendlyRelationSuppressesWarPressure() {
    std::vector<oikumene::Polity> polities{MakeWarPolity(0, 22.0F), MakeWarPolity(1, 20.0F)};

    auto friendly = MakeRelation();
    friendly.friendship = 0.92F;
    friendly.competition = 0.05F;
    friendly.border_tension = 0.05F;
    friendly.blockade_tendency = 0.02F;

    auto competitive = MakeRelation();
    competitive.friendship = 0.05F;
    competitive.competition = 0.72F;
    competitive.border_tension = 0.64F;
    competitive.blockade_tendency = 0.28F;

    const auto friendly_pressures = oikumene::BuildWarPressures(polities, {friendly});
    const auto competitive_pressures = oikumene::BuildWarPressures(polities, {competitive});

    assert(!friendly_pressures.empty());
    assert(!competitive_pressures.empty());
    assert(friendly_pressures.front().friendly_penalty > 0.7F);
    assert(TopDeclarationPressure(friendly_pressures) < TopDeclarationPressure(competitive_pressures));
}

void TestLeveragePolityGetsBlockadePressure() {
    std::vector<oikumene::Polity> polities{MakeWarPolity(0, 28.0F), MakeWarPolity(1, 16.0F)};
    auto relation = MakeRelation();
    relation.friendship = 0.08F;
    relation.competition = 0.52F;
    relation.border_tension = 0.48F;
    relation.trade_route_efficiency = 0.2F;
    relation.dependence_a_on_b = 0.04F;
    relation.dependence_b_on_a = 0.82F;
    relation.dependent_polity_id = 1;
    relation.leverage_polity_id = 0;
    relation.blockade_tendency = 0.84F;

    const auto pressures = oikumene::BuildWarPressures(polities, {relation});
    const auto* pressure = PressureFor(pressures, 0, 1);

    assert(pressure != nullptr);
    assert(pressure->objective == oikumene::WarObjective::Blockade);
    assert(pressure->trade_conflict_weight > 0.58F);
    assert(pressure->blockade_pressure > 0.80F);
    assert(pressure->declaration_pressure > 0.45F);
}

void TestDependentPolityGetsBreakoutPressure() {
    std::vector<oikumene::Polity> polities{MakeWarPolity(0, 20.0F), MakeWarPolity(1, 22.0F)};
    auto relation = MakeRelation();
    relation.friendship = 0.08F;
    relation.competition = 0.50F;
    relation.border_tension = 0.42F;
    relation.trade_route_efficiency = 0.2F;
    relation.dependence_a_on_b = 0.04F;
    relation.dependence_b_on_a = 0.82F;
    relation.dependent_polity_id = 1;
    relation.leverage_polity_id = 0;
    relation.blockade_tendency = 0.84F;

    const auto pressures = oikumene::BuildWarPressures(polities, {relation});
    const auto* pressure = PressureFor(pressures, 1, 0);

    assert(pressure != nullptr);
    assert(pressure->objective == oikumene::WarObjective::DependencyBreakout);
    assert(pressure->dependency_pressure > 0.55F);
    assert(pressure->trade_conflict_weight > 0.55F);
}

void TestGrievanceRaisesAndRestraintSuppressesWarPressure() {
    std::vector<oikumene::Polity> polities{MakeWarPolity(0, 24.0F), MakeWarPolity(1, 22.0F)};

    auto grievance = MakeRelation();
    grievance.friendship = 0.08F;
    grievance.competition = 0.35F;
    grievance.border_tension = 0.28F;
    grievance.grievance_a_to_b = 0.88F;

    auto restraint = grievance;
    restraint.grievance_a_to_b = 0.0F;
    restraint.restraint_a_to_b = 0.88F;

    const auto grievance_pressures = oikumene::BuildWarPressures(polities, {grievance});
    const auto restraint_pressures = oikumene::BuildWarPressures(polities, {restraint});
    const auto* grievance_pressure = PressureFor(grievance_pressures, 0, 1);
    const auto* restraint_pressure = PressureFor(restraint_pressures, 0, 1);

    assert(grievance_pressure != nullptr);
    assert(restraint_pressure != nullptr);
    assert(grievance_pressure->grievance_pressure > 0.65F);
    assert(restraint_pressure->restraint_pressure > 0.60F);
    assert(grievance_pressure->declaration_pressure > restraint_pressure->declaration_pressure);
}

void TestVassalageCreatesDependencyBreakoutPressure() {
    std::vector<oikumene::Polity> polities{MakeWarPolity(0, 24.0F), MakeWarPolity(1, 25.0F)};
    auto relation = MakeRelation();
    relation.friendship = 0.10F;
    relation.competition = 0.24F;
    relation.border_tension = 0.20F;
    relation.vassalage_a_to_b = 0.82F;
    relation.dependence_a_on_b = 0.82F;
    relation.dependent_polity_id = 0;
    relation.leverage_polity_id = 1;
    relation.blockade_tendency = 0.40F;

    const auto pressures = oikumene::BuildWarPressures(polities, {relation});
    const auto* pressure = PressureFor(pressures, 0, 1);

    assert(pressure != nullptr);
    assert(pressure->objective == oikumene::WarObjective::DependencyBreakout);
    assert(pressure->vassalage_pressure > 0.75F);
    assert(pressure->dependency_pressure > 0.80F);
}

void TestExplicitTreatySubjectGetsBreakoutPressure() {
    std::vector<oikumene::Polity> polities{MakeWarPolity(0, 30.0F), MakeWarPolity(1, 26.0F)};
    auto relation = MakeRelation();
    relation.friendship = 0.22F;
    relation.competition = 0.18F;
    relation.border_tension = 0.12F;
    relation.active_vassal_treaty_id = 5;
    relation.treaty_overlord_polity_id = 0;
    relation.treaty_subject_polity_id = 1;
    relation.treaty_strength = 0.70F;
    relation.treaty_loyalty = 0.36F;
    relation.treaty_liberty_desire = 0.76F;
    relation.dependence_b_on_a = 0.82F;
    relation.dependent_polity_id = 1;
    relation.leverage_polity_id = 0;

    const auto pressures = oikumene::BuildWarPressures(polities, {relation});
    const auto* subject_pressure = PressureFor(pressures, 1, 0);
    const auto* overlord_pressure = PressureFor(pressures, 0, 1);

    assert(subject_pressure != nullptr);
    assert(overlord_pressure != nullptr);
    assert(subject_pressure->objective == oikumene::WarObjective::DependencyBreakout);
    assert(subject_pressure->vassalage_pressure > 0.70F);
    assert(subject_pressure->declaration_pressure > overlord_pressure->declaration_pressure);
    assert(overlord_pressure->restraint_pressure > 0.55F);
}

} // namespace

int main() {
    TestFriendlyRelationSuppressesWarPressure();
    TestLeveragePolityGetsBlockadePressure();
    TestDependentPolityGetsBreakoutPressure();
    TestGrievanceRaisesAndRestraintSuppressesWarPressure();
    TestVassalageCreatesDependencyBreakoutPressure();
    TestExplicitTreatySubjectGetsBreakoutPressure();
    std::cout << "war planner tests passed\n";
    return 0;
}
