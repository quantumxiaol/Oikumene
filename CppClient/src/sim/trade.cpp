#include "oikumene/sim/trade.hpp"

namespace oikumene {

std::string ToString(TradeGood good) {
    switch (good) {
    case TradeGood::None:
        return "None";
    case TradeGood::Food:
        return "Food";
    case TradeGood::Wood:
        return "Wood";
    case TradeGood::Ore:
        return "Ore";
    case TradeGood::Wealth:
        return "Wealth";
    }
    return "Unknown";
}

} // namespace oikumene
