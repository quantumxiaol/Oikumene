#include "oikumene/sim/route.hpp"

namespace oikumene {

std::string ToString(RoutePurpose purpose) {
    switch (purpose) {
        case RoutePurpose::Admin:
            return "Admin";
        case RoutePurpose::Resource:
            return "Resource";
        case RoutePurpose::SettlementLink:
            return "SettlementLink";
        case RoutePurpose::FutureTrade:
            return "FutureTrade";
        case RoutePurpose::Strategic:
            return "Strategic";
    }
    return "Unknown";
}

float RouteMaintenancePerTile(RouteKind kind) {
    switch (kind) {
        case RouteKind::Trail:
            return 0.012F;
        case RouteKind::Road:
            return 0.035F;
        case RouteKind::RiverRoute:
            return 0.020F;
        case RouteKind::CoastalRoute:
            return 0.025F;
    }
    return 0.0F;
}

}  // namespace oikumene
