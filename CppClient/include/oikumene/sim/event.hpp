#pragma once

#include <string>

#include "oikumene/core/types.hpp"

namespace oikumene {

enum class EventType {
    BandMigrated,
    SettlementFounded,
    PopulationGrowth,
    Famine,
    SettlementUpgraded,
    TileImproved,
    FarmBuilt,
    LumberCampBuilt,
    PastureBuilt,
    ShallowMineBuilt,
    ForestCleared,
    CarryingCapacityReached,
    PolityFounded,
    SettlementJoinedPolity,
    ControlFieldExpanded,
    ContestedRegionFormed,
    ResearchStarted,
    TechnologyUnlocked,
};

[[nodiscard]] std::string ToString(EventType type);

struct SimEvent {
    Turn turn = 0;
    EventType type = EventType::BandMigrated;
    int actor_id = -1;
    int x = 0;
    int y = 0;
    std::string summary;
};

}  // namespace oikumene
