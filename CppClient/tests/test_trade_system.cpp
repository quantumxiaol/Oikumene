#include <cassert>
#include <iostream>
#include <vector>

#include "oikumene/sim/trade_planner.hpp"
#include "oikumene/sim/trade_system.hpp"

namespace {

oikumene::World MakeTradeWorld() {
    using namespace oikumene;
    World world(12, 5, 91);
    for (auto& tile : world.Tiles()) {
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.biome = Biome::Grassland;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.55F;
    }
    return world;
}

oikumene::Settlement MakeCapital(int id, int x, int y, oikumene::PolityId polity_id) {
    oikumene::Settlement settlement;
    settlement.id = id;
    settlement.x = x;
    settlement.y = y;
    settlement.level = oikumene::SettlementLevel::Village;
    settlement.population = 240;
    settlement.polity_id = polity_id;
    settlement.is_capital = true;
    return settlement;
}

oikumene::Polity MakeFoodPolity() {
    oikumene::Polity polity;
    polity.id = 0;
    polity.name = "River Grain";
    polity.capital_settlement_id = 0;
    polity.member_settlement_ids = {0};
    polity.population = 300;
    polity.stability = 1.0F;
    polity.budget.food_income = 18.0F;
    polity.budget.food_surplus = 10.0F;
    polity.budget.wood_income = 0.2F;
    polity.budget.wood_surplus = -2.0F;
    polity.budget.ore_income = 0.02F;
    polity.budget.wealth_surplus = 1.0F;
    return polity;
}

oikumene::Polity MakeWoodPolity() {
    oikumene::Polity polity;
    polity.id = 1;
    polity.name = "Forest Timber";
    polity.capital_settlement_id = 1;
    polity.member_settlement_ids = {1};
    polity.population = 280;
    polity.stability = 1.0F;
    polity.budget.food_income = 0.1F;
    polity.budget.food_surplus = -3.0F;
    polity.budget.wood_income = 9.0F;
    polity.budget.wood_surplus = 7.0F;
    polity.budget.ore_income = 0.03F;
    polity.budget.wealth_surplus = 0.7F;
    return polity;
}

void AddRoadBetweenCapitals(oikumene::World& world) {
    for (int x = 1; x <= 8; ++x) {
        auto& tile = world.At(x, 2);
        tile.has_route = true;
        tile.route_kind = oikumene::RouteKind::Road;
        tile.route_quality = oikumene::RouteQuality(oikumene::RouteKind::Road);
        tile.route_polity_id = 0;
        tile.route_id = 4;
    }
}

void TestTradeCandidateUsesResourceComplementarity() {
    auto world = MakeTradeWorld();
    std::vector<oikumene::Settlement> settlements{
        MakeCapital(0, 1, 2, 0),
        MakeCapital(1, 8, 2, 1),
    };
    const auto food = MakeFoodPolity();
    const auto wood = MakeWoodPolity();

    const auto candidate = oikumene::BuildTradeCandidate(world, settlements, food, wood);

    if (!candidate.viable) {
        std::cerr << "candidate failed: complementarity " << candidate.complementarity << " route_cost "
                  << candidate.route_cost << " gross " << candidate.gross_value << " transport "
                  << candidate.transport_cost << " profit " << candidate.expected_profit << " reason "
                  << candidate.reason << "\n";
    }
    assert(candidate.viable);
    assert(candidate.complementarity > 0.0F);
    assert(candidate.expected_profit > 0.0F);
    assert(candidate.export_from_a == oikumene::TradeGood::Food);
    assert(candidate.export_from_b == oikumene::TradeGood::Wood);
}

void TestRouteNetworkImprovesTradeRouteCost() {
    auto world = MakeTradeWorld();
    AddRoadBetweenCapitals(world);
    std::vector<oikumene::Settlement> settlements{
        MakeCapital(0, 1, 2, 0),
        MakeCapital(1, 8, 2, 1),
    };
    const auto food = MakeFoodPolity();
    const auto wood = MakeWoodPolity();

    const auto candidate = oikumene::BuildTradeCandidate(world, settlements, food, wood);

    assert(candidate.viable);
    assert(candidate.route_saving > 0.0F);
    assert(candidate.route_cost < candidate.route_cost_without_network);
}

void TestTradeSystemOpensAgreementAndAppliesProfit() {
    auto world = MakeTradeWorld();
    AddRoadBetweenCapitals(world);
    std::vector<oikumene::Settlement> settlements{
        MakeCapital(0, 1, 2, 0),
        MakeCapital(1, 8, 2, 1),
    };
    std::vector<oikumene::Polity> polities{
        MakeFoodPolity(),
        MakeWoodPolity(),
    };
    std::vector<oikumene::TradeAgreement> trades;
    oikumene::EventLog events;

    oikumene::TradeSystem::UpdateTrades(world, 15, settlements, polities, trades, events);

    assert(trades.size() == 1);
    assert(trades.front().active);
    assert(polities[0].active_trade_count == 1);
    assert(polities[1].active_trade_count == 1);
    assert(polities[0].trade_profit > 0.0F);
    assert(polities[1].trade_profit > 0.0F);
    assert(polities[0].budget.wealth_surplus > 1.0F);
    assert(events.Size() == 1);
    assert(events.Events().front().type == oikumene::EventType::TradeOpened);
}

} // namespace

int main() {
    TestTradeCandidateUsesResourceComplementarity();
    TestRouteNetworkImprovesTradeRouteCost();
    TestTradeSystemOpensAgreementAndAppliesProfit();
    std::cout << "trade system tests passed\n";
    return 0;
}
