# Architecture

Oikumene is built around one hard boundary: C++ owns the world, Python advises.

## Runtime Roles

### C++ Oikumene App

The C++ side is the authoritative simulator.

It owns:

- World state.
- Deterministic simulation rules.
- Map generation.
- Economy, trade, war, control fields, and technology effects.
- Raylib rendering and debug UI.
- Save/load and replay.
- Validation and execution of actions.

In Phase 0, the C++ side is split into:

- `oikumene_core`: simulation and protocol code, independent from Raylib.
- `oikumene_app`: Raylib window and user-facing application shell.

### PythonAgentServer

PythonAgentServer is an advisory service. It can inspect strategic reports and candidate actions, then return suggested actions. It never mutates world state.

It will eventually host:

- Heuristic strategic AI.
- LLM-backed diplomacy and strategy.
- Prompt building.
- Diplomacy memory.
- Decision logging support.

## Decision Boundary

Python and LLMs cannot directly edit the simulation. They can only return action IDs from the candidates supplied by C++.

The C++ side must validate:

- Protocol version.
- Request ID.
- Turn freshness.
- Polity ID.
- Candidate action existence.
- Resource costs.
- Tech prerequisites.
- Diplomatic constraints.
- War ROI and target validity.

If a remote decision is missing, late, malformed, or illegal, C++ falls back to local heuristic AI.

## Communication

Phase 0 uses local HTTP JSON:

```text
C++ RemoteDecisionProvider
        |
        | GET /health
        | POST /api/v1/decisions/batch
        v
PythonAgentServer
```

This is intentionally low-frequency and coarse-grained. Python is for strategic turns and major events, not per-frame or per-villager decisions.

## Frontend

The MVP frontend is `CppClient` using Raylib. The `frontend/` directory is reserved for a future web replay or dashboard and should not become a second authoritative client.
