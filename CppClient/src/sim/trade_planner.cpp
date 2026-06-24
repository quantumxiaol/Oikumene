#include "oikumene/sim/trade_planner.hpp"

#include <algorithm>

#include "oikumene/sim/pathfinding.hpp"
#include "oikumene/sim/technology.hpp"

namespace oikumene {
namespace {

constexpr float kMaxTradePathCost = 180.0F;
constexpr float kMinimumComplementarity = 0.18F;
constexpr float kMinimumExpectedProfit = 0.08F;

struct TradeBalance {
    TradeGood good = TradeGood::None;
    float surplus = 0.0F;
    float demand = 0.0F;
    float weight = 1.0F;
};

const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id) {
    for (const auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

float OreDemandTarget(const Polity& polity) {
    float target = 0.12F + static_cast<float>(polity.population) * 0.00015F;
    if (HasTech(polity.research, TechId::Mining)) {
        target += 0.18F;
    }
    if (HasTech(polity.research, TechId::BronzeWorking)) {
        target += 0.32F;
    }
    if (HasTech(polity.research, TechId::Fortification)) {
        target += 0.16F;
    }
    return target;
}

std::vector<TradeBalance> BuildTradeBalances(const Polity& polity) {
    const float member_count = static_cast<float>(std::max(1, static_cast<int>(polity.member_settlement_ids.size())));
    const float population = static_cast<float>(std::max(1, polity.population));
    const float food_baseline = population * 0.0065F;
    const float wood_baseline = member_count * 0.36F;
    const float tradeable_food_capacity = std::max(0.0F, polity.budget.food_income - population * 0.0040F) * 0.35F;
    const float tradeable_wood_capacity = std::max(0.0F, polity.budget.wood_income - member_count * 0.18F) * 0.55F;
    const float tradeable_ore_capacity = std::max(0.0F, polity.budget.ore_income - 0.02F) * 0.85F;

    return {
        TradeBalance{
            .good = TradeGood::Food,
            .surplus = std::max(0.0F, polity.budget.food_surplus) + tradeable_food_capacity,
            .demand =
                std::max(0.0F, -polity.budget.food_surplus) + std::max(0.0F, food_baseline - polity.budget.food_income),
            .weight = 1.25F,
        },
        TradeBalance{
            .good = TradeGood::Wood,
            .surplus = std::max(0.0F, polity.budget.wood_surplus) + tradeable_wood_capacity,
            .demand =
                std::max(0.0F, -polity.budget.wood_surplus) + std::max(0.0F, wood_baseline - polity.budget.wood_income),
            .weight = 1.00F,
        },
        TradeBalance{
            .good = TradeGood::Ore,
            .surplus = tradeable_ore_capacity,
            .demand = std::max(0.0F, OreDemandTarget(polity) - polity.budget.ore_income),
            .weight = 2.10F,
        },
        TradeBalance{
            .good = TradeGood::Wealth,
            .surplus = std::max(0.0F, polity.budget.wealth_surplus),
            .demand = std::max(0.0F, -polity.budget.wealth_surplus),
            .weight = 0.75F,
        },
    };
}

float AddDirectionalMatches(const std::vector<TradeBalance>& exporter, const std::vector<TradeBalance>& importer,
                            TradeGood& primary_good, float& primary_value) {
    float total = 0.0F;
    for (const auto& export_good : exporter) {
        for (const auto& import_good : importer) {
            if (export_good.good != import_good.good) {
                continue;
            }
            const float flow = std::min(export_good.surplus, import_good.demand);
            const float value = flow * export_good.weight;
            total += value;
            if (value > primary_value) {
                primary_value = value;
                primary_good = export_good.good;
            }
        }
    }
    return total;
}

PathResult CapitalPath(const World& world, const Settlement& a, const Settlement& b, const Polity& polity_a,
                       const Polity& polity_b, bool use_routes) {
    PathOptions options;
    options.allow_ocean = HasTech(polity_a.research, TechId::Sailing) && HasTech(polity_b.research, TechId::Sailing);
    options.use_existing_routes = use_routes;
    options.route_polity_id = kInvalidPolityId;
    options.max_cost = kMaxTradePathCost;
    return FindPath(world, TileCoord{.x = a.x, .y = a.y}, TileCoord{.x = b.x, .y = b.y}, options);
}

bool MeetsTradeThreshold(const TradeCandidate& candidate) {
    return candidate.complementarity >= kMinimumComplementarity && candidate.expected_profit >= kMinimumExpectedProfit;
}

} // namespace

bool IsTradeCandidateProfitable(const TradeCandidate& candidate) {
    return candidate.viable && MeetsTradeThreshold(candidate);
}

TradeCandidate BuildTradeCandidate(const World& world, const std::vector<Settlement>& settlements,
                                   const Polity& polity_a, const Polity& polity_b) {
    TradeCandidate candidate;
    candidate.polity_a_id = polity_a.id;
    candidate.polity_b_id = polity_b.id;

    const auto* capital_a = SettlementById(settlements, polity_a.capital_settlement_id);
    const auto* capital_b = SettlementById(settlements, polity_b.capital_settlement_id);
    if (capital_a == nullptr || capital_b == nullptr || polity_a.id == polity_b.id) {
        candidate.reason = "missing capital or same polity";
        return candidate;
    }
    if (polity_a.stability < 0.25F || polity_b.stability < 0.25F) {
        candidate.reason = "unstable polity";
        return candidate;
    }

    const auto balance_a = BuildTradeBalances(polity_a);
    const auto balance_b = BuildTradeBalances(polity_b);
    candidate.value_a_to_b =
        AddDirectionalMatches(balance_a, balance_b, candidate.export_from_a, candidate.value_a_to_b);
    candidate.value_b_to_a =
        AddDirectionalMatches(balance_b, balance_a, candidate.export_from_b, candidate.value_b_to_a);
    candidate.complementarity = candidate.value_a_to_b + candidate.value_b_to_a;
    if (candidate.complementarity <= 0.0F) {
        candidate.reason = "no resource complementarity";
        return candidate;
    }

    const PathResult with_routes = CapitalPath(world, *capital_a, *capital_b, polity_a, polity_b, true);
    if (!with_routes.reachable) {
        candidate.reason = "no reachable trade path";
        return candidate;
    }
    const PathResult without_routes = CapitalPath(world, *capital_a, *capital_b, polity_a, polity_b, false);
    candidate.route_cost = with_routes.total_cost;
    candidate.route_cost_without_network =
        without_routes.reachable ? without_routes.total_cost : with_routes.total_cost;
    candidate.route_saving = std::max(0.0F, candidate.route_cost_without_network - candidate.route_cost);
    candidate.route_efficiency = std::clamp(1.0F - candidate.route_cost / kMaxTradePathCost, 0.0F, 1.0F);
    const float route_support =
        candidate.route_cost_without_network <= 0.0F
            ? 0.0F
            : std::clamp(candidate.route_saving / candidate.route_cost_without_network, 0.0F, 1.0F);
    candidate.gross_value =
        candidate.complementarity * (0.70F + candidate.route_efficiency * 0.85F + route_support * 0.35F);
    candidate.transport_cost = candidate.route_cost * 0.008F;
    candidate.expected_profit = candidate.gross_value - candidate.transport_cost;
    candidate.viable = MeetsTradeThreshold(candidate);
    candidate.reason = "resource complementarity over route network";
    return candidate;
}

std::vector<TradeCandidate> BuildTradeCandidates(const World& world, const std::vector<Settlement>& settlements,
                                                 const std::vector<Polity>& polities) {
    std::vector<TradeCandidate> candidates;
    for (std::size_t i = 0; i < polities.size(); ++i) {
        for (std::size_t j = i + 1; j < polities.size(); ++j) {
            auto candidate = BuildTradeCandidate(world, settlements, polities[i], polities[j]);
            if (candidate.viable) {
                candidates.push_back(candidate);
            }
        }
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.expected_profit > rhs.expected_profit; });
    return candidates;
}

} // namespace oikumene
