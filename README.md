# Oikumene / 人居界

Oikumene（中文名《人居界》）是一个地理驱动的文明演化沙盒。项目目标是生成一个有高度、气候、水文、生态、资源和交通成本的世界，然后观察部落、定居点、城邦和国家如何在地理约束下扩张、贸易、战争并形成自然边界。

架构边界：

- `CppClient` 是主程序，拥有权威世界状态、模拟规则、数值结算、渲染、存档、回放和动作校验。
- `PythonAgentServer` 是外部 Agent 决策服务，只返回建议动作，不直接修改世界。
- LLM 后续只负责外交、科技、贸易、战争等低频宏观决策；村民行为、生产、寻路、控制力、战争结算等由 C++ 规则系统负责。
- `frontend` 预留给未来 GitHub Pages / Web 回放器；当前 MVP 前端是 `CppClient` 的 Raylib 窗口。

## 当前状态

当前已经进入 Phase 5.7：

- C++ 主程序能打开 Raylib 窗口，生成 80x56 世界地图。
- 支持 Biome、Elevation、Rainfall、Temperature、Fertility、Resources、SettlementScore、PolityControl、RouteNetwork、TradeNetwork 图层。
- 已有河流 overlay、资源 marker、band / settlement marker、hover tile inspector。
- `Simulation` 拥有唯一权威 `World`，渲染读取 `simulation.GetWorld()`。
- 已有 band 迁徙、采集、定居、camp 升级 village、事件日志。
- 已有最小村庄经济：村庄会在周围建设 Farm、LumberCamp、Pasture、ShallowMine，并受 carrying capacity 约束。
- 已有早期政治共同体：Village 可形成 Chiefdom，附近聚落会按地形成本加入，首都和成员村庄会向外扩散控制力。
- 已有 Polity / Control 图层，可查看不同 polity 的控制范围、争议区和首都。
- 已有 polity 资源池与行政维护：food/wood/ore/wealth 收入、行政负担、行政容量、overextension、stability 会影响控制力扩散。
- 已有 polity-level 科技系统：Pottery、Irrigation、AnimalHusbandry、Mining、Roads、Administration、BronzeWorking、Fortification、Sailing。
- 科技由 C++ heuristic 选择和推进，暂时不接 LLM；科技效果会影响农田/牧场/矿井产出、行政能力、控制力路径成本和未来军事潜力。
- 已有显式路线网络：polity 会根据首都-成员村庄、矿点/资源点连接收益自动建设 Trail、Road、RiverRoute、CoastalRoute；路线会降低同 polity 的路径/控制成本，增加矿点转运效率，并产生维护成本。
- `RouteNetwork` 图层可以直接查看路线；选中路线 tile 时详情面板会显示 route id、类型、目的、维护成本、ROI 和建造原因。
- 已有路线效果审计：可以用 `--disable-routes` 关闭路线系统，对照控制范围、行政距离、矿产连接、矿产收入、维护成本和人口规模。
- 已有早期贸易系统：C++ heuristic 会根据 polity 之间的粮食、木材、矿石、财富互补性和首都间路线成本建立 `TradeAgreement`，贸易收益会写入 polity 的 `trade_profit` 和 wealth surplus。
- 已有贸易稳定性校准：贸易开约门槛高于续约门槛，协议会记录 `weak_refresh_count`，连续多轮疲弱才关闭，避免贸易关系因短期预算波动反复开关。
- `TradeNetwork` 图层可以直接查看活跃贸易协议的首都间路径；选中或悬停贸易路径 tile 时，详情面板会显示经过此处的协议 id、双方、货物、利润、路线效率和疲弱次数。
- 已有早期外交关系评分：每对 polity 会生成 `DiplomacyRelation`，根据贸易收益、互补性、依赖方向、边界摩擦、经济重叠和路线脆弱性计算 Friendly、Competitive、Dependent、BlockadeRisk 等姿态。
- 选中 polity 时详情面板会显示相关外交关系；HUD 中的 `Dip F/C/D/B` 分别代表友好、竞争、依赖和封锁风险关系数量。
- 已有早期战争压力候选：`WarPlanner` 会从外交关系生成定向 `WarPressure`，友好关系会提高宣战惩罚，依赖和封锁风险会提高贸易冲突权重，并区分 BorderDispute、TradeCoercion、Blockade、DependencyBreakout 等目标倾向。
- 已有具体战争目标候选：`WarTargetPlanner` 会从 WarPressure 进一步生成 Settlement、ResourceRegion、ContestedBorder、TradeRouteNode、StrategicPass 等目标，并计算目标价值、动员成本、补给成本、装备成本、地形损耗、防御成本、占领维护和 ROI。
- 已有抽象战争执行：`WarSystem` 会把高分 WarTargetCandidate 转成 `WarCampaign`，按回合消耗人口、粮食和装备，推进战役进度，并产生 WarDeclared、WarTargetOccupied、WarRetreated、PeaceSigned 事件。
- 已有战后占领系统：`OccupationSystem` 会把成功占领转成 `OccupationRecord`，持续消耗粮食、木材和财富，积累动荡与整合度，并产生 TerritoryCeded、OccupationWithdrawn、VassalCreated、OccupationRevolt 事件。
- 占领压力会写回 polity 的 `occupation_load`、`occupation_unrest`、`occupied_settlements` 和 `vassal_count`，并在下一轮行政维护、稳定度和控制力扩散中体现；当前战争仍是战略层抽象结算，不做逐格战棋。战争目标每 5 回合刷新一次，避免每回合全量路径搜索拖慢模拟。
- 已有占领结局外交长期记忆：割让、撤军、附庸化和叛乱会写回 `DiplomacyRelation`，形成 `grievance`、`vassalage`、`restraint` 记忆，并随时间衰减；这些记忆会继续影响外交姿态、战争压力和宣战倾向。
- 已有图例系统：`F2` 打开 Legend 面板，`docs/LEGEND.md` 维护图标和覆盖层说明。
- UI 底部有轻量播放控制条：Play/Pause、Step、+10、+100、TPS 调整、Reset Bands。
- 已有 headless 工具：
  - `oikumene_worldgen_batch`：批量生成世界并输出世界生成报告。
  - `oikumene_sim_batch`：无窗口跑部落/定居仿真并输出 summary、final_state、events。
  - `oikumene_sim_balance_batch`：批量跑多个 seed，输出人口、农田、牧场、伐木、粮食供需、承载力、polity 行政、稳定性、科技、路线网络、贸易、外交、外交长期记忆、战争压力、战争目标、战争执行和战后占领指标。

## C++ 依赖

macOS / Homebrew：

```bash
brew install cmake ninja raylib nlohmann-json pkgconf cpp-httplib
```

项目内 vendored 依赖：

- `Dear ImGui`
- `rlImGui`
- `FastNoiseLite`

拉取或更新这些源码：

```bash
python3 scripts/vendor_third_party.py
```

CMake 会自动检测 `CppClient/third_party/`。源码存在时会启用 `oikumene_imgui` 和 `oikumene_fastnoise`；源码不存在时，核心目标仍然可以构建到当前可用程度。

## Python Agent 服务

```bash
cd PythonAgentServer
uv sync
uv run uvicorn oikumene_agent.main:app --reload --port 8000
```

接口：

- `GET /health`
- `POST /api/v1/decisions/batch`

## 编译 C++ 主程序

```bash
cd CppClient
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

运行窗口程序：

```bash
./build/oikumene_app
```

常用参数：

```bash
./build/oikumene_app --seed 42 --width 80 --height 56 --bands 8 --window 1280x720
./build/oikumene_app --seed 42 --bands 8 --auto-run --turns-per-second 4
./build/oikumene_app --seed 42 --bands 8 --disable-routes
```

如果没有安装 Ninja：

```bash
cd CppClient
cmake -S . -B build-make -DCMAKE_BUILD_TYPE=Debug
cmake --build build-make
./build-make/oikumene_app
```

## 配置文件

程序会尝试读取：

```text
CppClient/config/settings.json
```

可以从示例复制：

```bash
mkdir -p CppClient/config
cp CppClient/config/settings.example.json CppClient/config/settings.json
```

命令行参数会覆盖配置文件里的值。

## 窗口快捷键

- `1`-`9`、`0`：切换地图图层：Biome、Elevation、Rainfall、Temperature、Fertility、Resources、SettlementScore、PolityControl、RouteNetwork、TradeNetwork。
- `F2`：显示/隐藏图例面板。
- `R`：使用新 seed 重新生成世界，并重置仿真。
- `B`：在当前世界上重置 band。
- `H`：重新检测 Python Agent 服务是否在线。
- `A`：开启/关闭自动推进。
- `-` / `=`：降低/提高自动推进 TPS。
- `Space`：推进 1 个仿真回合。
- `N`：推进 10 个仿真回合。
- `Shift+N`：推进 100 个仿真回合。
- `Tab`：显示/隐藏详情调试面板，默认隐藏。
- `E`：显示/隐藏 Recent Events 面板，默认隐藏。
- `WASD` / 方向键：平移地图。
- 鼠标右键或中键拖动：拖拽地图。
- 鼠标滚轮：以鼠标指向位置缩放地图。
- 鼠标左键：选择 Settlement / Band / Route tile / Improvement tile / Tile，UI 面板区域不会穿透点击地图。
- `C`：居中到当前选中对象；没有选中对象时适配整张地图。
- `Home` / `F`：适配整张地图到窗口。
- `F1`：显示/隐藏帮助面板。
- `F11`：切换全屏。
- `P`：导出当前窗口截图到 `runs/worldgen_seed_<seed>/layer_<layer>.png`。
- `M`：导出世界生成报告到 `runs/worldgen_seed_<seed>/report.json`。

默认只显示左上角轻量 HUD，避免挡住地图。需要看 tile、band、settlement 的详细解释字段时再按 `Tab`。

## 测试

```bash
cd CppClient
cmake --build build
ctest --test-dir build --output-on-failure
```

当前测试覆盖：

- 决策协议模型。
- 世界生成基础约束。
- 世界生成平衡性。
- app 配置和命令行解析。
- 地图相机数学、缩放稳定性、选择优先级和 UI 点击阻挡。
- band 迁徙、定居、settlement 成长、village 升级、牧场建设、仿真确定性和事件顺序。
- Chiefdom 形成、附近聚落加入、远处聚落不加入、控制力场地形阻隔、河谷扩散、海洋阻断和争议区。
- Polity 资源收入、行政负担、行政容量、overextension、stability，以及图例符号注册完整性。
- Polity 科技研究、前置条件、heuristic 选题、科技效果、确定性，以及 batch 科技字段导出。
- 路径搜索、路线网络建设、道路/小径科技差异、路线确定性、路线对行政距离/控制力/矿点产出的效果。
- 贸易候选评分、贸易协议建立、贸易路径保存、贸易收益写入、贸易弱势续约和连续疲弱关闭。
- 外交关系评分、外交关系对 War ROI / 宣战倾向的修正、具体战争目标候选的价值/成本拆分，以及抽象战争执行。

## 批处理工具

批量检查世界生成：

```bash
cd CppClient
./build/oikumene_worldgen_batch --start-seed 0 --count 100 --out ../runs/worldgen_batch_001
```

输出：

- `summary.csv`
- `summary.json`
- `failed_seeds.txt`

无窗口跑仿真：

```bash
cd CppClient
./build/oikumene_sim_batch --seed 42 --width 80 --height 56 --bands 8 --turns 200 --sample-every 50 --out ../runs/sim_seed42_t200
```

输出：

- `summary.json`
- `final_state.json`
- `events.jsonl`
- `world_report.json`
- `states.jsonl`：仅在传入 `--sample-every N` 时生成。

`summary.json` 会包含 camps、villages、active/inactive bands、total population、settlement 平均分、settlement 平均肥沃度、最大 settlement 人口，以及 farm/lumbercamp/pasture/worked tile 数量、上一回合食物/木材产出、食物消耗、平均承载力、polity 数量、controlled land ratio、contested tiles、平均 admin load/capacity、overextension、stability、平均解锁科技数、knowledge income、关键科技解锁率、路线网络规模、贸易规模、外交关系分布、外交长期记忆、战争压力候选摘要、战争目标价值/成本摘要、战争执行摘要和战后占领摘要。传入 `--disable-routes` 时会完全关闭路线建设、路线 tile 缓存、路线路径加成和矿点转运加成，用来做 routes-on/off 对照。`final_state.json` 会保留 Band / Settlement / Polity / Route / Trade / DiplomacyRelation / WarPressure / WarTargetCandidate / WarCampaign / OccupationRecord 的调试字段，并导出 `improved_tiles` 与 `route_tiles` 摘要；每个 Trade 会导出 `path`、`tile_count` 和 `weak_refresh_count`，方便检查贸易路线与协议稳定性；每个 DiplomacyRelation 会导出 posture、friendship、competition、dependence、blockade_tendency、border_tension、economic_overlap、grievance、vassalage、restraint、last_incident 和 incident_count；每个 WarPressure 会导出 objective、war_roi、declaration_pressure、friendly_penalty、trade_conflict_weight、dependency_pressure、blockade_pressure、grievance_pressure、restraint_pressure 和 vassalage_pressure；每个 WarTargetCandidate 会导出 kind、objective、path、target_value、campaign_cost、occupation_cost、roi、action_score 和各项价值/成本拆分；每个 WarCampaign 会导出 status、progress、mobilized_manpower、population_lost、food_spent、equipment_spent、occupation_profit 和 outcome_reason；每个 OccupationRecord 会导出 status、maintenance_cost、cumulative_maintenance、cumulative_shortfall、unrest、integration、revolt_risk、border_stability_delta 和 outcome_reason；每个 polity 会包含 `research`、`unlocked_techs`、`active_effects`、`military_potential`、`tool_efficiency`、`route_ids`、`route_maintenance`、`connected_settlements`、`connected_mines`、`connected_mine_potential`、`active_connected_mines`、`connected_ore_income`、`unconnected_ore_income`、`trade_ids`、`active_trade_count`、`trade_profit`、`occupation_load`、`occupation_unrest`、`occupied_settlements` 和 `vassal_count`。

批量检查村庄经济、polity、路线、贸易和外交效果：

```bash
cd CppClient
./build/oikumene_sim_balance_batch --start-seed 0 --count 20 --width 80 --height 56 --bands 8 --turns 200 --out ../runs/sim_balance_check
```

路线效果审计建议固定跑一组 routes-on / routes-off：

```bash
cd CppClient
./build/oikumene_sim_balance_batch --start-seed 0 --count 20 --width 80 --height 56 --bands 8 --turns 1000 --out ../runs/routes_on_t1000
./build/oikumene_sim_balance_batch --start-seed 0 --count 20 --width 80 --height 56 --bands 8 --turns 1000 --disable-routes --out ../runs/routes_off_t1000
```

输出：

- `summary.csv`
- `summary.json`

这个工具用于调 `SimulationParams` 和 `settlement_system.cpp` 中的农田增长、承载力、粮食产出、伐木速度、牧场建设等参数。重点看：

- `mean_total_population`：多个 seed 的平均人口。
- `mean_farms` / `mean_lumbercamps` / `mean_pastures`：不同改良是否都能稳定出现。
- `mean_farm_share_of_worked_tiles`：农田是否过快铺满已开发地。
- `mean_food_output_consumption_ratio`：粮食是否过剩或过紧。
- `mean_wood_output`：伐木场是否真的被劳动力使用。
- `mean_famine_events`：是否出现异常饥荒。
- `mean_polities`：早期政治共同体数量。
- `mean_controlled_land_ratio`：陆地被 polity 控制的比例，不能过早接近 100%。
- `mean_contested_tiles`：控制力接近导致的争议区数量。
- `mean_largest_polity_population`：最大政治共同体人口。
- `mean_member_settlements_per_polity`：平均每个 polity 吸纳多少聚落。
- `mean_polity_food_income` / `mean_polity_wood_income` / `mean_polity_wealth_income`：polity 层资源收入。
- `mean_admin_load` / `mean_admin_capacity`：行政负担和行政容量。
- `mean_overextension` / `mean_stability`：治理过载和稳定度。
- `mean_control_maintenance`：控制范围带来的维护成本。
- `mean_unlocked_techs` / `mean_knowledge_income`：科技进展速度。
- `pottery_unlock_rate` / `mining_unlock_rate` / `roads_unlock_rate` / `administration_unlock_rate` 等：科技路线分布。
- `mean_ore_income` / `mean_tool_efficiency` / `mean_military_potential`：矿业、工具和未来军事潜力。
- `mean_routes` / `mean_route_tiles`：平均路线数量和路线 tile 数量。
- `mean_road_tiles` / `mean_trail_tiles` / `mean_river_route_tiles` / `mean_coastal_route_tiles`：不同路线类型的占比，用来判断路线是否过度依赖人造道路或天然廊道。
- `mean_connected_settlements` / `mean_connected_mine_potential`：路线连接了多少成员聚落和潜在矿点目标。
- `mean_active_connected_mines`：已经被村庄实际开采、且被路线连接的浅层矿井数量。
- `mean_connected_ore_income` / `mean_unconnected_ore_income`：已连通和未连通浅层矿井的估算 ore income，用来区分“路线接到了矿点”和“村庄真的在开矿”。
- `mean_admin_distance_cost` / `mean_admin_distance_saving`：平均行政距离成本，以及路线带来的真实路径成本节省。
- `mean_route_maintenance`：路线维护成本。
- `mean_ore_income_for_mining_polities`：已解锁 Mining 的 polity 的平均 ore income，方便判断矿点路线是否有实际收益。
- `mean_active_trades`：平均活跃贸易协议数。
- `mean_trade_profit`：活跃贸易协议的平均净收益。
- `mean_trade_complementarity`：贸易双方的资源互补评分。
- `mean_trade_route_cost` / `mean_trade_route_efficiency`：首都间贸易路径成本和效率，用来判断贸易是否真的受地理/路线网络约束。
- `mean_trade_opened_events`：平均贸易开启事件数，过高通常说明贸易关系过度震荡。
- `mean_trade_weak_refresh_count`：活跃贸易协议平均连续疲弱刷新次数，用来判断续约门槛是否过松或过紧。
- `mean_trade_path_tiles`：活跃贸易协议平均路径长度，用来检查贸易路线是否符合地图尺度。
- `mean_diplomacy_relations`：平均 polity 关系数量。
- `mean_friendly_relations` / `mean_competitive_relations` / `mean_dependent_relations` / `mean_blockade_risk_relations`：不同外交姿态数量。
- `mean_friendship` / `mean_competition` / `mean_blockade_tendency`：外交评分均值，用来校准贸易是否过度导致友好、竞争或封锁风险。
- `mean_diplomatic_grievance` / `mean_diplomatic_vassalage` / `mean_diplomatic_restraint`：战后结局写回外交关系后的平均长期记忆强度，分别表示怨恨、附庸依赖和克制倾向。
- `mean_war_pressure_candidates` / `mean_high_war_pressure_candidates`：平均 War ROI 候选数量和高宣战压力候选数量。
- `mean_war_roi` / `mean_max_declaration_pressure`：平均战争 ROI 和每个 seed 中最高宣战倾向。
- `mean_trade_conflict_weight`：依赖、封锁风险、路线脆弱性和竞争叠加后的贸易冲突权重。
- `mean_friendly_penalty`：友好关系对宣战倾向的平均抑制强度。
- `mean_blockade_pressure` / `mean_dependency_pressure`：封锁和依赖对战争目标倾向的平均贡献。
- `mean_grievance_pressure` / `mean_restraint_pressure` / `mean_vassalage_pressure`：长期怨恨、克制和附庸记忆对战争压力的平均贡献。
- `mean_war_target_candidates` / `mean_high_war_target_candidates`：平均具体战争目标数量和高价值目标数量。
- `mean_war_target_roi` / `mean_max_war_target_score`：具体目标平均 ROI 和每个 seed 中最高目标行动评分。
- `mean_war_target_value`：目标综合价值，来自农田、资源、聚落、贸易路线和战略位置。
- `mean_campaign_cost`：平均战役总成本，包含动员、补给、装备、地形、防御和占领维护。
- `mean_occupation_cost`：平均占领维护成本，用来观察远征、稳定性和友好惩罚是否让占领变得不划算。
- `mean_war_campaigns` / `mean_active_wars`：平均战争计划数量和仍在进行的战争数量。
- `mean_occupied_wars` / `mean_withdrawn_wars` / `mean_peace_wars`：占领、撤退、议和结束的战争数量。
- `mean_occupations` / `mean_active_occupations`：平均战后占领记录数量和仍在维护的占领数量。
- `mean_ceded_occupations` / `mean_withdrawn_occupations` / `mean_vassalized_occupations` / `mean_revolted_occupations`：割让整合、撤军、附庸化、叛乱的占领结局数量。
- `mean_occupation_load` / `mean_occupation_unrest`：占领对 polity 行政和边界稳定的长期压力。
- `mean_active_occupation_unrest` / `mean_active_occupation_maintenance`：仍在维护中的占领平均动荡和维护成本。
- `mean_war_population_lost` / `mean_war_food_spent` / `mean_war_equipment_spent`：战争消耗的人口、粮食和装备。
- `mean_war_declared_events` / `mean_war_occupied_events` / `mean_war_retreat_events` / `mean_peace_events`：战争事件数量。
- `mean_territory_ceded_events` / `mean_occupation_withdrawn_events` / `mean_vassal_created_events` / `mean_occupation_revolt_events`：战后占领结算事件数量。

一次 20 seed、1000 turn 的参考结果：

| 指标 | 路线开启 | 路线关闭 |
| --- | ---: | ---: |
| `mean_total_population` | 2197.60 | 2164.85 |
| `mean_controlled_land_ratio` | 0.637 | 0.588 |
| `mean_admin_distance_cost` | 7.404 | 9.235 |
| `mean_admin_distance_saving` | 7.201 | 0.000 |
| `mean_routes` | 7.55 | 0.00 |
| `mean_route_tiles` | 64.85 | 0.00 |
| `mean_active_connected_mines` | 0.60 | 0.00 |
| `mean_connected_ore_income` | 0.293 | 0.000 |
| `mean_unconnected_ore_income` | 0.018 | 0.240 |
| `mean_route_maintenance` | 0.653 | 0.000 |

一次 6 seed、300 turn 的 Phase 5.1 贸易稳定性 smoke 结果：

| 指标 | 数值 |
| --- | ---: |
| `mean_total_population` | 1822.50 |
| `mean_polities` | 3.50 |
| `mean_routes` | 4.33 |
| `mean_active_trades` | 4.00 |
| `mean_trade_profit` | 2.80 |
| `mean_trade_opened_events` | 6.50 |
| `mean_trade_weak_refresh_count` | 0.03 |
| `mean_trade_path_tiles` | 37.50 |

一次 6 seed、300 turn 的 Phase 5.2 外交评分 smoke 结果：

| 指标 | 数值 |
| --- | ---: |
| `mean_diplomacy_relations` | 4.50 |
| `mean_friendly_relations` | 0.67 |
| `mean_competitive_relations` | 0.83 |
| `mean_dependent_relations` | 2.50 |
| `mean_blockade_risk_relations` | 0.00 |
| `mean_friendship` | 0.60 |
| `mean_competition` | 0.30 |
| `mean_blockade_tendency` | 0.29 |

一次 6 seed、300 turn 的 Phase 5.3 战争压力 smoke 结果：

| 指标 | 数值 |
| --- | ---: |
| `mean_war_pressure_candidates` | 9.00 |
| `mean_high_war_pressure_candidates` | 1.33 |
| `mean_war_roi` | 0.23 |
| `mean_max_declaration_pressure` | 0.73 |
| `mean_trade_conflict_weight` | 0.26 |
| `mean_friendly_penalty` | 0.52 |
| `mean_blockade_pressure` | 0.25 |
| `mean_dependency_pressure` | 0.41 |

一次 6 seed、300 turn 的 Phase 5.4 战争目标 smoke 结果：

| 指标 | 数值 |
| --- | ---: |
| `mean_war_target_candidates` | 182.17 |
| `mean_high_war_target_candidates` | 2.50 |
| `mean_war_target_roi` | 0.19 |
| `mean_max_war_target_score` | 0.79 |
| `mean_war_target_value` | 0.51 |
| `mean_campaign_cost` | 2.50 |
| `mean_occupation_cost` | 0.69 |

一次 6 seed、300 turn 的 Phase 5.5 抽象战争执行 smoke 结果：

| 指标 | 数值 |
| --- | ---: |
| `mean_war_campaigns` | 0.50 |
| `mean_active_wars` | 0.17 |
| `mean_occupied_wars` | 0.33 |
| `mean_withdrawn_wars` | 0.00 |
| `mean_peace_wars` | 0.00 |
| `mean_war_population_lost` | 4.33 |
| `mean_war_food_spent` | 3.13 |
| `mean_war_equipment_spent` | 0.53 |
| `mean_war_declared_events` | 0.50 |
| `mean_war_occupied_events` | 0.33 |

一次 6 seed、300 turn 的 Phase 5.6 战后占领 smoke 结果：

| 指标 | 数值 |
| --- | ---: |
| `mean_war_campaigns` | 0.50 |
| `mean_occupied_wars` | 0.50 |
| `mean_occupations` | 0.50 |
| `mean_active_occupations` | 0.33 |
| `mean_ceded_occupations` | 0.00 |
| `mean_withdrawn_occupations` | 0.00 |
| `mean_vassalized_occupations` | 0.17 |
| `mean_revolted_occupations` | 0.00 |
| `mean_occupation_load` | 0.12 |
| `mean_occupation_unrest` | 0.05 |
| `mean_active_occupation_unrest` | 0.12 |
| `mean_active_occupation_maintenance` | 0.31 |
| `mean_vassal_created_events` | 0.17 |

一次 6 seed、300 turn 的 Phase 5.7 外交长期记忆与战争频率校准 smoke 结果：

| 指标 | 数值 |
| --- | ---: |
| `mean_war_campaigns` | 1.83 |
| `mean_active_wars` | 0.17 |
| `mean_occupied_wars` | 1.67 |
| `mean_occupations` | 1.67 |
| `mean_active_occupations` | 0.50 |
| `mean_vassalized_occupations` | 1.00 |
| `mean_revolted_occupations` | 0.17 |
| `mean_diplomatic_grievance` | 0.052 |
| `mean_diplomatic_vassalage` | 0.124 |
| `mean_diplomatic_restraint` | 0.052 |
| `mean_grievance_pressure` | 0.038 |
| `mean_restraint_pressure` | 0.048 |
| `mean_vassalage_pressure` | 0.071 |
| `mean_max_declaration_pressure` | 0.586 |
| `mean_high_war_target_candidates` | 2.17 |
| `mean_stability` | 0.898 |
| `mean_overextension` | 0.111 |

## 开发格式化

仓库包含：

- `.editorconfig`：统一 UTF-8、LF、缩进和结尾换行。
- `.clang-format`：C++20 代码格式化配置。
- `PythonAgentServer/pyproject.toml` 中的 `ruff` 配置。

C++ 格式化示例：

```bash
python3 scripts/format_cpp.py
```

脚本会在缺少 `clang-format` 时直接提示 `brew install clang-format`，不会让本地验证失败。

## 下一阶段

Phase 5.8 / Phase 6 前置重点：

- 把附庸从当前的长期外交记忆进一步升级为显式条约/宗主关系对象，方便后续外交和 LLM 读取。
- 在更大的 seed 集合上继续校准割让、撤军、附庸和叛乱频率，避免单一结局过度占优。
- 增强边境稳定观测：对比占领前后 contested tiles、control strength、polity stability 和外交记忆变化。
- 为 Phase 6 的 Python/LLM 宏观决策准备更紧凑的国家战略报告。
