#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Repoint mock-handler test includes from component wrappers to basic surfaces.

Background (T-0032 Path B): every `tests/mock_handlers/*_test.cpp` already
exercises the platform-agnostic `mpapp::internal::basic_<X>` surface + a
`platform::mock` handler (verified: no test body instantiates a wrapper
type). But many of them still `#include <mpapp/<X>.hpp>` — the *wrapper*
header, which after ADR-0024 embeds a `<platform::current>` handler and
therefore transitively pulls the WinUI 3 / GTK4 / etc. platform headers.
On a clean Windows runner (no WindowsAppSDK) that breaks the build with
`fatal error C1083: 'winrt/Microsoft.UI.Xaml.Controls.h'`.

This sweep swaps each `#include <mpapp/<X>.hpp>` for
`#include <mpapp/internal/basic_<X>.hpp>` whenever that basic surface
header exists, leaving genuinely platform-neutral includes (view.hpp,
layout.hpp, executor.hpp, route.hpp, cell.hpp, hybrid_bridge.hpp, …) and
any include that has no `basic_` counterpart untouched. The result:
`mock_handlers_test` compiles with zero platform-SDK dependency, so it
can run in cloud Windows CI.

Idempotent. Run from the repo root: `python tools/dev/sweep-mock-test-includes.py`.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
TESTS_DIR = REPO / "tests" / "mock_handlers"
INTERNAL_DIR = REPO / "include" / "mpapp" / "internal"

# Components that have an `internal/basic_<X>.hpp` surface — derived from
# the filesystem so the script stays correct as surfaces are added.
BASIC_SURFACES = {
    p.stem[len("basic_"):]
    for p in INTERNAL_DIR.glob("basic_*.hpp")
}

INCLUDE_RE = re.compile(r'^#include <mpapp/([a-z_]+)\.hpp>\s*$')


def sweep_file(path: Path) -> list[tuple[str, str]]:
    changed: list[tuple[str, str]] = []
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    for i, line in enumerate(lines):
        m = INCLUDE_RE.match(line.rstrip("\n").rstrip("\r"))
        if not m:
            continue
        comp = m.group(1)
        if comp not in BASIC_SURFACES:
            continue  # platform-neutral or no basic surface — leave as-is
        eol = "\r\n" if line.endswith("\r\n") else "\n"
        new_line = f"#include <mpapp/internal/basic_{comp}.hpp>{eol}"
        if new_line != line:
            changed.append((line.strip(), new_line.strip()))
            lines[i] = new_line
    if changed:
        path.write_text("".join(lines), encoding="utf-8")
    return changed


def main() -> int:
    if not TESTS_DIR.is_dir():
        print(f"error: {TESTS_DIR} not found", file=sys.stderr)
        return 1
    total = 0
    for test in sorted(TESTS_DIR.glob("*_test.cpp")):
        changes = sweep_file(test)
        if changes:
            total += len(changes)
            print(f"{test.name}: {len(changes)} include(s) repointed")
            for old, new in changes:
                print(f"    {old}  ->  {new}")
    print(f"\nDone. {total} include(s) repointed across the mock-test suite.")
    print(f"basic_ surfaces available: {len(BASIC_SURFACES)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
