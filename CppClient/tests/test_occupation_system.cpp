#include <cassert>
#include <iostream>
#include <vector>

#include "oikumene/sim/occupation_system.hpp"

namespace {

oikumene::World MakeWorld() {
    oikumene::World world(10, 6, 77);
    for (auto& tile : world.Tiles()) {
        tile.biome = oikumene::Biome::Grassland;
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.55F;
        tile.defense_bonus = 0.04F;
    }
    world.At(1, 2).settlement_id = 0;
    world.At(7, 2).settlement_id = 1;
    world.At(1, 2).owner_polity_id = 0;
    world.At(1, 2).controller_polity_id = 0;
    world.At(7, 2).owner_polity_id = 0;
    world.At(7, 2).controller_polity_id = 0;
    return world;
}

std::vector<oikumene::Settlement> MakeSettlements(float food = 300.0F, float wood = 120.0F, float wealth = 40.0F) {
    oikumene::Settlement capital;
    capital.id = 0;
    capital.x = 1;
    capital.y = 2;
    capital.polity_id = 0;
    capital.is_capital = true;
    capital.population = 420;
    capital.level = oikumene::SettlementLevel::Village;
    capital.stockpile.food = food;
    capital.stockpile.wood = wood;
    capital.stockpile.wealth = wealth;

    oikumene::Settlement occupied;
    occupied.id = 1;
    occupied.x = 7;
    occupied.y = 2;
    occupied.polity_id = 0;
    occupied.is_capital = false;
    occupied.population = 160;
    occupied.level = oikumene::SettlementLevel::Village;
    occupied.stockpile.food = 80.0F;
    occupied.stockpile.wood = 25.0F;

    return {capital, occupied};
}

std::vector<oikumene::Polity> MakePolities(float occupier_stability = 0.82F) {
    return {
        oikumene::Polity{
            .id = 0,
            .name = "Occupier",
            .capital_settlement_id = 0,
            .member_settlement_ids = {0, 1},
            .population = 580,
            .stability = occupier_stability,
            .military_potential = 55.0F,
        },
        oikumene::Polity{
            .id = 1,
            .name = "Defeated",
            .capital_settlement_id = -1,
            .member_settlement_ids = {},
            .population = 0,
            .stability = 0.55F,
            .military_potential = 10.0F,
        },
    };
}

oikumene::WarCampaign MakeOccupiedCampaign() {
    oikumene::WarCampaign campaign;
    campaign.id = 4;
    campaign.actor_polity_id = 0;
    campaign.target_polity_id = 1;
    campaign.target_kind = oikumene::WarTargetKind::Settlement;
    campaign.status = oikumene::WarCampaignStatus::Occupied;
    campaign.x = 7;
    campaign.y = 2;
    campaign.target_settlement_id = 1;
    campaign.started_turn = 12;
    campaign.ended_turn = 18;
    campaign.target_value = 1.2F;
    campaign.campaign_cost = 1.0F;
    campaign.occupation_cost = 0.30F;
    campaign.occupation_profit = 0.90F;
    return campaign;
}

oikumene::OccupationRecord MakeActiveOccupation(float maintenance = 0.35F, float unrest = 0.20F,
                                                float integration = 0.30F) {
    oikumene::OccupationRecord occupation;
    occupation.id = 0;
    occupation.source_campaign_id = 4;
    occupation.occupier_polity_id = 0;
    occupation.previous_owner_polity_id = 1;
    occupation.target_kind = oikumene::WarTargetKind::Settlement;
    occupation.settlement_id = 1;
    occupation.x = 7;
    occupation.y = 2;
    occupation.status = oikumene::OccupationStatus::Active;
    occupation.started_turn = 18;
    occupation.last_update_turn = 18;
    occupation.turns_held = 3;
    occupation.occupation_value = 1.4F;
    occupation.maintenance_cost = maintenance;
    occupation.unrest = unrest;
    occupation.integration = integration;
    return occupation;
}

int CountEvents(const oikumene::EventLog& log, oikumene::EventType type) {
    int count = 0;
    for (const auto& event : log.Events()) {
        count += event.type == type ? 1 : 0;
    }
    return count;
}

void TestOccupiedCampaignRegistersOccupationPressure() {
    auto world = MakeWorld();
    auto settlements = MakeSettlements();
    auto polities = MakePolities();
    std::vector<oikumene::WarCampaign> campaigns{MakeOccupiedCampaign()};
    std::vector<oikumene::OccupationRecord> occupations;
    std::vector<oikumene::DiplomacyRelation> diplomacy;
    std::vector<oikumene::VassalTreaty> treaties;
    oikumene::EventLog log;

    oikumene::OccupationSystem::UpdateOccupations(world, 19, settlements, polities, campaigns, occupations, diplomacy,
                                                  treaties, log);

    assert(occupations.size() == 1);
    assert(occupations.front().status == oikumene::OccupationStatus::Active);
    assert(polities.front().occupation_load > 0.0F);
    assert(polities.front().occupation_unrest > 0.0F);
    assert(polities.front().occupied_settlements == 1);
}

void TestStableOccupationCedesTerritory() {
    auto world = MakeWorld();
    auto settlements = MakeSettlements();
    auto polities = MakePolities();
    std::vector<oikumene::WarCampaign> campaigns;
    std::vector<oikumene::OccupationRecord> occupations{MakeActiveOccupation(0.18F, 0.05F, 0.84F)};
    std::vector<oikumene::DiplomacyRelation> diplomacy;
    std::vector<oikumene::VassalTreaty> treaties;
    oikumene::EventLog log;

    oikumene::OccupationSystem::UpdateOccupations(world, 24, settlements, polities, campaigns, occupations, diplomacy,
                                                  treaties, log);

    assert(occupations.front().status == oikumene::OccupationStatus::Ceded);
    assert(settlements[1].polity_id == 0);
    assert(diplomacy.size() == 1);
    assert(diplomacy.front().last_incident == oikumene::DiplomaticIncidentKind::TerritoryCeded);
    assert(CountEvents(log, oikumene::EventType::TerritoryCeded) == 1);
}

void TestUnpaidOccupationWithdrawsOrRevolts() {
    auto world = MakeWorld();
    auto settlements = MakeSettlements(0.0F, 0.0F, 0.0F);
    auto polities = MakePolities(0.15F);
    std::vector<oikumene::WarCampaign> campaigns;
    std::vector<oikumene::OccupationRecord> occupations{MakeActiveOccupation(3.0F, 0.72F, 0.08F)};
    std::vector<oikumene::DiplomacyRelation> diplomacy;
    std::vector<oikumene::VassalTreaty> treaties;
    occupations.front().occupation_value = 0.2F;
    occupations.front().cumulative_shortfall = 1.6F;
    oikumene::EventLog log;

    oikumene::OccupationSystem::UpdateOccupations(world, 24, settlements, polities, campaigns, occupations, diplomacy,
                                                  treaties, log);

    assert(occupations.front().status == oikumene::OccupationStatus::Withdrawn ||
           occupations.front().status == oikumene::OccupationStatus::Revolted);
    assert(settlements[1].polity_id == 1);
    assert(polities[1].capital_settlement_id == 1);
    assert(CountEvents(log, oikumene::EventType::OccupationWithdrawn) +
               CountEvents(log, oikumene::EventType::OccupationRevolt) ==
           1);
    assert(!diplomacy.empty());
    assert(diplomacy.front().last_incident == oikumene::DiplomaticIncidentKind::OccupationWithdrawn ||
           diplomacy.front().last_incident == oikumene::DiplomaticIncidentKind::OccupationRevolt);
}

void TestOccupationCanBecomeVassalBuffer() {
    auto world = MakeWorld();
    auto settlements = MakeSettlements();
    auto polities = MakePolities();
    std::vector<oikumene::WarCampaign> campaigns;
    std::vector<oikumene::OccupationRecord> occupations{MakeActiveOccupation(0.22F, 0.56F, 0.32F)};
    std::vector<oikumene::DiplomacyRelation> diplomacy;
    std::vector<oikumene::VassalTreaty> treaties;
    occupations.front().turns_held = 6;
    occupations.front().occupation_value = 2.5F;
    oikumene::EventLog log;

    oikumene::OccupationSystem::UpdateOccupations(world, 28, settlements, polities, campaigns, occupations, diplomacy,
                                                  treaties, log);

    assert(occupations.front().status == oikumene::OccupationStatus::Vassalized);
    assert(occupations.front().subject_polity_id == 1);
    assert(occupations.front().vassal_treaty_id == 0);
    assert(treaties.size() == 1);
    assert(treaties.front().status == oikumene::VassalTreatyStatus::Active);
    assert(treaties.front().overlord_polity_id == 0);
    assert(treaties.front().subject_polity_id == 1);
    assert(settlements[1].polity_id == 1);
    assert(diplomacy.size() == 1);
    assert(diplomacy.front().last_incident == oikumene::DiplomaticIncidentKind::VassalCreated);
    assert(diplomacy.front().vassalage_b_to_a > 0.5F);
    assert(CountEvents(log, oikumene::EventType::VassalCreated) == 1);
}

} // namespace

int main() {
    TestOccupiedCampaignRegistersOccupationPressure();
    TestStableOccupationCedesTerritory();
    TestUnpaidOccupationWithdrawsOrRevolts();
    TestOccupationCanBecomeVassalBuffer();
    std::cout << "occupation system tests passed\n";
    return 0;
}
