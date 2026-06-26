# Oikumene 架构说明

Oikumene（《人居界》）的核心边界是：C++ 拥有世界真相，Python 只提供战略建议。

这个边界比具体实现更重要。任何 Python/LLM 决策都不能直接修改世界状态，只能从 C++ 生成的合法候选动作中选择；动作必须回到 C++ 校验和执行。

## 总体分层

```text
Oikumene/
├── CppClient/             # C++ 主程序、模拟核心、Raylib/ImGui 前端、headless 工具
├── PythonAgentServer/     # Python FastAPI Agent 服务，Phase 6 起接入远程战略决策
├── shared/                # C++ / Python 共享协议样例和 schema
├── docs/                  # 架构、协议、路线图、图例说明
├── frontend/              # 预留给未来 GitHub Pages / Web 回放器
├── runs/                  # 本地实验输出目录
└── scripts/               # 格式化、第三方源码拉取等脚本
```

运行时关系：

```text
             optional HTTP JSON
CppClient  -------------------->  PythonAgentServer
   |
   | owns authoritative World/Simulation
   v
Raylib + ImGui app, batch tools, exported JSON/JSONL
```

当前 MVP 前端是 `CppClient` 的 Raylib 窗口。`frontend/` 暂时不是权威客户端，未来更适合做只读 Web 回放器或 GitHub Pages 展示页。

## CMake Targets

`CppClient/CMakeLists.txt` 当前按核心库、窗口程序、headless 工具和测试拆分。

### `oikumene_core`

静态库。它是项目的主逻辑层，负责：

- 世界生成。
- `World` / `Simulation` 权威状态。
- band、settlement、polity、technology、route、trade、diplomacy、war、occupation、vassal 等模拟系统。
- 控制力、寻路、路线规划、战争 ROI、贸易评分等纯规则逻辑。
- 决策协议数据结构和远程 AI client 封装。
- headless JSON 导出和压缩版 `StrategicReport`。

约束：

- 不依赖 Raylib。
- 不依赖 Dear ImGui。
- 不拥有窗口、输入、相机或 UI 状态。
- 应该能被测试和 headless 工具直接链接。

### `oikumene_app`

Raylib 窗口程序，链接 `oikumene_core`。

负责：

- 窗口生命周期。
- 用户输入。
- 相机和地图渲染。
- ImGui 面板。
- 图层切换、对象选择、截图和导出触发。

它可以读取 `Simulation` 状态，但不应该绕开核心系统直接改世界规则。

### Headless Tools

这些工具都链接 `oikumene_core`，不需要窗口：

- `oikumene_worldgen_batch`：批量生成世界，输出世界生成报告。
- `oikumene_sim_batch`：跑单个 seed 的完整仿真，输出 `summary.json`、`final_state.json`、`events.jsonl`、`strategic_reports.json`、`decision_batch.json`。
- `oikumene_sim_balance_batch`：跑多个 seed 的平衡性统计，输出 CSV/JSON，用于调参和回归。

工具代码放在 `CppClient/tools/`。工具专用的指标采集和输出可以放在 `tools/` 内部，不要强行塞进 `oikumene_core`，除非 GUI、测试或其他工具也需要复用。

### Tests

测试目标链接 `oikumene_core`，少数渲染数学测试会额外编译对应 app-side 源文件。

当前测试覆盖：

- 世界生成和世界生成平衡性。
- band 迁徙和 settlement 经济。
- polity 形成、控制力、行政经济。
- 科技、科技效果。
- pathfinding、route network、route effects。
- trade、diplomacy、war planner、war target planner、war system。
- occupation、vassal system。
- strategic report 和 decision models。
- app config、input policy、simulation controller、camera、selection、symbol registry。
- `oikumene_sim_batch` 和 `oikumene_sim_balance_batch` smoke tests。

## C++ 目录边界

### `CppClient/include/oikumene/`

公开头文件。这里放其他模块、测试、工具需要包含的稳定接口。

规则：

- 只放真正需要跨模块使用的声明。
- 不把私有 helper 暴露到这里。
- 避免让公开头包含 Raylib/ImGui，除非该接口明确属于 app/render 层。

### `CppClient/src/world/`

世界数据和世界生成。

当前世界生成已经按阶段拆分：

- `world_generator.cpp`：生成管线入口。
- `world_generator_terrain.cpp`：高度、海陆、海岸。
- `world_generator_climate.cpp`：温度、降雨。
- `world_generator_hydrology.cpp`：河流、湖泊、汇流。
- `world_generator_ecology.cpp`：biome、移动成本、定居评分。
- `world_generator_resources.cpp`：资源、土壤质量、森林覆盖。
- `world_generator_helpers.*`：噪声、随机、邻接、水体/矿产判断等内部工具。

这里的代码应该保持确定性：同 seed、同参数必须生成同样世界。

### `CppClient/src/sim/`

模拟规则系统。

主要模块：

- `band_system.*`：早期流动 band、迁徙、定居。
- `settlement_system.*`：settlement 每回合调度。
- `settlement_work.*`：settlement 工作半径、地块访问和共享工具。
- `settlement_improvement.*`：Farm、LumberCamp、Pasture、ShallowMine 等改良建设。
- `settlement_production.*`：产出、承载力、工作地块选择。
- `polity_system.*`：polity 形成、成员关系、行政状态。
- `control_field.*`：控制力扩散、自然边界、争议区。
- `technology_system.*` / `tech_effects.*`：科技研究和数值效果。
- `pathfinding.*`：路径搜索。
- `route_planner.*` / `route_system.*`：路线候选、建设、维护和效果。
- `trade_planner.*` / `trade_system.*`：贸易候选、协议和收益。
- `diplomacy_system.*`：外交关系、姿态、长期记忆。
- `war_planner.*` / `war_target_planner.*` / `war_system.*`：战争压力、具体目标、抽象战争执行。
- `occupation_system.*`：战后占领、割让、撤军、叛乱、附庸化。
- `vassal_system.*`：显式附庸条约和每回合写回。
- `simulation_metrics.*`、`simulation_conflict_metrics.*`、`simulation_polity_metrics.*`：给导出和 batch 使用的聚合指标。

约束：

- `sim` 层可以依赖 `world`。
- `sim` 层不应该依赖 Raylib/ImGui。
- 战争、贸易、外交等系统应该通过 `Simulation` 管线调度，不要互相直接制造隐式副作用。

### `CppClient/src/ai/`

AI 决策边界和战略报告。

当前模块：

- `decision_models.hpp`：C++ 决策协议模型。
- `heuristic_decision_provider.*`：本地启发式 provider。
- `remote_decision_provider.*`：HTTP 远程 provider。
- `strategic_report.cpp`：公开入口。
- `strategic_report_summary.*`：压缩版战略报告 JSON。
- `strategic_report_actions.*`：合法候选动作生成。
- `strategic_report_helpers.*`：关系排序、查找、风险辅助。

原则：

- AI 层负责“选择建议”，不负责直接改世界。
- LLM 后续只能选择 `candidate_actions` 中已有的 action id。
- C++ 必须校验 Python/LLM 返回结果。

### `CppClient/src/io/`

导出、协议 JSON 和调试输出。

当前模块：

- `simulation_json.*`：实体级 JSON 序列化。
- `simulation_snapshot_json.cpp`：summary、final state、state sample、strategic reports 等快照导出。
- `export_service.*`：app 侧截图和报告导出服务。

这里要避免把 UI 逻辑混入核心 JSON。窗口截图可以属于 app/export service，全量仿真状态导出应该保持 headless 可用。

### `CppClient/src/render/`

地图渲染、图层、颜色、相机、选择。

负责：

- Raylib 绘制。
- 颜色映射。
- 图层显示。
- 选择优先级。
- 相机移动和缩放。

不应该放模拟规则。

### `CppClient/src/ui/`

Dear ImGui 面板。

当前已拆分：

- `hud_panel.*`
- `playback_bar.*`
- `details_panel.*`
- `event_log_panel.*`
- `help_panel.*`
- `legend_panel.*`
- `panel_layout.*`
- `view_helpers.*`

UI 面板只读状态或发出明确命令，不应该偷偷推进仿真规则。

### `CppClient/src/app/`

应用壳层。

当前模块：

- `oikumene_app.*`：主 app 编排。
- `input_controller.*`：输入处理。
- `simulation_controller.*`：播放、步进、自动推进。
- `app_config.*` / `command_line.*` / `input_policy.*`：配置、命令行、输入策略。

这里允许依赖 Raylib 和 UI 层，但核心规则仍应留在 `sim`。

## PythonAgentServer 边界

Python 服务当前是 FastAPI advisory service：

- `GET /health`
- `POST /api/v1/decisions/batch`

目录：

```text
PythonAgentServer/src/oikumene_agent/
├── api/       # FastAPI routes
├── agents/    # heuristic / future LLM agents
├── schemas/   # Pydantic request/response schema
└── main.py    # FastAPI app
```

约束：

- Python 不持有权威世界状态。
- Python 不直接修改 `World`、`Simulation` 或存档。
- Python 返回 `DecisionBatchResponse`，C++ 负责校验和执行。
- 如果 Python 服务不可用、超时或返回非法 action，C++ 应回退到本地 heuristic。

## 数据流

### 正常 GUI 模式

```text
OikumeneApp
  -> SimulationController
    -> Simulation::AdvanceOneTurn()
      -> world/sim systems mutate authoritative state
  -> MapRenderer/UI panels read Simulation
```

### Headless batch 模式

```text
oikumene_sim_batch
  -> WorldGenerator::Generate()
  -> Simulation::InitializeBands()
  -> Simulation::AdvanceOneTurn() repeated
  -> io JSON exporters
```

### Phase 6 远程决策模式

```text
C++ builds StrategicReport + CandidateActions
  -> POST /api/v1/decisions/batch
  -> Python selects suggested action ids
  -> C++ validates response
  -> C++ enqueues/applies accepted actions at turn boundary
```

## 回归命令

常规 C++ 回归：

```bash
cd CppClient
python3 ../scripts/format_cpp.py
cmake --build build
ctest --test-dir build --output-on-failure
```

如果还没有配置 build 目录：

```bash
cd CppClient
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

headless 单 seed 回归：

```bash
cd CppClient
./build/oikumene_sim_batch \
  --seed 42 \
  --width 80 \
  --height 56 \
  --turns 180 \
  --bands 8 \
  --out /tmp/oikumene_regression_sim
```

批量平衡回归：

```bash
cd CppClient
./build/oikumene_sim_balance_batch \
  --start-seed 42 \
  --count 6 \
  --width 80 \
  --height 56 \
  --turns 180 \
  --bands 8 \
  --out /tmp/oikumene_regression_balance
```

路线效果对照：

```bash
cd CppClient
./build/oikumene_sim_balance_batch --start-seed 0 --count 20 --width 80 --height 56 --bands 8 --turns 1000 --out ../runs/routes_on_t1000
./build/oikumene_sim_balance_batch --start-seed 0 --count 20 --width 80 --height 56 --bands 8 --turns 1000 --disable-routes --out ../runs/routes_off_t1000
```

Python AgentServer 最小检查：

```bash
cd PythonAgentServer
uv sync
uv run python - <<'PY'
from oikumene_agent.api.routes import health, decide_batch
from oikumene_agent.schemas.decision import CandidateAction, DecisionBatchRequest, DecisionRequest
import asyncio

async def main():
    print(await health())
    request = DecisionBatchRequest(
        requests=[
            DecisionRequest(
                request_id="turn_0_polity_1",
                polity_id=1,
                candidate_actions=[
                    CandidateAction(action_id="research_mining", estimated_benefit=0.8, risk=0.2)
                ],
            )
        ]
    )
    print((await decide_batch(request)).model_dump())

asyncio.run(main())
PY
```

如果要启动服务：

```bash
cd PythonAgentServer
uv run uvicorn oikumene_agent.main:app --reload --port 8000
```

## 新代码放置规则

优先遵守这些规则：

1. 新模拟规则放 `src/sim/`，不要放 UI 或 render。
2. 新地图生成阶段放 `src/world/`，按阶段拆文件。
3. 新 GUI 面板放 `src/ui/`。
4. 新 Raylib 绘制逻辑放 `src/render/`。
5. 新窗口输入和播放控制放 `src/app/`。
6. 新 headless 工具放 `tools/`。
7. 多个工具、GUI、测试都需要的纯逻辑再进入 `oikumene_core`。
8. 只给某个工具用的统计/输出逻辑留在 `tools/`。
9. 私有 helper 优先放在对应 `src/...` 目录，不要暴露到 `include/oikumene/...`。
10. 任何 Python/LLM 决策都必须通过候选 action 和 C++ 校验。

## 当前架构风险

需要持续注意：

- `tools/sim_balance_output.cpp` 和 `tools/sim_balance_metrics.cpp` 仍然接近 500 行，但职责集中在批量平衡工具，可以暂时接受。
- `occupation_system.cpp`、`diplomacy_system.cpp`、`war_system.cpp` 仍有继续拆分空间。
- Python AgentServer 暂时没有正式 pytest 测试；Phase 6 前建议补最小 API 测试，并决定是否加入 `httpx2` 或改用其他测试方式。
- `StrategicReport` 的 JSON shape 已经比较大，后续接 LLM 前需要继续限制字段数量和 recent event 长度。

整体方向是：保持 `oikumene_core` 可 headless、可测试、可复现；保持 app 层薄；保持 Python advisory，而不是第二个模拟器。
