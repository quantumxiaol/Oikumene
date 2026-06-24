#include "oikumene/sim/diplomacy.hpp"

namespace oikumene {

std::string ToString(DiplomaticPosture posture) {
    switch (posture) {
    case DiplomaticPosture::Neutral:
        return "Neutral";
    case DiplomaticPosture::Friendly:
        return "Friendly";
    case DiplomaticPosture::Competitive:
        return "Competitive";
    case DiplomaticPosture::Dependent:
        return "Dependent";
    case DiplomaticPosture::BlockadeRisk:
        return "BlockadeRisk";
    }
    return "Unknown";
}

} // namespace oikumene
