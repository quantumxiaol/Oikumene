#!/usr/bin/env python3
"""Format Oikumene C++ sources when clang-format is available."""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOTS = [
    ROOT / "CppClient" / "include",
    ROOT / "CppClient" / "src",
    ROOT / "CppClient" / "tests",
    ROOT / "CppClient" / "tools",
]
SUFFIXES = {".hpp", ".cpp"}


def iter_sources() -> list[Path]:
    paths: list[Path] = []
    for root in SOURCE_ROOTS:
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if path.is_file() and path.suffix in SUFFIXES:
                paths.append(path)
    return sorted(paths)


def main() -> int:
    clang_format = shutil.which("clang-format")
    if clang_format is None:
        print("clang-format not found; install with: brew install clang-format")
        return 0

    sources = iter_sources()
    if not sources:
        print("no C++ sources found")
        return 0

    command = [clang_format, "-i", *[str(path) for path in sources]]
    subprocess.run(command, check=True)
    print(f"formatted {len(sources)} C++ files")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        print(f"clang-format failed with exit code {error.returncode}", file=sys.stderr)
        raise SystemExit(error.returncode)
