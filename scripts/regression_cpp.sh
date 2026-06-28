#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CPP_DIR="$ROOT/CppClient"
BUILD_DIR="${BUILD_DIR:-$CPP_DIR/build}"
BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}"
GENERATOR="${CMAKE_GENERATOR:-Ninja}"

SEED="${REGRESSION_SEED:-42}"
START_SEED="${REGRESSION_START_SEED:-42}"
COUNT="${REGRESSION_COUNT:-6}"
WIDTH="${REGRESSION_WIDTH:-80}"
HEIGHT="${REGRESSION_HEIGHT:-56}"
BANDS="${REGRESSION_BANDS:-8}"
TURNS="${REGRESSION_TURNS:-180}"
OUT_ROOT="${REGRESSION_OUT_ROOT:-/tmp/oikumene_regression}"

echo "[regression] root: $ROOT"

if [[ "${SKIP_FORMAT:-0}" != "1" ]]; then
    echo "[regression] formatting C++"
    python3 "$ROOT/scripts/format_cpp.py"
fi

if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
    echo "[regression] configuring CMake: $BUILD_DIR"
    cmake -S "$CPP_DIR" -B "$BUILD_DIR" -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
fi

echo "[regression] building"
cmake --build "$BUILD_DIR"

echo "[regression] running CTest"
ctest --test-dir "$BUILD_DIR" --output-on-failure

SIM_OUT="$OUT_ROOT/sim_seed${SEED}_t${TURNS}"
BALANCE_OUT="$OUT_ROOT/balance_seed${START_SEED}_count${COUNT}_t${TURNS}"

echo "[regression] running headless simulation: $SIM_OUT"
"$BUILD_DIR/oikumene_sim_batch" \
    --seed "$SEED" \
    --width "$WIDTH" \
    --height "$HEIGHT" \
    --turns "$TURNS" \
    --bands "$BANDS" \
    --out "$SIM_OUT"

echo "[regression] running balance batch: $BALANCE_OUT"
"$BUILD_DIR/oikumene_sim_balance_batch" \
    --start-seed "$START_SEED" \
    --count "$COUNT" \
    --width "$WIDTH" \
    --height "$HEIGHT" \
    --turns "$TURNS" \
    --bands "$BANDS" \
    --out "$BALANCE_OUT"

echo "[regression] complete"
