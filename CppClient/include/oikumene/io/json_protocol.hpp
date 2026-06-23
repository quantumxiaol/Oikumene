#pragma once

#include <nlohmann/json.hpp>

#include "oikumene/ai/decision_models.hpp"

namespace oikumene {

using Json = nlohmann::json;

inline Json SerializeDecisionRequest(const DecisionRequest& request) {
    return Json(request);
}

inline DecisionResponse ParseDecisionResponse(const Json& value) {
    return value.get<DecisionResponse>();
}

inline Json SerializeDecisionBatchRequest(const DecisionBatchRequest& request) {
    return Json(request);
}

inline DecisionBatchResponse ParseDecisionBatchResponse(const Json& value) {
    return value.get<DecisionBatchResponse>();
}

}  // namespace oikumene
