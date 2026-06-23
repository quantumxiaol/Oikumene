# Oikumene / 人居界

Oikumene（中文名《人居界》）是一个地理驱动的文明演化沙盒。项目目标是生成一个有高度、气候、水文、生态、资源和交通成本的世界，然后观察部落、定居点、城邦和国家如何在地理约束下扩张、贸易、战争并形成自然边界。

架构边界：

- `CppClient` 是主程序，拥有权威世界状态、模拟规则、数值结算、渲染、存档、回放和动作校验。
- `PythonAgentServer` 是外部 Agent 决策服务，只返回建议动作，不直接修改世界。
- LLM 后续只负责外交、科技、贸易、战争等低频宏观决策；村民行为、生产、寻路、控制力、战争结算等由 C++ 规则系统负责。
- `frontend` 预留给未来 GitHub Pages / Web 回放器；当前 MVP 前端是 `CppClient` 的 Raylib 窗口。

## 当前状态

当前已经进入 Phase 2 入口：

- C++ 主程序能打开 Raylib 窗口，生成 80x56 世界地图。
- 支持 Biome、Elevation、Rainfall、Temperature、Fertility、Resources、SettlementScore 图层。
- 已有河流 overlay、资源 marker、band / settlement marker、hover tile inspector。
- `Simulation` 拥有唯一权威 `World`，渲染读取 `simulation.GetWorld()`。
- 已有 band 迁徙、采集、定居、camp 升级 village、事件日志。
- 已有最小村庄经济：村庄会在周围建设 Farm、LumberCamp、Pasture、ShallowMine，并受 carrying capacity 约束。
- UI 底部有轻量播放控制条：Play/Pause、Step、+10、+100、TPS 调整、Reset Bands。
- 已有 headless 工具：
  - `oikumene_worldgen_batch`：批量生成世界并输出世界生成报告。
  - `oikumene_sim_batch`：无窗口跑部落/定居仿真并输出 summary、final_state、events。
  - `oikumene_sim_balance_batch`：批量跑多个 seed，输出人口、农田、牧场、伐木、粮食供需和承载力指标。

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

- `1`-`7`：切换地图图层：Biome、Elevation、Rainfall、Temperature、Fertility、Resources、SettlementScore。
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
- `C`：重新居中地图。
- `F1`：显示/隐藏帮助面板。
- `F11`：切换全屏。
- `P`：导出当前窗口截图到 `runs/worldgen_seed_<seed>/layer_<layer>.png`。
- `M`：导出世界生成报告到 `runs/worldgen_seed_<seed>/report.json`。
- 方向键：平移地图。
- 鼠标滚轮：缩放地图。
- 鼠标 hover：查看 tile 信息。
- 鼠标左键：选择当前位置的 band 或 settlement。

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
- band 迁徙、定居、settlement 成长、village 升级、牧场建设、仿真确定性和事件顺序。

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

`summary.json` 会包含 camps、villages、active/inactive bands、total population、settlement 平均分、settlement 平均肥沃度、最大 settlement 人口，以及 farm/lumbercamp/pasture/worked tile 数量、上一回合食物/木材产出、食物消耗和平均承载力。`final_state.json` 会保留 Band / Settlement 的调试字段，并导出 `improved_tiles` 摘要。

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

Phase 2 后续重点：

- 让 settlement 生产模型更明确：粮食、木材、浅层矿产、财富。
- 增加村庄经济面板和曲线。
- 让 band / settlement 事件在 UI 中可筛选。
- 批量跑多个 seed，观察定居速度、Village 升级速度和饥荒频率。
- 继续平衡 Farm/LumberCamp/Pasture 的建设速度、人口增长速度和 carrying capacity。
- 进入 Phase 3：polity、首都、控制力场和自然边界。
