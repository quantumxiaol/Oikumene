#pragma once

#include "oikumene/ai/decision_provider.hpp"

namespace oikumene {

class HeuristicDecisionProvider final : public DecisionProvider {
public:
    [[nodiscard]] std::string Name() const override;
    [[nodiscard]] HealthStatus CheckHealth() override;
    [[nodiscard]] DecisionBatchResponse DecideBatch(const DecisionBatchRequest& request) override;
};

}  // namespace oikumene
