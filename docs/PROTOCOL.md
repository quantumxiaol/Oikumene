# Protocol

Protocol version: `0.1`

The protocol is advisory. C++ sends strategic reports and legal candidate actions. Python returns suggested action IDs.

## GET /health

Checks whether PythonAgentServer is reachable.

Response:

```json
{
  "status": "ok",
  "service": "OikumeneAgentServer"
}
```

## POST /api/v1/decisions/batch

Returns suggested actions for one or more polities.

Request shape:

```json
{
  "protocol_version": "0.1",
  "requests": [
    {
      "protocol_version": "0.1",
      "request_id": "turn_10_polity_1",
      "turn": 10,
      "strategic_turn": 2,
      "polity_id": 1,
      "report": {
        "name": "Barkaquin Confederacy",
        "population": 842
      },
      "candidate_actions": [
        {
          "action_id": "research_mining",
          "type": "RESEARCH_TECH",
          "target": "mining",
          "cost": 40,
          "estimated_benefit": 0.8,
          "risk": 0.1
        }
      ]
    }
  ]
}
```

Response shape:

```json
{
  "protocol_version": "0.1",
  "responses": [
    {
      "protocol_version": "0.1",
      "request_id": "turn_10_polity_1",
      "turn": 10,
      "polity_id": 1,
      "selected_actions": [
        {
          "action_id": "research_mining",
          "priority": 0.7,
          "commitment": "medium"
        }
      ],
      "diplomatic_messages": [],
      "rationale_short": "Selected the highest benefit candidate action.",
      "confidence": 0.55
    }
  ]
}
```

## Validation Rules

C++ must reject any response that:

- Has the wrong request ID.
- Targets the wrong turn or polity.
- Selects an action ID not present in `candidate_actions`.
- Selects an action that is no longer legal.
- Is missing required protocol fields.
