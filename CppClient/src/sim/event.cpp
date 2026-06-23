#include "oikumene/sim/event.hpp"

namespace oikumene {

std::string ToString(EventType type) {
    switch (type) {
        case EventType::BandMigrated:
            return "BandMigrated";
        case EventType::SettlementFounded:
            return "SettlementFounded";
        case EventType::PopulationGrowth:
            return "PopulationGrowth";
        case EventType::Famine:
            return "Famine";
        case EventType::SettlementUpgraded:
            return "SettlementUpgraded";
    }
    return "Unknown";
}

}  // namespace oikumene
