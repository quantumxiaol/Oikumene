#pragma once

#include <string>

#include "oikumene/ai/decision_models.hpp"

namespace oikumene {

struct HealthStatus {
    bool online = false;
    int status_code = 0;
    std::string service;
    std::string message;
    long latency_ms = 0;
};

class DecisionProvider {
  public:
    virtual ~DecisionProvider() = default;

    [[nodiscard]] virtual std::string Name() const = 0;
    [[nodiscard]] virtual HealthStatus CheckHealth() = 0;
    [[nodiscard]] virtual DecisionBatchResponse DecideBatch(const DecisionBatchRequest& request) = 0;
};

} // namespace oikumene
