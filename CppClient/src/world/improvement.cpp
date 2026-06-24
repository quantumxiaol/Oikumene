#include "oikumene/world/improvement.hpp"

namespace oikumene {

std::string ToString(ImprovementKind improvement) {
    switch (improvement) {
    case ImprovementKind::None:
        return "None";
    case ImprovementKind::ForagingGround:
        return "ForagingGround";
    case ImprovementKind::Farm:
        return "Farm";
    case ImprovementKind::LumberCamp:
        return "LumberCamp";
    case ImprovementKind::Pasture:
        return "Pasture";
    case ImprovementKind::ShallowMine:
        return "ShallowMine";
    case ImprovementKind::Road:
        return "Road";
    }
    return "Unknown";
}

} // namespace oikumene
