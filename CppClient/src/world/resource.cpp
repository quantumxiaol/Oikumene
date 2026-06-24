#include "oikumene/world/resource.hpp"

namespace oikumene {

std::string ToString(ResourceKind resource) {
    switch (resource) {
    case ResourceKind::None:
        return "None";
    case ResourceKind::Wood:
        return "Wood";
    case ResourceKind::Bamboo:
        return "Bamboo";
    case ResourceKind::Horse:
        return "Horse";
    case ResourceKind::Copper:
        return "Copper";
    case ResourceKind::Tin:
        return "Tin";
    case ResourceKind::ShallowIron:
        return "ShallowIron";
    case ResourceKind::ShallowCoal:
        return "ShallowCoal";
    case ResourceKind::MeteoricIron:
        return "MeteoricIron";
    case ResourceKind::Gold:
        return "Gold";
    case ResourceKind::Silver:
        return "Silver";
    case ResourceKind::Stone:
        return "Stone";
    case ResourceKind::Clay:
        return "Clay";
    case ResourceKind::Salt:
        return "Salt";
    }
    return "Unknown";
}

} // namespace oikumene
