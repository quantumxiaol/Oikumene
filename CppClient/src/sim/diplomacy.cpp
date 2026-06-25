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

std::string ToString(DiplomaticIncidentKind kind) {
    switch (kind) {
    case DiplomaticIncidentKind::None:
        return "None";
    case DiplomaticIncidentKind::TerritoryCeded:
        return "TerritoryCeded";
    case DiplomaticIncidentKind::OccupationWithdrawn:
        return "OccupationWithdrawn";
    case DiplomaticIncidentKind::VassalCreated:
        return "VassalCreated";
    case DiplomaticIncidentKind::OccupationRevolt:
        return "OccupationRevolt";
    }
    return "Unknown";
}

} // namespace oikumene
