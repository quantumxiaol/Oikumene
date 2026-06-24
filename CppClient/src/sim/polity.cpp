#include "oikumene/sim/polity.hpp"

namespace oikumene {

std::string ToString(PolityLevel level) {
    switch (level) {
    case PolityLevel::Chiefdom:
        return "Chiefdom";
    case PolityLevel::CityState:
        return "CityState";
    case PolityLevel::Kingdom:
        return "Kingdom";
    }
    return "Unknown";
}

} // namespace oikumene
