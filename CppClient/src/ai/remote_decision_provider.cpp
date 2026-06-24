#include "oikumene/ai/remote_decision_provider.hpp"

#include <nlohmann/json.hpp>

#include "oikumene/io/json_protocol.hpp"
#include "oikumene/network/http_client.hpp"

namespace oikumene {

RemoteDecisionProvider::RemoteDecisionProvider(std::string host, int port, std::chrono::milliseconds timeout)
    : host_(std::move(host)), port_(port), timeout_(timeout) {}

std::string RemoteDecisionProvider::Name() const {
    return "remote-http";
}

HealthStatus RemoteDecisionProvider::CheckHealth() {
    const auto response = http::Get(host_, port_, "/health", timeout_);

    HealthStatus status;
    status.online = response.status_code == 200;
    status.status_code = response.status_code;
    status.latency_ms = response.latency_ms;

    if (!response.error.empty()) {
        status.message = response.error;
        return status;
    }

    try {
        const auto body = nlohmann::json::parse(response.body);
        status.service = body.value("service", "");
        status.message = body.value("status", "");
        status.online = status.online && status.message == "ok";
    } catch (const std::exception& error) {
        status.online = false;
        status.message = error.what();
    }

    return status;
}

DecisionBatchResponse RemoteDecisionProvider::DecideBatch(const DecisionBatchRequest& request) {
    const auto body = SerializeDecisionBatchRequest(request).dump();
    const auto response = http::PostJson(host_, port_, "/api/v1/decisions/batch", body, timeout_);

    if (response.status_code != 200 || !response.error.empty()) {
        DecisionBatchResponse fallback;
        fallback.protocol_version = request.protocol_version;
        return fallback;
    }

    try {
        return ParseDecisionBatchResponse(nlohmann::json::parse(response.body));
    } catch (const std::exception&) {
        DecisionBatchResponse fallback;
        fallback.protocol_version = request.protocol_version;
        return fallback;
    }
}

} // namespace oikumene
