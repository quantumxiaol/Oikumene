#pragma once

#include <cstdint>
#include <string>

namespace oikumene {

using Turn = std::int64_t;
using PolityId = std::int32_t;

inline constexpr const char* kProtocolVersion = "0.1";
inline constexpr PolityId kInvalidPolityId = -1;

enum class ActionType {
    ResearchTech,
    OpenTrade,
    PrepareWar,
    DeclareWar,
    OfferPeace,
    Unknown,
};

inline std::string ToString(ActionType type) {
    switch (type) {
    case ActionType::ResearchTech:
        return "RESEARCH_TECH";
    case ActionType::OpenTrade:
        return "OPEN_TRADE";
    case ActionType::PrepareWar:
        return "PREPARE_WAR";
    case ActionType::DeclareWar:
        return "DECLARE_WAR";
    case ActionType::OfferPeace:
        return "OFFER_PEACE";
    case ActionType::Unknown:
    default:
        return "UNKNOWN";
    }
}

inline ActionType ActionTypeFromString(const std::string& value) {
    if (value == "RESEARCH_TECH") {
        return ActionType::ResearchTech;
    }
    if (value == "OPEN_TRADE") {
        return ActionType::OpenTrade;
    }
    if (value == "PREPARE_WAR") {
        return ActionType::PrepareWar;
    }
    if (value == "DECLARE_WAR") {
        return ActionType::DeclareWar;
    }
    if (value == "OFFER_PEACE") {
        return ActionType::OfferPeace;
    }
    return ActionType::Unknown;
}

} // namespace oikumene
