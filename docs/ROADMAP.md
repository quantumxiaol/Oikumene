# Roadmap

## Phase 0: Engineering Skeleton

- C++ `oikumene_core` and `oikumene_app` targets.
- Raylib window.
- PythonAgentServer FastAPI service.
- `/health` and batch decision protocol.
- Shared protocol examples.

## Phase 1: World Generation

- Height, ocean, lake, coast, river, temperature, rainfall, fertility, biome, resources.
- Seed-stable world generation.
- Raylib map layer rendering.

## Phase 1.5: Worldgen Audit And Map Semantics

- River semantics fixed as an overlay (`has_river`, `river_flow`) instead of replacing primary biome.
- Simplified flow accumulation for river generation.
- World generation report with biome/resource counts, ratios, means, and top settlement candidates.
- Resource layer renders dimmed terrain plus resource markers.
- Screenshot and report export under `runs/worldgen_seed_<seed>/`.

## Phase 2: Bands and Settlements

- Hunting/gathering bands.
- Settlement scoring.
- Founding villages near water, fertility, forest, grassland, and shallow resources.
- Band migration, camp founding, food/wood stockpiles, population growth, and village upgrade.

## Phase 3: Village Economy and Technology

- Food, wood, ore, metal, wealth.
- Population growth and famine.
- Data-driven technology effects.

## Phase 4: Polities and Control Fields

- Capitals, cities, control diffusion, contested land, wilderness, natural borders.
- Polity resource pools, administration, overextension, stability, and polity-level technology.

## Phase 4.5: Explicit Route Network

- Pathfinding-aware Trail, Road, RiverRoute, and CoastalRoute infrastructure.
- Route ROI scoring for capital-member links and resource/mine access.
- Route maintenance, connected settlement/mine stats, route event logs, and RouteNetwork map layer.
- Route effects on control/path cost, administrative distance, and shallow mine transfer.

## Phase 4.6: Route Effect Audit

- `--disable-routes` switch for the app and headless simulation tools.
- Routes-on/routes-off batch comparison for population, controlled land, administrative distance, mine connection, ore income, and route maintenance.
- Split route audit statistics from route construction code.
- Keep route infrastructure distinct from future trade routes.

## Phase 5: Trade and War ROI

- Resource complementarity.
- Trade routes.
- War target valuation.
- Campaign and occupation costs.

## Phase 6: Remote Strategic AI

- Async C++ decision orchestration.
- Python heuristic provider.
- Timeout and fallback behavior.

## Phase 7: LLM Strategy Layer

- Structured outputs.
- Diplomacy messages.
- Strategic memory.
- Personality profiles.

## Phase 8: Replay and Analysis

- Headless runs.
- Event logs.
- State hashes.
- Seed batch testing.
