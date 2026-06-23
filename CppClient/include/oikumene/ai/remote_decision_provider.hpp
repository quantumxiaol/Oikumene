#pragma once

#include <chrono>
#include <string>

#include "oikumene/ai/decision_provider.hpp"

namespace oikumene {

class RemoteDecisionProvider final : public DecisionProvider {
public:
    RemoteDecisionProvider(std::string host = "127.0.0.1",
                           int port = 8000,
                           std::chrono::milliseconds timeout = std::chrono::milliseconds(1200));

    [[nodiscard]] std::string Name() const override;
    [[nodiscard]] HealthStatus CheckHealth() override;
    [[nodiscard]] DecisionBatchResponse DecideBatch(const DecisionBatchRequest& request) override;

private:
    std::string host_;
    int port_ = 8000;
    std::chrono::milliseconds timeout_;
};

}  // namespace oikumene
