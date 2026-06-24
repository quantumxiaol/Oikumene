#include "oikumene/sim/war_target.hpp"

namespace oikumene {

std::string ToString(WarTargetKind kind) {
    switch (kind) {
    case WarTargetKind::Settlement:
        return "Settlement";
    case WarTargetKind::ResourceRegion:
        return "ResourceRegion";
    case WarTargetKind::ContestedBorder:
        return "ContestedBorder";
    case WarTargetKind::TradeRouteNode:
        return "TradeRouteNode";
    case WarTargetKind::StrategicPass:
        return "StrategicPass";
    }
    return "Unknown";
}

} // namespace oikumene
