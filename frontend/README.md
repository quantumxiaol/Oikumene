# Frontend / GitHub Pages

This directory is reserved for a future GitHub Pages replay/dashboard.

The MVP frontend is `CppClient` using Raylib. This web frontend should initially stay replay-oriented:

- Read exported run artifacts such as `world.json`, `states.jsonl`, and `events.jsonl`.
- Show generated worlds, timeline events, polity summaries, and maps in the browser.
- Avoid becoming a second authoritative simulation client.

The C++ app remains the source of truth for simulation and validation.
