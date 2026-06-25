#include "oikumene/sim/occupation.hpp"

namespace oikumene {

std::string ToString(OccupationStatus status) {
    switch (status) {
    case OccupationStatus::Active:
        return "Active";
    case OccupationStatus::Ceded:
        return "Ceded";
    case OccupationStatus::Withdrawn:
        return "Withdrawn";
    case OccupationStatus::Vassalized:
        return "Vassalized";
    case OccupationStatus::Revolted:
        return "Revolted";
    }
    return "Unknown";
}

} // namespace oikumene
