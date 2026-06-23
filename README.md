# Oikumene / 人居界

Oikumene（中文名《人居界》）是一个地理驱动的文明演化沙盒。项目目标是生成一个有高度、气候、水文、生态、资源和交通成本的世界，然后观察部落、定居点、城邦和国家如何在地理约束下扩张、贸易、战争并形成自然边界。

架构边界：

- `CppClient` 是主程序，拥有权威世界状态、模拟规则、数值结算、渲染、存档、回放和动作校验。
- `PythonAgentServer` 是外部 Agent 决策服务，只返回建议动作，不直接修改世界。
- LLM 后续只负责外交、科技、贸易、战争等低频宏观决策；村民行为、生产、寻路、控制力、战争结算等由 C++ 规则系统负责。
- `frontend` 预留给未来 GitHub Pages / Web 回放器；当前 MVP 前端是 `CppClient` 的 Raylib 窗口。

## 当前状态

当前已经进入 Phase 4.0：

- C++ 主程序能打开 Raylib 窗口，生成 80x56 世界地图。
- 支持 Biome、Elevation、Rainfall、Temperature、Fertility、Resources、SettlementScore 图层。
- 已有河流 overlay、资源 marker、band / settlement marker、hover tile inspector。
- `Simulation` 拥有唯一权威 `World`，渲染读取 `simulation.GetWorld()`。
- 已有 band 迁徙、采集、定居、camp 升级 village、事件日志。
- 已有最小村庄经济：村庄会在周围建设 Farm、LumberCamp、Pasture、ShallowMine，并受 carrying capacity 约束。
- 已有早期政治共同体：Village 可形成 Chiefdom，附近聚落会按地形成本加入，首都和成员村庄会向外扩散控制力。
- 已有 Polity / Control 图层，可查看不同 polity 的控制范围、争议区和首都。
- 已有 polity 资源池与行政维护：food/wood/ore/wealth 收入、行政负担、行政容量、overextension、stability 会影响控制力扩散。
- 已有 polity-level 科技系统：Pottery、Irrigation、AnimalHusbandry、Mining、Roads、Administration、BronzeWorking、Fortification、Sailing。
- 科技由 C++ heuristic 选择和推进，暂时不接 LLM；科技效果会影响农田/牧场/矿井产出、行政能力、控制力路径成本和未来军事潜力。
- 已有图例系统：`F2` 打开 Legend 面板，`docs/LEGEND.md` 维护图标和覆盖层说明。
- UI 底部有轻量播放控制条：Play/Pause、Step、+10、+100、TPS 调整、Reset Bands。
- 已有 headless 工具：
  - `oikumene_worldgen_batch`：批量生成世界并输出世界生成报告。
  - `oikumene_sim_batch`：无窗口跑部落/定居仿真并输出 summary、final_state、events。
  - `oikumene_sim_balance_batch`：批量跑多个 seed，输出人口、农田、牧场、伐木、粮食供需、承载力、polity 行政、稳定性和科技指标。

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

- `1`-`8`：切换地图图层：Biome、Elevation、Rainfall、Temperature、Fertility、Resources、SettlementScore、PolityControl。
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
- 鼠标左键：选择 Settlement / Band / Improvement tile / Tile，UI 面板区域不会穿透点击地图。
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

`summary.json` 会包含 camps、villages、active/inactive bands、total population、settlement 平均分、settlement 平均肥沃度、最大 settlement 人口，以及 farm/lumbercamp/pasture/worked tile 数量、上一回合食物/木材产出、食物消耗、平均承载力、polity 数量、controlled land ratio、contested tiles、平均 admin load/capacity、overextension、stability、平均解锁科技数、knowledge income 和关键科技解锁率。`final_state.json` 会保留 Band / Settlement / Polity 的调试字段，并导出 `improved_tiles` 摘要；每个 polity 会包含 `research`、`unlocked_techs`、`active_effects`、`military_potential` 和 `tool_efficiency`。

批量检查村庄经济平衡：

```bash
cd CppClient
./build/oikumene_sim_balance_batch --start-seed 0 --count 20 --width 80 --height 56 --bands 8 --turns 200 --out ../runs/sim_balance_check
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

## 开发格式化

仓库包含：

- `.editorconfig`：统一 UTF-8、LF、缩进和结尾换行。
- `.clang-format`：C++20 代码格式化配置。
- `PythonAgentServer/pyproject.toml` 中的 `ruff` 配置。

C++ 格式化示例：

```bash
clang-format -i CppClient/include/oikumene/**/*.hpp CppClient/src/**/*.cpp CppClient/tests/*.cpp CppClient/tools/*.cpp
```

## 下一阶段

Phase 4 后续重点：

- 继续校准科技成本、研究选择和科技对人口/控制范围的影响。
- 做 Roads / Route Network 的显式路线层，但暂时不做贸易和战争。
- 后续进入 Trade Routes，再进入 Military Potential / War ROI。
