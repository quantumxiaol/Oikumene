#include "oikumene/sim/occupation_system.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "oikumene/sim/diplomacy_system.hpp"

namespace oikumene {
namespace {

constexpr int kMinimumResolutionTurns = 3;
constexpr int kVassalResolutionTurns = 5;
constexpr float kCessionIntegrationThreshold = 0.72F;
constexpr float kRevoltRiskThreshold = 0.84F;

Polity* PolityById(std::vector<Polity>& polities, PolityId id) {
    for (auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

const Polity* PolityById(const std::vector<Polity>& polities, PolityId id) {
    for (const auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

Settlement* SettlementById(std::vector<Settlement>& settlements, int id) {
    for (auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id) {
    for (const auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

bool ContainsSettlement(const Polity& polity, int settlement_id) {
    return std::find(polity.member_settlement_ids.begin(), polity.member_settlement_ids.end(), settlement_id) !=
           polity.member_settlement_ids.end();
}

void RemoveSettlement(Polity& polity, int settlement_id) {
    polity.member_settlement_ids.erase(
        std::remove(polity.member_settlement_ids.begin(), polity.member_settlement_ids.end(), settlement_id),
        polity.member_settlement_ids.end());
}

void RepairCapital(std::vector<Settlement>& settlements, Polity& polity) {
    if (polity.member_settlement_ids.empty()) {
        polity.capital_settlement_id = -1;
        return;
    }
    if (ContainsSettlement(polity, polity.capital_settlement_id)) {
        return;
    }
    polity.capital_settlement_id = polity.member_settlement_ids.front();
    if (auto* capital = SettlementById(settlements, polity.capital_settlement_id)) {
        capital->is_capital = true;
    }
}

void RecountPolitySnapshot(const std::vector<Settlement>& settlements, Polity& polity) {
    polity.population = 0;
    polity.food = 0.0F;
    polity.wood = 0.0F;
    polity.ore = 0.0F;
    polity.wealth = 0.0F;
    for (const int settlement_id : polity.member_settlement_ids) {
        const auto* settlement = SettlementById(settlements, settlement_id);
        if (settlement == nullptr) {
            continue;
        }
        polity.population += settlement->population;
        polity.food += settlement->stockpile.food;
        polity.wood += settlement->stockpile.wood;
        polity.ore += settlement->stockpile.ore;
        polity.wealth += settlement->stockpile.wealth;
    }
}

int NextOccupationId(const std::vector<OccupationRecord>& occupations) {
    int next = 0;
    for (const auto& occupation : occupations) {
        next = std::max(next, occupation.id + 1);
    }
    return next;
}

bool HasOccupationForCampaign(const std::vector<OccupationRecord>& occupations, int campaign_id) {
    return std::any_of(occupations.begin(), occupations.end(), [&](const OccupationRecord& occupation) {
        return occupation.source_campaign_id == campaign_id;
    });
}

float TakeResource(float& resource, float amount) {
    if (amount <= 0.0F) {
        return 1.0F;
    }
    const float taken = std::min(resource, amount);
    resource -= taken;
    return std::clamp(taken / amount, 0.0F, 1.0F);
}

float ConsumeOccupationMaintenance(std::vector<Settlement>& settlements, const Polity& polity, float maintenance) {
    if (maintenance <= 0.0F) {
        return 1.0F;
    }

    float food_need = maintenance * 6.0F;
    float wood_need = maintenance * 1.2F;
    float wealth_need = maintenance * 0.35F;
    float food_paid = 0.0F;
    float wood_paid = 0.0F;
    float wealth_paid = 0.0F;

    for (const int settlement_id : polity.member_settlement_ids) {
        auto* settlement = SettlementById(settlements, settlement_id);
        if (settlement == nullptr) {
            continue;
        }
        if (food_need > 0.0F) {
            const float before = food_need;
            const float ratio = TakeResource(settlement->stockpile.food, food_need);
            food_paid += before * ratio;
            food_need -= before * ratio;
        }
        if (wood_need > 0.0F) {
            const float before = wood_need;
            const float ratio = TakeResource(settlement->stockpile.wood, wood_need);
            wood_paid += before * ratio;
            wood_need -= before * ratio;
        }
        if (wealth_need > 0.0F) {
            const float before = wealth_need;
            const float ratio = TakeResource(settlement->stockpile.wealth, wealth_need);
            wealth_paid += before * ratio;
            wealth_need -= before * ratio;
        }
        if (food_need <= 0.0F && wood_need <= 0.0F && wealth_need <= 0.0F) {
            break;
        }
    }

    const float food_score = food_paid / std::max(0.001F, maintenance * 6.0F);
    const float wood_score = wood_paid / std::max(0.001F, maintenance * 1.2F);
    const float wealth_score = wealth_paid / std::max(0.001F, maintenance * 0.35F);
    return std::clamp(food_score * 0.55F + wood_score * 0.25F + wealth_score * 0.20F, 0.0F, 1.0F);
}

void AssignSettlement(World& world, std::vector<Settlement>& settlements, std::vector<Polity>& polities,
                      int settlement_id, PolityId new_owner_id) {
    auto* settlement = SettlementById(settlements, settlement_id);
    if (settlement == nullptr) {
        return;
    }

    const PolityId old_owner_id = settlement->polity_id;
    if (old_owner_id == new_owner_id) {
        if (world.InBounds(settlement->x, settlement->y)) {
            auto& tile = world.At(settlement->x, settlement->y);
            tile.owner_polity_id = new_owner_id;
            tile.controller_polity_id = new_owner_id;
            tile.settlement_id = settlement_id;
        }
        return;
    }

    auto* old_owner = PolityById(polities, old_owner_id);
    if (old_owner != nullptr) {
        RemoveSettlement(*old_owner, settlement_id);
        if (settlement->is_capital) {
            settlement->is_capital = false;
        }
        RepairCapital(settlements, *old_owner);
        RecountPolitySnapshot(settlements, *old_owner);
    }

    settlement->polity_id = new_owner_id;
    settlement->is_capital = false;
    auto* new_owner = PolityById(polities, new_owner_id);
    if (new_owner != nullptr) {
        if (!ContainsSettlement(*new_owner, settlement_id)) {
            new_owner->member_settlement_ids.push_back(settlement_id);
        }
        if (new_owner->capital_settlement_id < 0) {
            new_owner->capital_settlement_id = settlement_id;
            settlement->is_capital = true;
        }
        RecountPolitySnapshot(settlements, *new_owner);
    }

    if (world.InBounds(settlement->x, settlement->y)) {
        auto& tile = world.At(settlement->x, settlement->y);
        tile.owner_polity_id = new_owner_id;
        tile.controller_polity_id = new_owner_id;
        tile.settlement_id = settlement_id;
    }
}

void ReturnTarget(World& world, std::vector<Settlement>& settlements, std::vector<Polity>& polities,
                  OccupationRecord& occupation) {
    const PolityId owner = PolityById(polities, occupation.previous_owner_polity_id) == nullptr
                               ? kInvalidPolityId
                               : occupation.previous_owner_polity_id;
    if (occupation.settlement_id >= 0) {
        AssignSettlement(world, settlements, polities, occupation.settlement_id, owner);
        return;
    }
    if (world.InBounds(occupation.x, occupation.y)) {
        auto& tile = world.At(occupation.x, occupation.y);
        tile.owner_polity_id = owner;
        tile.controller_polity_id = owner;
    }
}

void KeepTarget(World& world, std::vector<Settlement>& settlements, std::vector<Polity>& polities,
                const OccupationRecord& occupation) {
    if (occupation.settlement_id >= 0) {
        AssignSettlement(world, settlements, polities, occupation.settlement_id, occupation.occupier_polity_id);
        return;
    }
    if (world.InBounds(occupation.x, occupation.y)) {
        auto& tile = world.At(occupation.x, occupation.y);
        tile.owner_polity_id = occupation.occupier_polity_id;
        tile.controller_polity_id = occupation.occupier_polity_id;
    }
}

void AddOccupationEvent(Turn turn, const OccupationRecord& occupation, EventType type, EventLog& event_log) {
    std::ostringstream summary;
    summary << "Occupation " << occupation.id << " " << ToString(occupation.status) << " P"
            << occupation.occupier_polity_id << " over former P" << occupation.previous_owner_polity_id << " at ("
            << occupation.x << "," << occupation.y << "), unrest " << occupation.unrest << ", integration "
            << occupation.integration << ", reason: " << occupation.outcome_reason;
    event_log.Add(SimEvent{
        .turn = turn,
        .type = type,
        .actor_id = occupation.occupier_polity_id,
        .x = occupation.x,
        .y = occupation.y,
        .summary = summary.str(),
    });
}

OccupationRecord OccupationFromCampaign(int id, Turn turn, const WarCampaign& campaign,
                                        const std::vector<Polity>& polities) {
    const auto* occupier = PolityById(polities, campaign.actor_polity_id);
    const float occupier_stability = occupier == nullptr ? 0.55F : occupier->stability;

    OccupationRecord occupation;
    occupation.id = id;
    occupation.source_campaign_id = campaign.id;
    occupation.occupier_polity_id = campaign.actor_polity_id;
    occupation.previous_owner_polity_id = campaign.target_polity_id;
    occupation.target_kind = campaign.target_kind;
    occupation.settlement_id = campaign.target_settlement_id;
    occupation.x = campaign.x;
    occupation.y = campaign.y;
    occupation.started_turn = campaign.ended_turn >= 0 ? campaign.ended_turn : turn;
    occupation.last_update_turn = turn;
    occupation.occupation_value = std::max(0.08F, campaign.target_value);
    occupation.maintenance_cost = std::max(0.18F, campaign.occupation_cost * 1.30F + campaign.campaign_cost * 0.08F);
    occupation.unrest = std::clamp(
        0.28F + campaign.occupation_cost * 0.28F - std::clamp(occupier_stability, 0.0F, 1.0F) * 0.10F, 0.12F, 0.85F);
    occupation.integration = std::clamp(0.12F + std::max(0.0F, campaign.occupation_profit) * 0.10F, 0.05F, 0.35F);
    occupation.revolt_risk = std::clamp(occupation.unrest - occupation.integration * 0.25F, 0.0F, 1.0F);
    occupation.border_stability_delta = occupation.unrest * 0.12F + occupation.maintenance_cost * 0.04F;
    occupation.outcome_reason = "occupation opened from successful campaign";
    return occupation;
}

void RegisterNewOccupations(Turn turn, const std::vector<WarCampaign>& campaigns, std::vector<Polity>& polities,
                            std::vector<OccupationRecord>& occupations) {
    for (const auto& campaign : campaigns) {
        if (campaign.status != WarCampaignStatus::Occupied || HasOccupationForCampaign(occupations, campaign.id)) {
            continue;
        }
        occupations.push_back(OccupationFromCampaign(NextOccupationId(occupations), turn, campaign, polities));
    }
}

void ResetPolityOccupationPressure(std::vector<Polity>& polities) {
    for (auto& polity : polities) {
        polity.occupation_load = 0.0F;
        polity.occupation_unrest = 0.0F;
        polity.occupied_settlements = 0;
        polity.vassal_count = 0;
    }
}

void ApplyOccupationPressure(std::vector<Polity>& polities, const std::vector<OccupationRecord>& occupations) {
    ResetPolityOccupationPressure(polities);
    for (const auto& occupation : occupations) {
        auto* occupier = PolityById(polities, occupation.occupier_polity_id);
        if (occupier == nullptr) {
            continue;
        }
        if (occupation.status == OccupationStatus::Active) {
            occupier->occupation_load += occupation.maintenance_cost * (1.0F + occupation.unrest);
            occupier->occupation_unrest += occupation.unrest + occupation.border_stability_delta;
            occupier->occupied_settlements += occupation.settlement_id >= 0 ? 1 : 0;
        } else if (occupation.status == OccupationStatus::Vassalized) {
            ++occupier->vassal_count;
        }
    }
}

void ResolveOccupation(World& world, Turn turn, std::vector<Settlement>& settlements, std::vector<Polity>& polities,
                       OccupationRecord& occupation, OccupationStatus status, const std::string& reason,
                       std::vector<DiplomacyRelation>& diplomacy_relations, EventLog& event_log) {
    occupation.status = status;
    occupation.ended_turn = turn;
    occupation.last_update_turn = turn;
    occupation.outcome_reason = reason;

    EventType type = EventType::OccupationWithdrawn;
    if (status == OccupationStatus::Ceded) {
        KeepTarget(world, settlements, polities, occupation);
        type = EventType::TerritoryCeded;
        DiplomacySystem::RecordIncident(diplomacy_relations, turn, occupation.occupier_polity_id,
                                        occupation.previous_owner_polity_id, DiplomaticIncidentKind::TerritoryCeded,
                                        occupation.occupation_value);
    } else if (status == OccupationStatus::Vassalized) {
        occupation.subject_polity_id = occupation.previous_owner_polity_id;
        ReturnTarget(world, settlements, polities, occupation);
        type = EventType::VassalCreated;
        DiplomacySystem::RecordIncident(diplomacy_relations, turn, occupation.occupier_polity_id,
                                        occupation.previous_owner_polity_id, DiplomaticIncidentKind::VassalCreated,
                                        occupation.occupation_value);
    } else if (status == OccupationStatus::Revolted) {
        ReturnTarget(world, settlements, polities, occupation);
        type = EventType::OccupationRevolt;
        DiplomacySystem::RecordIncident(diplomacy_relations, turn, occupation.occupier_polity_id,
                                        occupation.previous_owner_polity_id, DiplomaticIncidentKind::OccupationRevolt,
                                        occupation.unrest);
    } else {
        ReturnTarget(world, settlements, polities, occupation);
        type = EventType::OccupationWithdrawn;
        DiplomacySystem::RecordIncident(diplomacy_relations, turn, occupation.occupier_polity_id,
                                        occupation.previous_owner_polity_id,
                                        DiplomaticIncidentKind::OccupationWithdrawn, occupation.cumulative_shortfall);
    }

    AddOccupationEvent(turn, occupation, type, event_log);
}

bool OccupierStillControlsTarget(const World& world, const std::vector<Settlement>& settlements,
                                 const OccupationRecord& occupation) {
    if (occupation.settlement_id >= 0) {
        const auto* settlement = SettlementById(settlements, occupation.settlement_id);
        return settlement != nullptr && settlement->polity_id == occupation.occupier_polity_id;
    }
    if (!world.InBounds(occupation.x, occupation.y)) {
        return false;
    }
    const auto& tile = world.At(occupation.x, occupation.y);
    return tile.owner_polity_id == occupation.occupier_polity_id ||
           tile.controller_polity_id == occupation.occupier_polity_id;
}

void AdvanceOccupation(World& world, Turn turn, std::vector<Settlement>& settlements, std::vector<Polity>& polities,
                       OccupationRecord& occupation, std::vector<DiplomacyRelation>& diplomacy_relations,
                       EventLog& event_log) {
    if (occupation.status != OccupationStatus::Active) {
        return;
    }

    auto* occupier = PolityById(polities, occupation.occupier_polity_id);
    if (occupier == nullptr) {
        ResolveOccupation(world, turn, settlements, polities, occupation, OccupationStatus::Withdrawn,
                          "occupier polity disappeared", diplomacy_relations, event_log);
        return;
    }
    if (!OccupierStillControlsTarget(world, settlements, occupation)) {
        ResolveOccupation(world, turn, settlements, polities, occupation, OccupationStatus::Withdrawn,
                          "occupier no longer controls target", diplomacy_relations, event_log);
        return;
    }

    const float annual_maintenance =
        occupation.maintenance_cost * (0.85F + occupation.unrest * 0.75F + occupier->overextension * 0.50F);
    const float fulfillment = ConsumeOccupationMaintenance(settlements, *occupier, annual_maintenance);
    occupation.cumulative_maintenance += annual_maintenance * fulfillment;
    occupation.cumulative_shortfall += annual_maintenance * (1.0F - fulfillment);

    const float unrest_delta = 0.025F + (1.0F - fulfillment) * 0.16F + occupier->overextension * 0.05F -
                               occupier->stability * 0.035F - occupation.integration * 0.012F;
    occupation.unrest = std::clamp(occupation.unrest + unrest_delta, 0.0F, 1.20F);

    const float integration_delta = fulfillment * (0.025F + occupier->stability * 0.025F) +
                                    std::max(0.0F, occupation.occupation_value - annual_maintenance) * 0.006F -
                                    occupation.unrest * 0.010F;
    occupation.integration = std::clamp(occupation.integration + integration_delta, 0.0F, 1.20F);
    occupation.revolt_risk = std::clamp(
        occupation.unrest + occupation.cumulative_shortfall / (occupation.occupation_value * 8.0F + 1.0F) * 0.25F +
            occupier->overextension * 0.15F - occupation.integration * 0.25F,
        0.0F, 1.0F);
    occupation.border_stability_delta = std::clamp(
        0.06F + occupation.unrest * 0.14F + annual_maintenance * 0.04F - occupation.integration * 0.08F, 0.0F, 0.35F);
    occupation.last_update_turn = turn;
    ++occupation.turns_held;

    if (occupation.turns_held >= kMinimumResolutionTurns && occupation.integration >= kCessionIntegrationThreshold &&
        occupation.revolt_risk < 0.45F) {
        ResolveOccupation(world, turn, settlements, polities, occupation, OccupationStatus::Ceded,
                          "occupation integrated into occupier", diplomacy_relations, event_log);
        return;
    }
    if (occupation.turns_held >= kMinimumResolutionTurns &&
        (occupation.revolt_risk >= kRevoltRiskThreshold || occupation.unrest > 1.02F)) {
        ResolveOccupation(world, turn, settlements, polities, occupation, OccupationStatus::Revolted,
                          "occupation unrest became unmanageable", diplomacy_relations, event_log);
        return;
    }
    if (occupation.turns_held >= kMinimumResolutionTurns &&
        (occupation.cumulative_shortfall > occupation.occupation_value * 2.4F + 1.0F ||
         (occupier->stability < 0.18F && occupation.unrest > 0.65F))) {
        ResolveOccupation(world, turn, settlements, polities, occupation, OccupationStatus::Withdrawn,
                          "occupation maintenance became uneconomic", diplomacy_relations, event_log);
        return;
    }
    if (occupation.turns_held >= kVassalResolutionTurns && occupation.integration < 0.55F &&
        occupation.unrest >= 0.52F && occupation.unrest < kRevoltRiskThreshold &&
        PolityById(polities, occupation.previous_owner_polity_id) != nullptr) {
        ResolveOccupation(world, turn, settlements, polities, occupation, OccupationStatus::Vassalized,
                          "occupation converted into vassal buffer", diplomacy_relations, event_log);
    }
}

} // namespace

void OccupationSystem::Reset(std::vector<OccupationRecord>& occupations, std::vector<Polity>& polities) {
    occupations.clear();
    ResetPolityOccupationPressure(polities);
}

void OccupationSystem::UpdateOccupations(World& world, Turn turn, std::vector<Settlement>& settlements,
                                         std::vector<Polity>& polities, const std::vector<WarCampaign>& campaigns,
                                         std::vector<OccupationRecord>& occupations,
                                         std::vector<DiplomacyRelation>& diplomacy_relations, EventLog& event_log) {
    RegisterNewOccupations(turn, campaigns, polities, occupations);
    for (auto& occupation : occupations) {
        AdvanceOccupation(world, turn, settlements, polities, occupation, diplomacy_relations, event_log);
    }
    ApplyOccupationPressure(polities, occupations);
}

} // namespace oikumene
