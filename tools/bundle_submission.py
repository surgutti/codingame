#!/usr/bin/env python3
"""Bundle a multi-file C++ source tree into a single file.

This keeps the same include-expansion behavior as the original compress.py,
but adds a clearer CLI and stable path handling.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path
from typing import List, Set

INCLUDE_RE = re.compile(r'^\s*#include\s+"(.+)"\s*$')


class Bundler:
    def __init__(self, include_markers: bool = True) -> None:
        self.include_markers = include_markers
        self.included: Set[Path] = set()
        self.lines: List[str] = []

    def bundle(self, path: Path, indent: int = 0) -> None:
        path = path.resolve()

        if path in self.included:
            return

        self.included.add(path)

        prefix = " " * indent
        if self.include_markers:
            self.lines.append(f"{prefix}// *** Start of: {path} *** ")

        base_dir = path.parent
        for raw_line in path.read_text(encoding="utf-8").splitlines():
            line = raw_line.rstrip("\n")

            if line.strip() == "#pragma once":
                continue

            match = INCLUDE_RE.match(line)
            if match:
                include_path = (base_dir / match.group(1)).resolve()
                self.bundle(include_path, indent + 1)
                continue

            self.lines.append(f"{prefix}{line}")

        if self.include_markers:
            self.lines.append(f"{prefix}// *** End of: {path} *** ")

    def render(self) -> str:
        return "\n".join(self.lines) + "\n"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Bundle C++ sources by expanding local #include \"...\" directives.")
    parser.add_argument("entry", type=Path, help="Entry C++ file, e.g. brain.cpp")
    parser.add_argument("-o", "--output", type=Path, help="Output file (defaults to stdout)")
    parser.add_argument(
        "--no-markers",
        action="store_true",
        help="Do not emit // *** Start/End of ... *** markers",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    bundler = Bundler(include_markers=not args.no_markers)
    bundler.bundle(args.entry)
    output = bundler.render()

    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(output, encoding="utf-8")
    else:
        print(output, end="")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
