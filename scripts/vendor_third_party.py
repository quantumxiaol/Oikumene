#!/usr/bin/env python3
"""Download small vendored C/C++ dependencies for Oikumene.

This script intentionally uses only the Python standard library so it can run
before the PythonAgentServer virtual environment exists.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import sys
import tempfile
import urllib.request
import zipfile
from dataclasses import dataclass
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
THIRD_PARTY_DIR = REPO_ROOT / "CppClient" / "third_party"


@dataclass(frozen=True)
class Vendor:
    name: str
    repo: str
    ref_kind: str
    ref: str
    directory: str
    license_candidates: tuple[str, ...]

    @property
    def archive_url(self) -> str:
        if self.ref_kind == "tag":
            return f"https://github.com/{self.repo}/archive/refs/tags/{self.ref}.zip"
        if self.ref_kind == "branch":
            return f"https://github.com/{self.repo}/archive/refs/heads/{self.ref}.zip"
        raise ValueError(f"Unsupported ref kind for {self.name}: {self.ref_kind}")


VENDORS: tuple[Vendor, ...] = (
    Vendor(
        name="Dear ImGui",
        repo="ocornut/imgui",
        ref_kind="branch",
        ref="master",
        directory="imgui",
        license_candidates=("LICENSE.txt",),
    ),
    Vendor(
        name="rlImGui",
        repo="raylib-extras/rlImGui",
        ref_kind="branch",
        ref="main",
        directory="rlImGui",
        license_candidates=("LICENSE", "LICENSE.txt"),
    ),
    Vendor(
        name="FastNoiseLite",
        repo="Auburn/FastNoiseLite",
        ref_kind="branch",
        ref="master",
        directory="FastNoiseLite",
        license_candidates=("LICENSE", "LICENSE.txt"),
    ),
)


@dataclass(frozen=True)
class ResolvedVendor:
    vendor: Vendor
    archive_url: str
    resolved_ref: str
    resolved_ref_kind: str


def read_json(url: str) -> dict[str, object]:
    request = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json"})
    with urllib.request.urlopen(request, timeout=60) as response:
        return json.loads(response.read().decode("utf-8"))


def resolve_vendor(vendor: Vendor) -> ResolvedVendor:
    if vendor.ref_kind == "branch":
        api_url = f"https://api.github.com/repos/{vendor.repo}/commits/{vendor.ref}"
        payload = read_json(api_url)
        sha = str(payload["sha"])
        return ResolvedVendor(
            vendor=vendor,
            archive_url=f"https://github.com/{vendor.repo}/archive/{sha}.zip",
            resolved_ref=sha,
            resolved_ref_kind="commit",
        )

    return ResolvedVendor(
        vendor=vendor,
        archive_url=vendor.archive_url,
        resolved_ref=vendor.ref,
        resolved_ref_kind=vendor.ref_kind,
    )


def download(url: str, destination: Path) -> str:
    print(f"Downloading {url}")
    digest = hashlib.sha256()
    with urllib.request.urlopen(url, timeout=60) as response, destination.open("wb") as output:
        while True:
            chunk = response.read(1024 * 1024)
            if not chunk:
                break
            digest.update(chunk)
            output.write(chunk)
    return digest.hexdigest()


def first_directory(path: Path) -> Path:
    entries = [entry for entry in path.iterdir() if entry.is_dir()]
    if len(entries) != 1:
        raise RuntimeError(f"Expected one top-level directory in archive, found {len(entries)}")
    return entries[0]


def verify_license(vendor: Vendor, target: Path) -> None:
    for candidate in vendor.license_candidates:
        if (target / candidate).exists():
            return
    expected = ", ".join(vendor.license_candidates)
    print(f"warning: {vendor.name} license file not found; expected one of: {expected}", file=sys.stderr)


def write_revision(resolved: ResolvedVendor, target: Path, sha256: str) -> None:
    vendor = resolved.vendor
    revision = {
        "name": vendor.name,
        "repo": vendor.repo,
        "requested_ref_kind": vendor.ref_kind,
        "requested_ref": vendor.ref,
        "resolved_ref_kind": resolved.resolved_ref_kind,
        "resolved_ref": resolved.resolved_ref,
        "archive_url": resolved.archive_url,
        "archive_sha256": sha256,
    }
    (target / "OIKUMENE_VENDOR_REVISION.json").write_text(
        json.dumps(revision, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )


def vendor_one(vendor: Vendor, dry_run: bool) -> None:
    target = THIRD_PARTY_DIR / vendor.directory
    print(f"\n==> {vendor.name} -> {target.relative_to(REPO_ROOT)}")
    print(f"ref: {vendor.ref_kind}:{vendor.ref}")

    if dry_run:
        print(f"would download: {vendor.archive_url}")
        print(f"would replace:  {target}")
        return

    with tempfile.TemporaryDirectory(prefix=f"oikumene-{vendor.directory}-") as temp_name:
        temp_dir = Path(temp_name)
        archive = temp_dir / f"{vendor.directory}.zip"
        resolved = resolve_vendor(vendor)
        if resolved.resolved_ref_kind == "commit":
            print(f"resolved commit: {resolved.resolved_ref}")
        sha256 = download(resolved.archive_url, archive)

        extract_dir = temp_dir / "extract"
        extract_dir.mkdir()
        with zipfile.ZipFile(archive) as zip_file:
            zip_file.extractall(extract_dir)

        extracted_root = first_directory(extract_dir)
        replacement = temp_dir / vendor.directory
        shutil.move(str(extracted_root), replacement)

        if target.exists():
            shutil.rmtree(target)
        shutil.move(str(replacement), target)

        verify_license(vendor, target)
        write_revision(resolved, target, sha256)

    print(f"installed {vendor.name}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print planned downloads without changing third_party.",
    )
    parser.add_argument(
        "--only",
        choices=[vendor.directory for vendor in VENDORS],
        action="append",
        help="Download only one vendor directory. Can be passed more than once.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    THIRD_PARTY_DIR.mkdir(parents=True, exist_ok=True)

    selected = VENDORS
    if args.only:
        selected = tuple(vendor for vendor in VENDORS if vendor.directory in set(args.only))

    for vendor in selected:
        vendor_one(vendor, dry_run=args.dry_run)

    print("\nDone.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
