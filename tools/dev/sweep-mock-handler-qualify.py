#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Qualify bare component-handler names in mock tests with `internal::`.

Companion to sweep-mock-test-includes.py. Once the mock tests stop
including the component *wrapper* headers (which exported the
convenience alias `mpapp::<X>_handler = internal::<X>_handler`), any test
that named the handler unqualified (`button_handler<platform::mock>`)
no longer compiles. This sweep prefixes those usages with `internal::`.

Four handlers are declared at top-level `mpapp::` scope (not in
`internal/`) and must stay unqualified: view_handler, layout_handler,
application_handler, bindable_layout_handler. Everything else is an
`internal::` handler. The sets are derived from the headers so the
script tracks reality.

Idempotent (already-qualified `internal::X_handler` is skipped via the
not-preceded-by-`:` lookbehind). Run from repo root:
`python tools/dev/sweep-mock-handler-qualify.py`.
"""

from __future__ import annotations

import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
TESTS_DIR = REPO / "tests" / "mock_handlers"
INC = REPO / "include" / "mpapp"


def handler_names(glob_dir: Path, pattern: str) -> set[str]:
    names: set[str] = set()
    for hpp in glob_dir.glob(pattern):
        for m in re.finditer(r'class ([a-z_]+_handler);', hpp.read_text(encoding="utf-8")):
            names.add(m.group(1))
    return names


# Top-level handlers (declared in include/mpapp/*.hpp) stay `mpapp::`.
TOP_LEVEL = handler_names(INC, "*.hpp")
# Internal handlers (declared in include/mpapp/internal/basic_*.hpp).
INTERNAL = handler_names(INC / "internal", "basic_*.hpp") - TOP_LEVEL

# Bare handler token immediately followed by `<`, not already part of a
# qualified name (not preceded by a word char or `:`): `X_handler<…>`.
BARE_RE = re.compile(r'(?<![\w:])([a-z_]+_handler)(?=<)')

# `mpapp::X_handler<…>` — the wrapper-exported convenience alias, which
# is gone once the wrapper header is no longer included. Rewrite to
# `mpapp::internal::X_handler`. (Already-rewritten `mpapp::internal::…`
# does not match because `internal` is not `[a-z_]+_handler`.)
MPAPP_RE = re.compile(r'\bmpapp::([a-z_]+_handler)(?=<)')


def qualify_line(line: str) -> str:
    stripped = line.lstrip()
    if stripped.startswith("//") or stripped.startswith("*"):
        return line  # leave comments untouched

    def bare_repl(m: re.Match[str]) -> str:
        name = m.group(1)
        return f"internal::{name}" if name in INTERNAL else name

    def mpapp_repl(m: re.Match[str]) -> str:
        name = m.group(1)
        return f"mpapp::internal::{name}" if name in INTERNAL else f"mpapp::{name}"

    line = MPAPP_RE.sub(mpapp_repl, line)
    line = BARE_RE.sub(bare_repl, line)
    return line


def main() -> int:
    total = 0
    for test in sorted(TESTS_DIR.glob("*_test.cpp")):
        lines = test.read_text(encoding="utf-8").splitlines(keepends=True)
        n = 0
        for i, line in enumerate(lines):
            new = qualify_line(line)
            if new != line:
                lines[i] = new
                n += 1
        if n:
            total += n
            test.write_text("".join(lines), encoding="utf-8")
            print(f"{test.name}: {n} handler usage(s) qualified with internal::")
    print(f"\nDone. {total} usage(s) qualified.")
    print(f"top-level (left as mpapp::): {sorted(TOP_LEVEL)}")
    print(f"internal handler count: {len(INTERNAL)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
