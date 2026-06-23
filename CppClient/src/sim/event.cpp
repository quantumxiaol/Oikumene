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
        case EventType::TileImproved:
            return "TileImproved";
        case EventType::FarmBuilt:
            return "FarmBuilt";
        case EventType::LumberCampBuilt:
            return "LumberCampBuilt";
        case EventType::PastureBuilt:
            return "PastureBuilt";
        case EventType::ShallowMineBuilt:
            return "ShallowMineBuilt";
        case EventType::ForestCleared:
            return "ForestCleared";
        case EventType::CarryingCapacityReached:
            return "CarryingCapacityReached";
    }
    return "Unknown";
}

}  // namespace oikumene
