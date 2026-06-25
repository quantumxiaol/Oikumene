#include "oikumene/sim/war_system.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace oikumene {
namespace {

constexpr int kWarOpenInterval = 10;
constexpr int kMaxActiveCampaigns = 6;
constexpr float kDeclarationThreshold = 0.72F;
constexpr float kOccupationProfitThreshold = -0.05F;

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

int NextCampaignId(const std::vector<WarCampaign>& campaigns) {
    int next = 0;
    for (const auto& campaign : campaigns) {
        next = std::max(next, campaign.id + 1);
    }
    return next;
}

int ActiveCampaignCount(const std::vector<WarCampaign>& campaigns) {
    int count = 0;
    for (const auto& campaign : campaigns) {
        count += campaign.status == WarCampaignStatus::Active ? 1 : 0;
    }
    return count;
}

bool HasActiveCampaignFor(const std::vector<WarCampaign>& campaigns, PolityId actor_id, PolityId target_id) {
    return std::any_of(campaigns.begin(), campaigns.end(), [&](const WarCampaign& campaign) {
        return campaign.status == WarCampaignStatus::Active &&
               (campaign.actor_polity_id == actor_id || campaign.target_polity_id == actor_id ||
                campaign.actor_polity_id == target_id || campaign.target_polity_id == target_id);
    });
}

bool HasCampaignForTarget(const std::vector<WarCampaign>& campaigns, const WarTargetCandidate& target) {
    return std::any_of(campaigns.begin(), campaigns.end(), [&](const WarCampaign& campaign) {
        return campaign.actor_polity_id == target.actor_polity_id &&
               campaign.target_polity_id == target.target_polity_id && campaign.x == target.x &&
               campaign.y == target.y && campaign.target_settlement_id == target.target_settlement_id &&
               campaign.target_trade_id == target.target_trade_id;
    });
}

bool TargetStillValid(const World& world, const std::vector<Settlement>& settlements,
                      const WarTargetCandidate& target) {
    if (target.target_settlement_id >= 0) {
        const auto* settlement = SettlementById(settlements, target.target_settlement_id);
        return settlement != nullptr && settlement->polity_id == target.target_polity_id;
    }
    if (!world.InBounds(target.x, target.y)) {
        return false;
    }
    const auto& tile = world.At(target.x, target.y);
    return tile.controller_polity_id == target.target_polity_id || tile.owner_polity_id == target.target_polity_id;
}

float ConsumeFood(std::vector<Settlement>& settlements, const Polity& polity, float amount) {
    if (amount <= 0.0F) {
        return 1.0F;
    }
    float remaining = amount;
    for (const int settlement_id : polity.member_settlement_ids) {
        auto* settlement = SettlementById(settlements, settlement_id);
        if (settlement == nullptr) {
            continue;
        }
        const float taken = std::min(settlement->stockpile.food, remaining);
        settlement->stockpile.food -= taken;
        remaining -= taken;
        if (remaining <= 0.0F) {
            return 1.0F;
        }
    }
    return std::clamp((amount - remaining) / amount, 0.0F, 1.0F);
}

float ConsumeEquipment(std::vector<Settlement>& settlements, const Polity& polity, float amount) {
    if (amount <= 0.0F) {
        return 1.0F;
    }
    float remaining = amount;
    for (const int settlement_id : polity.member_settlement_ids) {
        auto* settlement = SettlementById(settlements, settlement_id);
        if (settlement == nullptr) {
            continue;
        }
        const float metal_taken = std::min(settlement->stockpile.metal * 1.6F, remaining);
        settlement->stockpile.metal -= metal_taken / 1.6F;
        remaining -= metal_taken;
        const float ore_taken = std::min(settlement->stockpile.ore, remaining);
        settlement->stockpile.ore -= ore_taken;
        remaining -= ore_taken;
        const float wood_taken = std::min(settlement->stockpile.wood * 0.45F, remaining);
        settlement->stockpile.wood -= wood_taken / 0.45F;
        remaining -= wood_taken;
        if (remaining <= 0.0F) {
            return 1.0F;
        }
    }
    return std::clamp((amount - remaining) / amount, 0.0F, 1.0F);
}

int ApplyPopulationLoss(std::vector<Settlement>& settlements, const Polity& polity, float loss) {
    int remaining = loss <= 0.0F ? 0 : std::max(1, static_cast<int>(std::ceil(loss)));
    int applied = 0;
    for (const int settlement_id : polity.member_settlement_ids) {
        auto* settlement = SettlementById(settlements, settlement_id);
        if (settlement == nullptr || settlement->population <= 12) {
            continue;
        }
        const int safe_available = std::max(0, settlement->population - 12);
        const int taken = std::min(safe_available, remaining);
        settlement->population -= taken;
        remaining -= taken;
        applied += taken;
        if (remaining <= 0) {
            break;
        }
    }
    return applied;
}

void RepairCapitalAfterCapture(std::vector<Settlement>& settlements, Polity& polity) {
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

WarCampaign CampaignFromTarget(int id, Turn turn, const WarTargetCandidate& target, const Polity& actor,
                               const Polity& defender) {
    const float actor_population = static_cast<float>(std::max(1, actor.population));
    const float defender_population = static_cast<float>(std::max(1, defender.population));
    WarCampaign campaign;
    campaign.id = id;
    campaign.target_candidate_id = target.id;
    campaign.actor_polity_id = target.actor_polity_id;
    campaign.target_polity_id = target.target_polity_id;
    campaign.objective = target.objective;
    campaign.target_kind = target.kind;
    campaign.x = target.x;
    campaign.y = target.y;
    campaign.target_settlement_id = target.target_settlement_id;
    campaign.target_trade_id = target.target_trade_id;
    campaign.path = target.path;
    campaign.started_turn = turn;
    campaign.last_update_turn = turn;
    campaign.target_value = target.target_value;
    campaign.campaign_cost = target.campaign_cost;
    campaign.occupation_cost = target.occupation_cost;
    campaign.action_score = target.action_score;
    campaign.mobilized_manpower = std::max(8.0F, actor_population * (0.030F + target.action_score * 0.035F));
    campaign.attacker_strength =
        actor.military_potential + actor.tool_efficiency * 8.0F + campaign.mobilized_manpower * 0.18F;
    campaign.defender_strength =
        defender.military_potential + defender_population * 0.035F + target.defense_cost * 14.0F;
    campaign.occupation_profit = target.target_value - target.occupation_cost;
    campaign.outcome_reason = "campaign opened from war target candidate";
    return campaign;
}

void AddWarDeclaredEvent(Turn turn, const WarCampaign& campaign, EventLog& event_log) {
    std::ostringstream summary;
    summary << "War " << campaign.id << " declared P" << campaign.actor_polity_id << " -> P"
            << campaign.target_polity_id << " over " << ToString(campaign.target_kind) << " at (" << campaign.x << ","
            << campaign.y << "), score " << campaign.action_score;
    event_log.Add(SimEvent{
        .turn = turn,
        .type = EventType::WarDeclared,
        .actor_id = campaign.actor_polity_id,
        .x = campaign.x,
        .y = campaign.y,
        .summary = summary.str(),
    });
}

void AddWarEndedEvent(Turn turn, const WarCampaign& campaign, EventType type, EventLog& event_log) {
    std::ostringstream summary;
    summary << "War " << campaign.id << " " << ToString(campaign.status) << " P" << campaign.actor_polity_id << " -> P"
            << campaign.target_polity_id << ", pop lost " << campaign.population_lost << ", food "
            << campaign.food_spent << ", equipment " << campaign.equipment_spent << ", progress " << campaign.progress
            << ", reason: " << campaign.outcome_reason;
    event_log.Add(SimEvent{
        .turn = turn,
        .type = type,
        .actor_id = campaign.actor_polity_id,
        .x = campaign.x,
        .y = campaign.y,
        .summary = summary.str(),
    });
}

void OccupyTarget(World& world, std::vector<Settlement>& settlements, std::vector<Polity>& polities,
                  WarCampaign& campaign) {
    auto* actor = PolityById(polities, campaign.actor_polity_id);
    auto* defender = PolityById(polities, campaign.target_polity_id);
    if (actor == nullptr || defender == nullptr) {
        campaign.outcome_reason = "missing polity during occupation";
        return;
    }

    if (campaign.target_settlement_id >= 0) {
        auto* settlement = SettlementById(settlements, campaign.target_settlement_id);
        if (settlement != nullptr) {
            RemoveSettlement(*defender, settlement->id);
            if (!ContainsSettlement(*actor, settlement->id)) {
                actor->member_settlement_ids.push_back(settlement->id);
            }
            settlement->polity_id = actor->id;
            settlement->is_capital = false;
            RepairCapitalAfterCapture(settlements, *defender);
            campaign.outcome_reason = "settlement occupied";
        }
    } else if (world.InBounds(campaign.x, campaign.y)) {
        auto& tile = world.At(campaign.x, campaign.y);
        tile.owner_polity_id = actor->id;
        tile.controller_polity_id = actor->id;
        campaign.outcome_reason = "target tile occupied";
    }

    RecountPolitySnapshot(settlements, *actor);
    RecountPolitySnapshot(settlements, *defender);
}

void AdvanceCampaign(World& world, Turn turn, std::vector<Settlement>& settlements, std::vector<Polity>& polities,
                     WarCampaign& campaign, EventLog& event_log) {
    auto* actor = PolityById(polities, campaign.actor_polity_id);
    auto* defender = PolityById(polities, campaign.target_polity_id);
    if (actor == nullptr || defender == nullptr || actor->population <= 0) {
        campaign.status = WarCampaignStatus::Withdrawn;
        campaign.ended_turn = turn;
        campaign.outcome_reason = "missing polity or exhausted attacker";
        AddWarEndedEvent(turn, campaign, EventType::WarRetreated, event_log);
        return;
    }

    const float food_need = 0.24F + campaign.mobilized_manpower * 0.012F + campaign.campaign_cost * 0.06F;
    const float equipment_need = 0.04F + campaign.campaign_cost * 0.035F;
    const float food_fulfillment = ConsumeFood(settlements, *actor, food_need);
    const float equipment_fulfillment = ConsumeEquipment(settlements, *actor, equipment_need);
    campaign.food_spent += food_need * food_fulfillment;
    campaign.equipment_spent += equipment_need * equipment_fulfillment;

    const float fulfillment = (food_fulfillment * 0.62F + equipment_fulfillment * 0.38F);
    campaign.supply_failures += fulfillment < 0.48F ? 1 : 0;
    const float loss = std::max(0.45F, campaign.mobilized_manpower *
                                           (0.010F + campaign.campaign_cost * 0.0030F + (1.0F - fulfillment) * 0.010F));
    campaign.population_lost += static_cast<float>(ApplyPopulationLoss(settlements, *actor, loss));

    const float strength_ratio =
        campaign.attacker_strength / std::max(1.0F, campaign.attacker_strength + campaign.defender_strength);
    const float progress_gain =
        std::max(0.015F, (0.060F + campaign.action_score * 0.070F + strength_ratio * 0.055F) * fulfillment -
                             campaign.campaign_cost * 0.006F);
    campaign.progress += progress_gain;
    campaign.last_update_turn = turn;
    ++campaign.turns_active;

    if (campaign.supply_failures >= 4 || (campaign.turns_active >= 8 && campaign.progress < 0.32F)) {
        campaign.status = WarCampaignStatus::Withdrawn;
        campaign.ended_turn = turn;
        campaign.outcome_reason = "supply failure or stalled campaign";
        AddWarEndedEvent(turn, campaign, EventType::WarRetreated, event_log);
        return;
    }

    if (campaign.progress < 1.0F) {
        return;
    }

    if (campaign.occupation_profit >= kOccupationProfitThreshold) {
        OccupyTarget(world, settlements, polities, campaign);
        campaign.status = WarCampaignStatus::Occupied;
        campaign.ended_turn = turn;
        AddWarEndedEvent(turn, campaign, EventType::WarTargetOccupied, event_log);
    } else {
        campaign.status = WarCampaignStatus::Peace;
        campaign.ended_turn = turn;
        campaign.outcome_reason = "campaign succeeded but occupation was not profitable";
        AddWarEndedEvent(turn, campaign, EventType::PeaceSigned, event_log);
    }
}

void OpenNewCampaigns(const World& world, Turn turn, const std::vector<Settlement>& settlements,
                      const std::vector<Polity>& polities, const std::vector<WarTargetCandidate>& targets,
                      std::vector<WarCampaign>& campaigns, EventLog& event_log) {
    if (turn <= 0 || turn % kWarOpenInterval != 0 || ActiveCampaignCount(campaigns) >= kMaxActiveCampaigns) {
        return;
    }

    for (const auto& target : targets) {
        if (target.action_score < kDeclarationThreshold || !target.high_value) {
            continue;
        }
        const auto* actor = PolityById(polities, target.actor_polity_id);
        const auto* defender = PolityById(polities, target.target_polity_id);
        if (actor == nullptr || defender == nullptr || actor->population < 80 || actor->stability < 0.22F) {
            continue;
        }
        if (HasActiveCampaignFor(campaigns, target.actor_polity_id, target.target_polity_id)) {
            continue;
        }
        if (HasCampaignForTarget(campaigns, target) || !TargetStillValid(world, settlements, target)) {
            continue;
        }
        campaigns.push_back(CampaignFromTarget(NextCampaignId(campaigns), turn, target, *actor, *defender));
        AddWarDeclaredEvent(turn, campaigns.back(), event_log);
        if (ActiveCampaignCount(campaigns) >= kMaxActiveCampaigns) {
            return;
        }
    }
}

} // namespace

void WarSystem::Reset(std::vector<WarCampaign>& campaigns) {
    campaigns.clear();
}

void WarSystem::UpdateWars(World& world, Turn turn, std::vector<Settlement>& settlements, std::vector<Polity>& polities,
                           const std::vector<TradeAgreement>& trades, const std::vector<WarTargetCandidate>& targets,
                           std::vector<WarCampaign>& campaigns, EventLog& event_log) {
    (void)trades;
    for (auto& campaign : campaigns) {
        if (campaign.status == WarCampaignStatus::Active) {
            AdvanceCampaign(world, turn, settlements, polities, campaign, event_log);
        }
    }
    OpenNewCampaigns(world, turn, settlements, polities, targets, campaigns, event_log);
}

} // namespace oikumene
