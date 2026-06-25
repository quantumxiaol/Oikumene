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

## Phase 5.0: Trade Candidates and Agreements

- Resource complementarity.
- Trade agreements between polities.
- Route-network-aware capital-to-capital trade cost.
- Trade profit applied to polity wealth surplus.

## Phase 5.1: Trade Stability and Visualization

- Separate higher opening threshold from softer continuation threshold.
- Keep weak agreements alive for a short grace window before closing them.
- Persist trade paths for JSON export and map overlays.
- Add TradeNetwork map layer and tile-level trade agreement details.

## Phase 5.2: Early Diplomacy Scores

- Build pairwise `DiplomacyRelation` records for every polity pair.
- Convert trade profit, complementarity, route efficiency, asymmetric imports, border tension, and economic overlap into friendship, competition, dependence, and blockade tendency scores.
- Classify relations as Neutral, Friendly, Competitive, Dependent, or BlockadeRisk.
- Export diplomacy metrics to headless JSON/CSV and show relation details in the debug UI.

## Phase 5.3: Diplomacy-Aware War Pressure

- Build directed `WarPressure` candidates from pairwise diplomacy relations.
- Friendly relations increase `friendly_penalty` and reduce declaration pressure.
- Dependence, blockade tendency, route fragility, and competition increase `trade_conflict_weight`.
- Classify early war objectives as BorderDispute, TradeCoercion, Blockade, or DependencyBreakout.
- Export War ROI and declaration pressure metrics to headless JSON/CSV and show top candidates in the debug UI.
- Keep this phase observational: no war declaration, battle resolution, occupation, or boundary mutation yet.

## Phase 5.4: War Target Valuation and Campaign Costs

- Build concrete `WarTargetCandidate` records from diplomacy-aware `WarPressure`.
- Target settlements, resource regions, contested border tiles, trade route nodes, and strategic passes.
- Score farmland, resource, settlement, trade, and strategic value.
- Split campaign cost into mobilization, supply, equipment, terrain attrition, defender power, defense, and occupation maintenance.
- Export target paths, value/cost components, ROI, and action score to headless JSON/CSV and selected-polity UI details.
- Refresh target candidates on a strategic cadence instead of every turn to avoid excessive pathfinding work.
- Keep this phase observational: no war declaration, battle resolution, occupation, resource consumption, or boundary mutation yet.

## Phase 5.5: Abstract War Execution

- Convert high-score `WarTargetCandidate` records into war plans.
- Track war state, mobilized resources, campaign progress, occupation, retreat, and peace events.
- Consume population, food, and equipment from member settlements during active campaigns.
- Mutate settlement polity ownership or target tile control on successful occupation.
- Export `WarCampaign` records and war event/resource metrics to headless JSON/CSV and selected-polity UI details.

## Phase 5.6: Occupation and Settlement Aftermath

- Convert successful `WarCampaign` occupations into persistent `OccupationRecord` records.
- Consume food, wood, and wealth every turn while an occupation is active.
- Track maintenance shortfall, unrest, integration, revolt risk, and border stability pressure.
- Resolve occupations into cession, withdrawal, vassal buffer, or frontier revolt events.
- Feed active occupation load and unrest back into polity administration, stability, and control power.
- Export occupation records, outcome counts, maintenance, and unrest metrics to headless JSON/CSV and selected-polity UI details.
- Calibrate occupation outcome frequency across seeds and map geographies.

## Phase 5.7: Diplomatic Memory and War Outcome Calibration

- Write cession, withdrawal, vassalization, and revolt outcomes back into pairwise `DiplomacyRelation` memory.
- Track long-term grievance, vassalage, and restraint memories with slow decay across diplomacy refreshes.
- Let diplomatic memory influence posture classification, War ROI, declaration pressure, and dependency-breakout conflicts.
- Surface memory terms in the selected-polity UI and export them in headless JSON/CSV metrics.
- Calibrate early war cadence and occupation outcome thresholds so small seed batches produce visible wars, vassals, and revolts without overwhelming polity stability.
- Keep occupation results authoritative in C++; future LLM strategy can only observe this memory and choose from legal actions.

## Phase 5.8: Explicit Vassal Treaties

- Promote vassalization from diplomacy memory into explicit `VassalTreaty` records.
- Track overlord, subject, source occupation, treaty strength, autonomy, tribute rate, protection, loyalty, liberty desire, tribute due, and military obligation.
- Let `OccupationSystem` create or refresh a treaty when an occupation resolves into a vassal buffer.
- Let `VassalSystem` write active treaty state back into polity fields such as overlord, vassal count, subject treaty ids, tribute, protection, and liberty desire.
- Let `DiplomacyRelation` explicitly reference active treaty ids and treaty direction, while still preserving decaying historical memory.
- Let War ROI use treaty direction: subjects with high liberty desire get dependency-breakout pressure, while overlords are restrained from directly attacking their own vassals.
- Export treaty records and treaty aggregate metrics in headless JSON/CSV, and show treaty lines in the selected-polity debug UI.

## Phase 5.9: Compressed Strategic Reports

- Build compact `StrategicReport` records for each polity as the C++ side of the Phase 6 decision input.
- Summarize polity status, stockpiles, budget, governance, technology, geography, trade, diplomacy, war pressure, war targets, wars, occupations, vassal treaties, and recent events.
- Generate legal candidate actions for research, trade, war preparation, and peace offers without mutating the world.
- Wrap strategic reports into the existing `DecisionBatchRequest` protocol shape.
- Export `strategic_reports.json` and `decision_batch.json` from `oikumene_sim_batch` for offline review before connecting Python/LLM decisions.
- Keep the report intentionally compressed: no full map, no full path dump, and no unbounded event history.

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
