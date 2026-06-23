#pragma once

#include <string>

namespace oikumene {

enum class RouteKind {
    Trail,
    Road,
    RiverRoute,
    CoastalRoute,
};

[[nodiscard]] std::string ToString(RouteKind kind);
[[nodiscard]] float RouteTravelMultiplier(RouteKind kind);
[[nodiscard]] float RouteQuality(RouteKind kind);

}  // namespace oikumene
