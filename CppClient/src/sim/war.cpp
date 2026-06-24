#include "oikumene/sim/war.hpp"

namespace oikumene {

std::string ToString(WarObjective objective) {
    switch (objective) {
    case WarObjective::BorderDispute:
        return "BorderDispute";
    case WarObjective::TradeCoercion:
        return "TradeCoercion";
    case WarObjective::Blockade:
        return "Blockade";
    case WarObjective::DependencyBreakout:
        return "DependencyBreakout";
    }
    return "Unknown";
}

} // namespace oikumene
