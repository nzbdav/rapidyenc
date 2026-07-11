#!/usr/bin/env python3
"""Package a rapidyenc shared library for a GitHub release."""

from __future__ import annotations

import argparse
import re
import shutil
import tarfile
import tempfile
import zipfile
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--rid", required=True)
    parser.add_argument("--version", required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    return parser.parse_args()


def expected_library(rid: str) -> tuple[str, ...]:
    if rid.startswith("win-"):
        return ("rapidyenc.dll", "librapidyenc.dll")
    if rid.startswith(("linux-", "osx-")):
        return ("librapidyenc.so", "librapidyenc.dylib")
    raise ValueError(f"unsupported runtime identifier: {rid}")


def find_library(build_dir: Path, names: tuple[str, ...]) -> Path:
    matches = [path for name in names for path in build_dir.rglob(name) if path.is_file()]
    if len(matches) != 1:
        raise RuntimeError(f"expected exactly one of {names} under {build_dir}, found: {matches}")
    return matches[0]


def verify_header_version(root: Path, version: str) -> None:
    header = (root / "rapidyenc.h").read_text(encoding="utf-8")
    parts = re.findall(r"#define RAPIDYENC_VERSION_(?:MAJOR|MINOR|PATCH) (\d+)", header)
    if len(parts) != 3 or ".".join(parts) != version:
        raise RuntimeError(f"header version does not match {version}")


def main() -> None:
    args = parse_args()
    root = Path(__file__).resolve().parents[1]
    verify_header_version(root, args.version)
    library = find_library(args.build_dir, expected_library(args.rid))
    args.output_dir.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as temp:
        staging = Path(temp) / f"rapidyenc-{args.version}-{args.rid}"
        (staging / "include").mkdir(parents=True)
        (staging / "lib").mkdir()
        (staging / "licenses").mkdir()
        shutil.copy2(root / "rapidyenc.h", staging / "include" / "rapidyenc.h")
        shutil.copy2(library, staging / "lib" / ("rapidyenc.dll" if args.rid.startswith("win-") else library.name))
        shutil.copy2(root / "LICENSE", staging / "licenses" / "CC0-1.0.txt")
        shutil.copy2(root / "crcutil-1.0" / "LICENSE", staging / "licenses" / "crcutil-Apache-2.0.txt")

        if args.rid.startswith("win-"):
            archive = args.output_dir / f"{staging.name}.zip"
            with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as output:
                for file in staging.rglob("*"):
                    if file.is_file():
                        output.write(file, file.relative_to(staging.parent))
        else:
            archive = args.output_dir / f"{staging.name}.tar.gz"
            with tarfile.open(archive, "w:gz") as output:
                output.add(staging, arcname=staging.name)

    print(archive)


if __name__ == "__main__":
    main()
