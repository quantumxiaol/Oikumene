#include "oikumene/world/route_kind.hpp"

namespace oikumene {

std::string ToString(RouteKind kind) {
    switch (kind) {
        case RouteKind::Trail:
            return "Trail";
        case RouteKind::Road:
            return "Road";
        case RouteKind::RiverRoute:
            return "RiverRoute";
        case RouteKind::CoastalRoute:
            return "CoastalRoute";
    }
    return "Unknown";
}

float RouteTravelMultiplier(RouteKind kind) {
    switch (kind) {
        case RouteKind::Trail:
            return 0.90F;
        case RouteKind::Road:
            return 0.65F;
        case RouteKind::RiverRoute:
            return 0.70F;
        case RouteKind::CoastalRoute:
            return 0.75F;
    }
    return 1.0F;
}

float RouteQuality(RouteKind kind) {
    switch (kind) {
        case RouteKind::Trail:
            return 0.35F;
        case RouteKind::Road:
            return 0.80F;
        case RouteKind::RiverRoute:
            return 0.70F;
        case RouteKind::CoastalRoute:
            return 0.65F;
    }
    return 0.0F;
}

}  // namespace oikumene
