#include "oikumene/sim/war_campaign.hpp"

namespace oikumene {

std::string ToString(WarCampaignStatus status) {
    switch (status) {
    case WarCampaignStatus::Active:
        return "Active";
    case WarCampaignStatus::Occupied:
        return "Occupied";
    case WarCampaignStatus::Withdrawn:
        return "Withdrawn";
    case WarCampaignStatus::Peace:
        return "Peace";
    }
    return "Unknown";
}

} // namespace oikumene
