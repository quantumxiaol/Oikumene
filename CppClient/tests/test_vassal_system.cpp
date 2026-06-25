#include <cassert>
#include <iostream>
#include <vector>

#include "oikumene/sim/vassal_system.hpp"

namespace {

oikumene::Polity MakePolity(oikumene::PolityId id, float stability, float military, float wealth_surplus) {
    oikumene::Polity polity;
    polity.id = id;
    polity.stability = stability;
    polity.military_potential = military;
    polity.budget.wealth_surplus = wealth_surplus;
    polity.budget.food_surplus = 6.0F;
    return polity;
}

void TestCreateAndApplyTreaty() {
    std::vector<oikumene::VassalTreaty> treaties;
    std::vector<oikumene::Polity> polities{MakePolity(0, 0.82F, 52.0F, 3.0F), MakePolity(1, 0.64F, 24.0F, 2.0F)};
    std::vector<oikumene::DiplomacyRelation> relations;

    const int treaty_id = oikumene::VassalSystem::CreateOrRefreshTreaty(treaties, 12, 0, 1, 7, 0.65F);
    assert(treaty_id == 0);
    assert(treaties.size() == 1);
    assert(treaties.front().status == oikumene::VassalTreatyStatus::Active);

    oikumene::VassalSystem::UpdateTreaties(20, polities, relations, treaties);

    assert(polities[0].vassal_count == 1);
    assert(polities[0].subject_treaty_ids.size() == 1);
    assert(polities[1].overlord_polity_id == 0);
    assert(polities[1].active_overlord_treaty_id == treaty_id);
    assert(treaties.front().tribute_due > 0.0F);
    assert(treaties.front().loyalty > 0.0F);
    assert(treaties.front().liberty_desire > 0.0F);
}

void TestDiplomaticGrievanceRaisesLibertyDesire() {
    std::vector<oikumene::VassalTreaty> treaties;
    std::vector<oikumene::Polity> polities{MakePolity(0, 0.80F, 45.0F, 3.0F), MakePolity(1, 0.42F, 42.0F, 2.0F)};
    std::vector<oikumene::DiplomacyRelation> relations;
    oikumene::DiplomacyRelation relation;
    relation.polity_a_id = 0;
    relation.polity_b_id = 1;
    relation.grievance_b_to_a = 0.86F;
    relations.push_back(relation);

    oikumene::VassalSystem::CreateOrRefreshTreaty(treaties, 12, 0, 1, 7, 0.60F);
    oikumene::VassalSystem::UpdateTreaties(20, polities, relations, treaties);

    assert(treaties.front().liberty_desire > 0.45F);
    assert(treaties.front().loyalty < 0.75F);
    assert(polities[1].vassal_liberty_desire == treaties.front().liberty_desire);
}

void TestMissingPolityBreaksTreaty() {
    std::vector<oikumene::VassalTreaty> treaties;
    std::vector<oikumene::Polity> polities{MakePolity(0, 0.82F, 52.0F, 3.0F)};
    std::vector<oikumene::DiplomacyRelation> relations;

    oikumene::VassalSystem::CreateOrRefreshTreaty(treaties, 12, 0, 1, 7, 0.65F);
    oikumene::VassalSystem::UpdateTreaties(20, polities, relations, treaties);

    assert(treaties.front().status == oikumene::VassalTreatyStatus::Broken);
    assert(treaties.front().ended_turn == 20);
    assert(polities.front().vassal_count == 0);
}

} // namespace

int main() {
    TestCreateAndApplyTreaty();
    TestDiplomaticGrievanceRaisesLibertyDesire();
    TestMissingPolityBreaksTreaty();
    std::cout << "vassal system tests passed\n";
    return 0;
}
