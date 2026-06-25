#include <cassert>
#include <iostream>

#include "oikumene/ai/strategic_report.hpp"

namespace {

oikumene::World MakeWorld() {
    oikumene::World world(8, 6, 909);
    for (auto& tile : world.Tiles()) {
        tile.biome = oikumene::Biome::Grassland;
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.fertility = 0.62F;
        tile.movement_cost = 1.0F;
        tile.controller_polity_id = tile.x < 4 ? 0 : 1;
        tile.owner_polity_id = tile.controller_polity_id;
        tile.is_contested = tile.x == 3 || tile.x == 4;
    }
    world.At(1, 2).settlement_id = 0;
    world.At(6, 2).settlement_id = 1;
    world.At(2, 2).resource = oikumene::ResourceKind::Copper;
    world.At(2, 2).resource_amount = 0.8F;
    return world;
}

oikumene::Settlement MakeSettlement(int id, int x, int y, oikumene::PolityId polity_id) {
    oikumene::Settlement settlement;
    settlement.id = id;
    settlement.x = x;
    settlement.y = y;
    settlement.polity_id = polity_id;
    settlement.level = oikumene::SettlementLevel::Village;
    settlement.population = 220;
    settlement.stockpile.food = 120.0F;
    settlement.stockpile.wood = 55.0F;
    settlement.stockpile.wealth = 9.0F;
    return settlement;
}

oikumene::Polity MakePolity(oikumene::PolityId id, int capital_id) {
    oikumene::Polity polity;
    polity.id = id;
    polity.name = id == 0 ? "Aster" : "Beryl";
    polity.capital_settlement_id = capital_id;
    polity.member_settlement_ids = {capital_id};
    polity.population = 220;
    polity.food = 120.0F;
    polity.wood = 55.0F;
    polity.wealth = 9.0F;
    polity.stability = 0.72F;
    polity.admin_capacity = 10.0F;
    polity.admin_load = 4.0F;
    polity.budget.food_surplus = 1.4F;
    polity.budget.wood_surplus = 0.8F;
    polity.budget.wealth_surplus = 0.6F;
    polity.knowledge_income = 1.2F;
    polity.military_potential = id == 0 ? 26.0F : 20.0F;
    return polity;
}

void PopulateStrategicState(oikumene::Simulation& sim) {
    sim.Settlements().push_back(MakeSettlement(0, 1, 2, 0));
    sim.Settlements().push_back(MakeSettlement(1, 6, 2, 1));
    sim.Polities().push_back(MakePolity(0, 0));
    sim.Polities().push_back(MakePolity(1, 1));

    oikumene::DiplomacyRelation relation;
    relation.id = 3;
    relation.polity_a_id = 0;
    relation.polity_b_id = 1;
    relation.posture = oikumene::DiplomaticPosture::Competitive;
    relation.border_tension = 0.55F;
    relation.competition = 0.48F;
    relation.friendship = 0.12F;
    relation.blockade_tendency = 0.22F;
    relation.grievance_a_to_b = 0.35F;
    relation.active_vassal_treaty_id = 2;
    relation.treaty_overlord_polity_id = 0;
    relation.treaty_subject_polity_id = 1;
    relation.treaty_liberty_desire = 0.44F;
    sim.DiplomacyRelations().push_back(relation);

    oikumene::WarPressure pressure;
    pressure.id = 4;
    pressure.actor_polity_id = 0;
    pressure.target_polity_id = 1;
    pressure.objective = oikumene::WarObjective::BorderDispute;
    pressure.declaration_pressure = 0.70F;
    pressure.war_roi = 0.62F;
    pressure.grievance_pressure = 0.28F;
    sim.WarPressures().push_back(pressure);

    oikumene::WarTargetCandidate target;
    target.id = 5;
    target.actor_polity_id = 0;
    target.target_polity_id = 1;
    target.kind = oikumene::WarTargetKind::ContestedBorder;
    target.objective = oikumene::WarObjective::BorderDispute;
    target.x = 3;
    target.y = 2;
    target.roi = 0.75F;
    target.action_score = 0.82F;
    target.target_value = 0.9F;
    target.campaign_cost = 1.1F;
    target.occupation_cost = 0.35F;
    target.high_value = true;
    sim.WarTargets().push_back(target);

    oikumene::VassalTreaty treaty;
    treaty.id = 2;
    treaty.overlord_polity_id = 0;
    treaty.subject_polity_id = 1;
    treaty.strength = 0.64F;
    treaty.loyalty = 0.58F;
    treaty.liberty_desire = 0.44F;
    treaty.tribute_due = 0.6F;
    sim.VassalTreaties().push_back(treaty);
}

void TestBuildStrategicReport() {
    oikumene::Simulation sim(MakeWorld());
    PopulateStrategicState(sim);

    oikumene::StrategicReportOptions options;
    options.max_neighbors = 2;
    options.max_war_targets = 2;
    const auto report = oikumene::BuildStrategicReport(sim, 0, options);

    assert(report.polity_id == 0);
    assert(report.report["polity"]["name"] == "Aster");
    assert(report.report["neighbors"].size() == 1);
    assert(report.report["war_targets"].size() == 1);
    assert(report.report["vassal_treaties"].size() == 1);
    assert(report.report["geography"]["controlled_land_tiles"].get<int>() > 0);
    assert(!report.candidate_actions.empty());

    bool saw_research = false;
    bool saw_war = false;
    for (const auto& action : report.candidate_actions) {
        saw_research = saw_research || action.type == oikumene::ActionType::ResearchTech;
        saw_war = saw_war || action.type == oikumene::ActionType::PrepareWar;
    }
    assert(saw_research);
    assert(saw_war);
}

void TestBuildDecisionBatchRequest() {
    oikumene::Simulation sim(MakeWorld());
    PopulateStrategicState(sim);

    const auto batch = oikumene::BuildDecisionBatchRequest(sim, 7);

    assert(batch.protocol_version == oikumene::kProtocolVersion);
    assert(batch.requests.size() == 2);
    assert(batch.requests.front().strategic_turn == 7);
    assert(batch.requests.front().report.is_object());
    assert(!batch.requests.front().candidate_actions.empty());

    nlohmann::json encoded = batch;
    assert(encoded["requests"].is_array());
    assert(encoded["requests"][0]["report"]["polity"].is_object());
}

} // namespace

int main() {
    TestBuildStrategicReport();
    TestBuildDecisionBatchRequest();
    std::cout << "strategic report tests passed\n";
    return 0;
}
