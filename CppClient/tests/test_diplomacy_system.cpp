#include <cassert>
#include <iostream>
#include <vector>

#include "oikumene/sim/diplomacy_system.hpp"

namespace {

oikumene::World MakeDiplomacyWorld(int width = 8, int height = 8) {
    using namespace oikumene;
    World world(width, height, 552);
    for (auto& tile : world.Tiles()) {
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.biome = Biome::Grassland;
        tile.movement_cost = 1.0F;
        tile.controller_polity_id = kInvalidPolityId;
        tile.is_contested = false;
    }
    return world;
}

void SetSharedBorder(oikumene::World& world, oikumene::PolityId a, oikumene::PolityId b) {
    const int split = world.Width() / 2;
    for (auto& tile : world.Tiles()) {
        tile.controller_polity_id = tile.x < split ? a : b;
        tile.is_contested = tile.x == split - 1 || tile.x == split;
    }
}

oikumene::Polity MakePolity(oikumene::PolityId id) {
    oikumene::Polity polity;
    polity.id = id;
    polity.population = 320;
    polity.stability = 1.0F;
    polity.military_potential = 18.0F;
    polity.budget.food_income = 8.0F;
    polity.budget.food_surplus = 4.0F;
    polity.budget.wood_income = 3.0F;
    polity.budget.wood_surplus = 1.0F;
    polity.budget.ore_income = 0.1F;
    polity.budget.wealth_surplus = 1.0F;
    return polity;
}

oikumene::TradeAgreement MakeTrade(float a_to_b, float b_to_a, float profit, float route_efficiency) {
    oikumene::TradeAgreement trade;
    trade.id = 4;
    trade.polity_a_id = 0;
    trade.polity_b_id = 1;
    trade.opened_turn = 10;
    trade.last_evaluated_turn = 40;
    trade.active = true;
    trade.value_a_to_b = a_to_b;
    trade.value_b_to_a = b_to_a;
    trade.complementarity = a_to_b + b_to_a;
    trade.route_cost = 40.0F;
    trade.route_efficiency = route_efficiency;
    trade.expected_profit = profit;
    trade.path = {oikumene::TileCoord{1, 1}, oikumene::TileCoord{2, 1}, oikumene::TileCoord{3, 1}};
    return trade;
}

const oikumene::DiplomacyRelation& OnlyRelation(const std::vector<oikumene::DiplomacyRelation>& relations) {
    assert(relations.size() == 1);
    return relations.front();
}

void TestBalancedTradeBecomesFriendly() {
    auto world = MakeDiplomacyWorld();
    std::vector<oikumene::Polity> polities{MakePolity(0), MakePolity(1)};
    std::vector<oikumene::TradeAgreement> trades{MakeTrade(1.2F, 1.2F, 1.1F, 0.9F)};
    std::vector<oikumene::VassalTreaty> treaties;
    std::vector<oikumene::DiplomacyRelation> relations;

    oikumene::DiplomacySystem::UpdateDiplomacy(world, 60, polities, trades, treaties, relations);

    const auto& relation = OnlyRelation(relations);
    assert(relation.has_active_trade);
    assert(relation.posture == oikumene::DiplomaticPosture::Friendly);
    assert(relation.friendship > relation.competition);
    assert(relation.blockade_tendency < 0.45F);
}

void TestOneWayTradeCreatesDependence() {
    auto world = MakeDiplomacyWorld();
    std::vector<oikumene::Polity> polities{MakePolity(0), MakePolity(1)};
    std::vector<oikumene::TradeAgreement> trades{MakeTrade(0.05F, 3.0F, 0.8F, 0.8F)};
    std::vector<oikumene::VassalTreaty> treaties;
    std::vector<oikumene::DiplomacyRelation> relations;

    oikumene::DiplomacySystem::UpdateDiplomacy(world, 60, polities, trades, treaties, relations);

    const auto& relation = OnlyRelation(relations);
    assert(relation.posture == oikumene::DiplomaticPosture::Dependent);
    assert(relation.dependent_polity_id == 0);
    assert(relation.leverage_polity_id == 1);
    assert(relation.dependence_a_on_b > 0.75F);
}

void TestSharedBorderWithoutTradeCreatesCompetition() {
    auto world = MakeDiplomacyWorld();
    SetSharedBorder(world, 0, 1);
    std::vector<oikumene::Polity> polities{MakePolity(0), MakePolity(1)};
    std::vector<oikumene::TradeAgreement> trades;
    std::vector<oikumene::VassalTreaty> treaties;
    std::vector<oikumene::DiplomacyRelation> relations;

    oikumene::DiplomacySystem::UpdateDiplomacy(world, 60, polities, trades, treaties, relations);

    const auto& relation = OnlyRelation(relations);
    assert(!relation.has_active_trade);
    assert(relation.posture == oikumene::DiplomaticPosture::Competitive);
    assert(relation.competition > relation.friendship);
    assert(relation.border_tension > 0.4F);
}

void TestDependenceAndBorderPressureCreatesBlockadeRisk() {
    auto world = MakeDiplomacyWorld();
    SetSharedBorder(world, 0, 1);
    std::vector<oikumene::Polity> polities{MakePolity(0), MakePolity(1)};
    std::vector<oikumene::TradeAgreement> trades{MakeTrade(0.05F, 3.0F, 0.2F, 0.2F)};
    trades.front().weak_refresh_count = 2;
    std::vector<oikumene::VassalTreaty> treaties;
    std::vector<oikumene::DiplomacyRelation> relations;

    oikumene::DiplomacySystem::UpdateDiplomacy(world, 60, polities, trades, treaties, relations);

    const auto& relation = OnlyRelation(relations);
    assert(relation.posture == oikumene::DiplomaticPosture::BlockadeRisk);
    assert(relation.blockade_tendency > 0.58F);
}

void TestOccupationIncidentMemorySurvivesDiplomacyRefresh() {
    auto world = MakeDiplomacyWorld();
    std::vector<oikumene::Polity> polities{MakePolity(0), MakePolity(1)};
    std::vector<oikumene::TradeAgreement> trades;
    std::vector<oikumene::VassalTreaty> treaties;
    std::vector<oikumene::DiplomacyRelation> relations;

    oikumene::DiplomacySystem::UpdateDiplomacy(world, 80, polities, trades, treaties, relations);
    oikumene::DiplomacySystem::RecordIncident(relations, 81, 0, 1, oikumene::DiplomaticIncidentKind::VassalCreated,
                                              1.0F);

    assert(OnlyRelation(relations).last_incident == oikumene::DiplomaticIncidentKind::VassalCreated);
    assert(OnlyRelation(relations).vassalage_b_to_a > 0.70F);

    oikumene::DiplomacySystem::UpdateDiplomacy(world, 95, polities, trades, treaties, relations);

    const auto& relation = OnlyRelation(relations);
    assert(relation.incident_count == 1);
    assert(relation.last_incident == oikumene::DiplomaticIncidentKind::VassalCreated);
    assert(relation.vassalage_b_to_a > 0.65F);
    assert(relation.dependent_polity_id == 1);
    assert(relation.leverage_polity_id == 0);
    assert(relation.posture == oikumene::DiplomaticPosture::Dependent);
}

void TestWithdrawalIncidentCreatesRestraintMemory() {
    auto world = MakeDiplomacyWorld();
    std::vector<oikumene::Polity> polities{MakePolity(0), MakePolity(1)};
    std::vector<oikumene::TradeAgreement> trades;
    std::vector<oikumene::VassalTreaty> treaties;
    std::vector<oikumene::DiplomacyRelation> relations;

    oikumene::DiplomacySystem::UpdateDiplomacy(world, 80, polities, trades, treaties, relations);
    oikumene::DiplomacySystem::RecordIncident(relations, 81, 0, 1,
                                              oikumene::DiplomaticIncidentKind::OccupationWithdrawn, 1.0F);
    oikumene::DiplomacySystem::UpdateDiplomacy(world, 90, polities, trades, treaties, relations);

    const auto& relation = OnlyRelation(relations);
    assert(relation.restraint_a_to_b > 0.50F);
    assert(relation.grievance_b_to_a > 0.20F);
    assert(relation.last_incident == oikumene::DiplomaticIncidentKind::OccupationWithdrawn);
}

void TestActiveTreatyCreatesExplicitDependence() {
    auto world = MakeDiplomacyWorld();
    std::vector<oikumene::Polity> polities{MakePolity(0), MakePolity(1)};
    std::vector<oikumene::TradeAgreement> trades;
    std::vector<oikumene::VassalTreaty> treaties;
    oikumene::VassalTreaty treaty;
    treaty.id = 9;
    treaty.status = oikumene::VassalTreatyStatus::Active;
    treaty.overlord_polity_id = 0;
    treaty.subject_polity_id = 1;
    treaty.strength = 0.72F;
    treaty.loyalty = 0.66F;
    treaty.liberty_desire = 0.34F;
    treaty.tribute_rate = 0.18F;
    treaties.push_back(treaty);
    std::vector<oikumene::DiplomacyRelation> relations;

    oikumene::DiplomacySystem::UpdateDiplomacy(world, 100, polities, trades, treaties, relations);

    const auto& relation = OnlyRelation(relations);
    assert(relation.active_vassal_treaty_id == 9);
    assert(relation.treaty_overlord_polity_id == 0);
    assert(relation.treaty_subject_polity_id == 1);
    assert(relation.dependent_polity_id == 1);
    assert(relation.leverage_polity_id == 0);
    assert(relation.posture == oikumene::DiplomaticPosture::Dependent);
}

} // namespace

int main() {
    TestBalancedTradeBecomesFriendly();
    TestOneWayTradeCreatesDependence();
    TestSharedBorderWithoutTradeCreatesCompetition();
    TestDependenceAndBorderPressureCreatesBlockadeRisk();
    TestOccupationIncidentMemorySurvivesDiplomacyRefresh();
    TestWithdrawalIncidentCreatesRestraintMemory();
    TestActiveTreatyCreatesExplicitDependence();
    std::cout << "diplomacy system tests passed\n";
    return 0;
}
