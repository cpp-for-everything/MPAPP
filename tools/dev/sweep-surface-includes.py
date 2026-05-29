#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Fix surface headers that include a sibling *wrapper* instead of its surface.

A `mpapp::internal::basic_<X>` surface header must stay platform-neutral:
it may depend on *other surfaces* (`basic_<Y>.hpp`) but never on a
component *wrapper* (`../<Y>.hpp`), because the wrapper embeds a
`<platform::current>` handler and so transitively pulls the WinUI 3 /
GTK4 / … SDK headers. Surfaces that did this (e.g. basic_content_page
including ../page.hpp for its `basic_page` base) broke the Windows core
build under T-0032 Path B.

This sweep rewrites each `#include "../<Y>.hpp"` in
include/mpapp/internal/basic_*.hpp to `#include "basic_<Y>.hpp"` when a
`basic_<Y>.hpp` surface exists. The classes already inherit / use the
`internal::basic_<Y>` types (verified), so only the include line is
wrong. Includes with no basic_ counterpart are left untouched.

Idempotent. Run from repo root: `python tools/dev/sweep-surface-includes.py`.
"""

from __future__ import annotations

import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
INTERNAL = REPO / "include" / "mpapp" / "internal"

SURFACES = {p.stem[len("basic_"):] for p in INTERNAL.glob("basic_*.hpp")}

# `#include "../<comp>.hpp"` — a sibling wrapper include from inside internal/.
WRAPPER_INC = re.compile(r'^(#include ")\.\./([a-z_]+)\.hpp(".*)$')


def sweep(path: Path) -> list[tuple[str, str]]:
    changed: list[tuple[str, str]] = []
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    for i, line in enumerate(lines):
        body = line.rstrip("\r\n")
        m = WRAPPER_INC.match(body)
        if not m:
            continue
        comp = m.group(2)
        if comp not in SURFACES:
            continue  # no basic_ surface (e.g. a genuinely shared header) — leave
        eol = "\r\n" if line.endswith("\r\n") else "\n"
        new = f"{m.group(1)}basic_{comp}.hpp{m.group(3)}{eol}"
        if new != line:
            changed.append((body.strip(), new.strip()))
            lines[i] = new
    if changed:
        path.write_text("".join(lines), encoding="utf-8")
    return changed


def main() -> int:
    total = 0
    for hpp in sorted(INTERNAL.glob("basic_*.hpp")):
        for old, new in sweep(hpp):
            total += 1
            print(f"{hpp.name}: {old}  ->  {new}")
    print(f"\nDone. {total} surface include(s) repointed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
