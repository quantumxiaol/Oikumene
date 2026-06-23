#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "oikumene/core/types.hpp"

namespace oikumene {

struct CandidateAction {
    std::string action_id;
    ActionType type = ActionType::Unknown;
    std::string target;
    double cost = 0.0;
    double estimated_benefit = 0.0;
    double risk = 0.0;
};

struct DecisionRequest {
    std::string protocol_version = kProtocolVersion;
    std::string request_id;
    Turn turn = 0;
    Turn strategic_turn = 0;
    PolityId polity_id = kInvalidPolityId;
    nlohmann::json report = nlohmann::json::object();
    std::vector<CandidateAction> candidate_actions;
};

struct SelectedAction {
    std::string action_id;
    double priority = 0.0;
    std::string commitment = "medium";
};

struct DiplomaticMessage {
    PolityId target_polity_id = kInvalidPolityId;
    std::string intent;
    std::string message;
};

struct DecisionResponse {
    std::string protocol_version = kProtocolVersion;
    std::string request_id;
    Turn turn = 0;
    PolityId polity_id = kInvalidPolityId;
    std::vector<SelectedAction> selected_actions;
    std::vector<DiplomaticMessage> diplomatic_messages;
    std::string rationale_short;
    double confidence = 0.0;
};

struct DecisionBatchRequest {
    std::string protocol_version = kProtocolVersion;
    std::vector<DecisionRequest> requests;
};

struct DecisionBatchResponse {
    std::string protocol_version = kProtocolVersion;
    std::vector<DecisionResponse> responses;
};

inline void to_json(nlohmann::json& json, const CandidateAction& action) {
    json = nlohmann::json{
        {"action_id", action.action_id},
        {"type", ToString(action.type)},
        {"target", action.target},
        {"cost", action.cost},
        {"estimated_benefit", action.estimated_benefit},
        {"risk", action.risk},
    };
}

inline void from_json(const nlohmann::json& json, CandidateAction& action) {
    action.action_id = json.value("action_id", "");
    action.type = ActionTypeFromString(json.value("type", "UNKNOWN"));
    action.target = json.value("target", "");
    action.cost = json.value("cost", 0.0);
    action.estimated_benefit = json.value("estimated_benefit", 0.0);
    action.risk = json.value("risk", 0.0);
}

inline void to_json(nlohmann::json& json, const DecisionRequest& request) {
    json = nlohmann::json{
        {"protocol_version", request.protocol_version},
        {"request_id", request.request_id},
        {"turn", request.turn},
        {"strategic_turn", request.strategic_turn},
        {"polity_id", request.polity_id},
        {"report", request.report},
        {"candidate_actions", request.candidate_actions},
    };
}

inline void from_json(const nlohmann::json& json, DecisionRequest& request) {
    request.protocol_version = json.value("protocol_version", std::string(kProtocolVersion));
    request.request_id = json.value("request_id", "");
    request.turn = json.value("turn", 0);
    request.strategic_turn = json.value("strategic_turn", 0);
    request.polity_id = json.value("polity_id", kInvalidPolityId);
    request.report = json.value("report", nlohmann::json::object());
    request.candidate_actions = json.value("candidate_actions", std::vector<CandidateAction>{});
}

inline void to_json(nlohmann::json& json, const SelectedAction& action) {
    json = nlohmann::json{
        {"action_id", action.action_id},
        {"priority", action.priority},
        {"commitment", action.commitment},
    };
}

inline void from_json(const nlohmann::json& json, SelectedAction& action) {
    action.action_id = json.value("action_id", "");
    action.priority = json.value("priority", 0.0);
    action.commitment = json.value("commitment", "medium");
}

inline void to_json(nlohmann::json& json, const DiplomaticMessage& message) {
    json = nlohmann::json{
        {"target_polity_id", message.target_polity_id},
        {"intent", message.intent},
        {"message", message.message},
    };
}

inline void from_json(const nlohmann::json& json, DiplomaticMessage& message) {
    message.target_polity_id = json.value("target_polity_id", kInvalidPolityId);
    message.intent = json.value("intent", "");
    message.message = json.value("message", "");
}

inline void to_json(nlohmann::json& json, const DecisionResponse& response) {
    json = nlohmann::json{
        {"protocol_version", response.protocol_version},
        {"request_id", response.request_id},
        {"turn", response.turn},
        {"polity_id", response.polity_id},
        {"selected_actions", response.selected_actions},
        {"diplomatic_messages", response.diplomatic_messages},
        {"rationale_short", response.rationale_short},
        {"confidence", response.confidence},
    };
}

inline void from_json(const nlohmann::json& json, DecisionResponse& response) {
    response.protocol_version = json.value("protocol_version", std::string(kProtocolVersion));
    response.request_id = json.value("request_id", "");
    response.turn = json.value("turn", 0);
    response.polity_id = json.value("polity_id", kInvalidPolityId);
    response.selected_actions = json.value("selected_actions", std::vector<SelectedAction>{});
    response.diplomatic_messages = json.value("diplomatic_messages", std::vector<DiplomaticMessage>{});
    response.rationale_short = json.value("rationale_short", "");
    response.confidence = json.value("confidence", 0.0);
}

inline void to_json(nlohmann::json& json, const DecisionBatchRequest& request) {
    json = nlohmann::json{
        {"protocol_version", request.protocol_version},
        {"requests", request.requests},
    };
}

inline void from_json(const nlohmann::json& json, DecisionBatchRequest& request) {
    request.protocol_version = json.value("protocol_version", std::string(kProtocolVersion));
    request.requests = json.value("requests", std::vector<DecisionRequest>{});
}

inline void to_json(nlohmann::json& json, const DecisionBatchResponse& response) {
    json = nlohmann::json{
        {"protocol_version", response.protocol_version},
        {"responses", response.responses},
    };
}

inline void from_json(const nlohmann::json& json, DecisionBatchResponse& response) {
    response.protocol_version = json.value("protocol_version", std::string(kProtocolVersion));
    response.responses = json.value("responses", std::vector<DecisionResponse>{});
}

}  // namespace oikumene
