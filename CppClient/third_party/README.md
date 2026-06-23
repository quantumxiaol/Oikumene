# Third Party Dependencies

This directory is for small vendored C/C++ libraries that should build consistently across macOS, Windows, and Linux.

## Policy

- Commit small source-only libraries here when they are required for normal builds.
- Keep each library at a known upstream ref.
- Keep upstream license files.
- Use `scripts/vendor_third_party.py` to refresh or bootstrap the directory.
- Do not make normal CMake configure/build depend on network access.

## Planned Vendors

| Directory | Upstream | Use |
| --- | --- | --- |
| `imgui/` | `ocornut/imgui` | Dear ImGui debug UI |
| `rlImGui/` | `raylib-extras/rlImGui` | Dear ImGui bridge for raylib |
| `FastNoiseLite/` | `Auburn/FastNoiseLite` | Noise generation for worldgen |

## Bootstrap

From the repository root:

```bash
python3 scripts/vendor_third_party.py
```

The script removes and recreates the three vendor directories. Review the diff and license files before committing updates.

## CMake Integration

`CppClient/cmake/ThirdParty.cmake` detects these directories:

- If `imgui/` and `rlImGui/` are present, it creates `oikumene_imgui`.
- If `FastNoiseLite/Cpp/FastNoiseLite.h` is present, it creates `oikumene_fastnoise`.

If the directories are absent, CMake prints a status message and the Phase 0 app still builds.
