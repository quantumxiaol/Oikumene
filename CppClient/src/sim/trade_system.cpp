#include "oikumene/sim/trade_system.hpp"

#include <algorithm>
#include <sstream>

#include "oikumene/sim/trade_planner.hpp"

namespace oikumene {
namespace {

constexpr int kTradeRefreshInterval = 10;
constexpr int kTradeOpenInterval = 15;
constexpr int kMaxTradesPerPolity = 3;
constexpr int kWeakTradeGraceRefreshes = 3;
constexpr float kKeepAliveProfit = -0.18F;
constexpr float kKeepAliveComplementarity = 0.08F;

const Polity* PolityById(const std::vector<Polity>& polities, PolityId id) {
    for (const auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

Polity* MutablePolityById(std::vector<Polity>& polities, PolityId id) {
    for (auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

int NextTradeId(const std::vector<TradeAgreement>& trades) {
    int next = 0;
    for (const auto& trade : trades) {
        next = std::max(next, trade.id + 1);
    }
    return next;
}

bool SamePair(const TradeAgreement& trade, PolityId a, PolityId b) {
    return (trade.polity_a_id == a && trade.polity_b_id == b) || (trade.polity_a_id == b && trade.polity_b_id == a);
}

bool HasActiveTrade(const std::vector<TradeAgreement>& trades, PolityId a, PolityId b) {
    return std::any_of(trades.begin(), trades.end(),
                       [&](const TradeAgreement& trade) { return trade.active && SamePair(trade, a, b); });
}

int ActiveTradeCountFor(const std::vector<TradeAgreement>& trades, PolityId polity_id) {
    int count = 0;
    for (const auto& trade : trades) {
        if (trade.active && (trade.polity_a_id == polity_id || trade.polity_b_id == polity_id)) {
            ++count;
        }
    }
    return count;
}

TradeAgreement AgreementFromCandidate(int id, Turn turn, const TradeCandidate& candidate) {
    return TradeAgreement{
        .id = id,
        .polity_a_id = candidate.polity_a_id,
        .polity_b_id = candidate.polity_b_id,
        .opened_turn = turn,
        .last_evaluated_turn = turn,
        .active = true,
        .weak_refresh_count = 0,
        .export_from_a = candidate.export_from_a,
        .export_from_b = candidate.export_from_b,
        .value_a_to_b = candidate.value_a_to_b,
        .value_b_to_a = candidate.value_b_to_a,
        .complementarity = candidate.complementarity,
        .route_cost = candidate.route_cost,
        .route_cost_without_network = candidate.route_cost_without_network,
        .route_saving = candidate.route_saving,
        .route_efficiency = candidate.route_efficiency,
        .gross_value = candidate.gross_value,
        .transport_cost = candidate.transport_cost,
        .expected_profit = candidate.expected_profit,
        .path = candidate.path,
        .reason = candidate.reason,
    };
}

void UpdateAgreementFromCandidate(TradeAgreement& agreement, Turn turn, const TradeCandidate& candidate,
                                  bool profitable) {
    agreement.last_evaluated_turn = turn;
    agreement.active = true;
    agreement.weak_refresh_count = profitable ? 0 : agreement.weak_refresh_count + 1;
    agreement.export_from_a = candidate.export_from_a;
    agreement.export_from_b = candidate.export_from_b;
    agreement.value_a_to_b = candidate.value_a_to_b;
    agreement.value_b_to_a = candidate.value_b_to_a;
    agreement.complementarity = candidate.complementarity;
    agreement.route_cost = candidate.route_cost;
    agreement.route_cost_without_network = candidate.route_cost_without_network;
    agreement.route_saving = candidate.route_saving;
    agreement.route_efficiency = candidate.route_efficiency;
    agreement.gross_value = candidate.gross_value;
    agreement.transport_cost = candidate.transport_cost;
    agreement.expected_profit = candidate.expected_profit;
    agreement.path = candidate.path;
    agreement.reason = candidate.reason;
}

bool CanKeepWeakTrade(const TradeCandidate& candidate) {
    if (!candidate.viable && candidate.path.empty()) {
        return false;
    }
    if (candidate.route_cost <= 0.0F || candidate.route_efficiency <= 0.0F) {
        return false;
    }
    return candidate.complementarity >= kKeepAliveComplementarity || candidate.expected_profit >= kKeepAliveProfit;
}

void ClearPolityTradeStats(std::vector<Polity>& polities) {
    for (auto& polity : polities) {
        polity.trade_ids.clear();
        polity.active_trade_count = 0;
        polity.trade_income = 0.0F;
        polity.trade_expense = 0.0F;
        polity.trade_profit = 0.0F;
        polity.trade_complementarity = 0.0F;
        polity.trade_route_cost = 0.0F;
        polity.trade_route_efficiency = 0.0F;
    }
}

void ApplyTradeToPolity(Polity& polity, const TradeAgreement& trade) {
    polity.trade_ids.push_back(trade.id);
    ++polity.active_trade_count;
    const float income = trade.gross_value * 0.50F;
    const float expense = trade.transport_cost * 0.50F;
    const float profit = trade.expected_profit * 0.50F;
    polity.trade_income += income;
    polity.trade_expense += expense;
    polity.trade_profit += profit;
    polity.trade_complementarity += trade.complementarity;
    polity.trade_route_cost += trade.route_cost;
    polity.trade_route_efficiency += trade.route_efficiency;
    polity.budget.wealth_income += income;
    polity.budget.wealth_surplus += profit;
}

void RefreshPolityTradeStats(std::vector<Polity>& polities, const std::vector<TradeAgreement>& trades) {
    ClearPolityTradeStats(polities);
    for (const auto& trade : trades) {
        if (!trade.active) {
            continue;
        }
        if (auto* polity = MutablePolityById(polities, trade.polity_a_id)) {
            ApplyTradeToPolity(*polity, trade);
        }
        if (auto* polity = MutablePolityById(polities, trade.polity_b_id)) {
            ApplyTradeToPolity(*polity, trade);
        }
    }
    for (auto& polity : polities) {
        if (polity.active_trade_count > 0) {
            const float count = static_cast<float>(polity.active_trade_count);
            polity.trade_route_cost /= count;
            polity.trade_route_efficiency /= count;
        }
    }
}

void AddTradeOpenedEvent(Turn turn, const TradeAgreement& trade, EventLog& event_log) {
    std::ostringstream summary;
    summary << "Trade " << trade.id << " opened between polity " << trade.polity_a_id << " and polity "
            << trade.polity_b_id << ", goods " << ToString(trade.export_from_a) << "/" << ToString(trade.export_from_b)
            << ", profit " << trade.expected_profit << ", route cost " << trade.route_cost;
    event_log.Add(SimEvent{
        .turn = turn,
        .type = EventType::TradeOpened,
        .actor_id = trade.polity_a_id,
        .x = 0,
        .y = 0,
        .summary = summary.str(),
    });
}

} // namespace

void TradeSystem::Reset(std::vector<TradeAgreement>& trades, std::vector<Polity>& polities) {
    trades.clear();
    ClearPolityTradeStats(polities);
}

void TradeSystem::UpdateTrades(const World& world, Turn turn, const std::vector<Settlement>& settlements,
                               std::vector<Polity>& polities, std::vector<TradeAgreement>& trades,
                               EventLog& event_log) {
    if (polities.size() < 2) {
        Reset(trades, polities);
        return;
    }

    const bool refresh_turn = turn > 0 && turn % kTradeRefreshInterval == 0;
    if (refresh_turn) {
        for (auto& trade : trades) {
            const auto* polity_a = PolityById(polities, trade.polity_a_id);
            const auto* polity_b = PolityById(polities, trade.polity_b_id);
            if (polity_a == nullptr || polity_b == nullptr) {
                trade.active = false;
                continue;
            }
            const auto candidate = BuildTradeCandidate(world, settlements, *polity_a, *polity_b);
            const bool profitable = IsTradeCandidateProfitable(candidate);
            if (!profitable && !CanKeepWeakTrade(candidate)) {
                trade.active = false;
                continue;
            }
            UpdateAgreementFromCandidate(trade, turn, candidate, profitable);
            if (trade.weak_refresh_count > kWeakTradeGraceRefreshes) {
                trade.active = false;
            }
        }

        trades.erase(
            std::remove_if(trades.begin(), trades.end(), [](const TradeAgreement& trade) { return !trade.active; }),
            trades.end());
    }

    const bool open_turn = turn > 0 && turn % kTradeOpenInterval == 0;
    if (open_turn) {
        const auto candidates = BuildTradeCandidates(world, settlements, polities);
        for (const auto& candidate : candidates) {
            if (HasActiveTrade(trades, candidate.polity_a_id, candidate.polity_b_id)) {
                continue;
            }
            if (ActiveTradeCountFor(trades, candidate.polity_a_id) >= kMaxTradesPerPolity ||
                ActiveTradeCountFor(trades, candidate.polity_b_id) >= kMaxTradesPerPolity) {
                continue;
            }
            trades.push_back(AgreementFromCandidate(NextTradeId(trades), turn, candidate));
            AddTradeOpenedEvent(turn, trades.back(), event_log);
        }
    }

    RefreshPolityTradeStats(polities, trades);
}

} // namespace oikumene
