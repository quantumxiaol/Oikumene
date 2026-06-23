# Oikumene / 人居界

Oikumene（中文名《人居界》）是一个地理驱动的文明演化沙盒。项目目标是生成一个有高度、气候、水文、生态、资源和交通成本的世界，然后观察部落、定居点、城邦和国家如何在地理约束下扩张、贸易、战争并形成自然边界。

项目架构的硬边界：

- `CppClient` 是主程序，拥有权威世界状态、模拟规则、数值结算、渲染、存档、回放和动作校验。
- `PythonAgentServer` 是外部 Agent 决策服务，只返回建议动作，不直接修改世界。
- LLM 以后只负责外交、科技、贸易、战争等低频宏观决策；村民行为、生产、寻路、控制力、战争结算等由 C++ 规则系统负责。

## 当前阶段

当前仓库是 Phase 0 工程骨架：

- `CppClient`：C++20 核心库 + Raylib 窗口应用。
- `PythonAgentServer`：FastAPI 决策服务。
- `shared`：C++ / Python 共用协议样例和 JSON Schema。
- `docs`：架构、协议和路线图文档。
- `frontend`：预留给未来 Web 回放器或数据看板；MVP 前端是 `CppClient`。

## C++ 依赖

macOS / Homebrew：

```bash
brew install cmake ninja raylib nlohmann-json pkgconf cpp-httplib
```

当前策略：

- `cmake`、`ninja`、`raylib`、`nlohmann-json`、`pkgconf`、`cpp-httplib` 用系统包管理器安装。
- `Dear ImGui`、`rlImGui`、`FastNoiseLite` 后续放进 `CppClient/third_party/`，固定版本并保留 LICENSE。
- `third_party` 里的小型源码建议提交到 GitHub，保证离线和跨平台构建稳定。
- 后续可以再补一个下载/更新脚本，用来从固定 tag 重新拉取这些第三方源码，但正常构建不应依赖联网下载。

拉取/更新 vendored 依赖：

```bash
python3 scripts/vendor_third_party.py
```

CMake 会自动检测这些源码是否存在。未下载时，Phase 0 仍可正常编译；下载后会启用：

- `oikumene_imgui`：Dear ImGui + rlImGui。
- `oikumene_fastnoise`：FastNoiseLite。

## 启动 Python Agent 服务

```bash
cd PythonAgentServer
uv sync
uv run uvicorn oikumene_agent.main:app --reload --port 8000
```

服务接口：

- `GET /health`
- `POST /api/v1/decisions/batch`

## 编译 C++ 主程序

```bash
cd CppClient
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/oikumene_app
```

如果没有安装 Ninja，也可以用 CMake 默认生成器：

```bash
cd CppClient
cmake -S . -B build-make -DCMAKE_BUILD_TYPE=Debug
cmake --build build-make
./build-make/oikumene_app
```

当前窗口内快捷键：

- `1`-`7`：切换地图图层：Biome、Elevation、Rainfall、Temperature、Fertility、Resources、SettlementScore。
- `R`：使用新 seed 重新生成世界。
- `H`：重新检测 Python Agent 服务是否在线。
- `Tab`：显示/隐藏调试面板。
- `F1`：显示/隐藏帮助面板。
- `P`：导出当前窗口截图到 `runs/worldgen_seed_<seed>/layer_<layer>.png`。
- `M`：导出世界生成报告到 `runs/worldgen_seed_<seed>/report.json`。
- `W/A/S/D`：平移地图。
- 鼠标滚轮：缩放地图。
- 鼠标 hover：查看 tile 信息。
- `Space`：推进占位模拟回合。

## 测试

```bash
cd CppClient
cmake --build build
ctest --test-dir build --output-on-failure
```

如果使用默认生成器：

```bash
cd CppClient
cmake --build build-make
ctest --test-dir build-make --output-on-failure
```

## 开发格式化

仓库包含：

- `.editorconfig`：统一 UTF-8、LF、缩进和结尾换行。
- `.clang-format`：C++20 代码格式化配置。
- `PythonAgentServer/pyproject.toml` 中的 `ruff` 配置：Python lint/format 规则。

C++ 格式化示例：

```bash
clang-format -i CppClient/include/oikumene/**/*.hpp CppClient/src/**/*.cpp CppClient/tests/*.cpp
```

## 下一阶段

Phase 1 会实现 C++ 世界生成和基础地图渲染：

- `Tile` / `World` 数据结构。
- 高度、海洋、湖泊、河流、温度、降雨、肥沃度、生物群落和资源分布。
- Raylib 地图图层显示。
- 基础相机、鼠标 hover tile 信息、seed 重新生成。
