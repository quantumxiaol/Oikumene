# Oikumene Agent Server

PythonAgentServer is an advisory AI service for Oikumene. It does not own or mutate world state. The C++ simulator sends strategic reports and candidate actions; this service returns suggested actions.

## Run

```bash
uv sync
uv run uvicorn oikumene_agent.main:app --reload --port 8000
```

## Endpoints

- `GET /health`
- `POST /api/v1/decisions/batch`
