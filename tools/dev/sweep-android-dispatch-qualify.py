#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Qualify Android JNI-trampoline calls with `internal::`.

The `src/handlers/android/*.cpp` JNI trampolines call the per-handler
dispatch free functions (`android_<X>_dispatch_<Y>`) and name handler
types (`<X>_handler<platform::android>`). The ADR-0024 migration moved
both into `mpapp::internal::`, but these trampolines still spelled them
`mpapp::…` — broken since the migration because the Android build had
been red for unrelated reasons and never recompiled them.

This sweep rewrites:
  * `mpapp::android_<…>`            -> `mpapp::internal::android_<…>`
  * `mpapp::<X>_handler<…>`         -> `mpapp::internal::<X>_handler<…>`
…except the four top-level handlers (view/layout/application/
bindable_layout) which really do live in `mpapp::`.

Idempotent (already-`mpapp::internal::…` doesn't re-match). Run from
repo root: `python tools/dev/sweep-android-dispatch-qualify.py`.
"""

from __future__ import annotations

import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
ANDROID = REPO / "src" / "handlers" / "android"
INC = REPO / "include" / "mpapp"

TOP_LEVEL = {
    m.group(1)
    for hpp in INC.glob("*.hpp")
    for m in re.finditer(r'class ([a-z_]+_handler);', hpp.read_text(encoding="utf-8"))
}

# `mpapp::android_foo_dispatch_bar` (the dispatch free functions, all in
# mpapp::internal). `mpapp::internal::android_…` won't match (the token
# after `mpapp::` is `internal`, not `android_`).
DISPATCH_RE = re.compile(r'\bmpapp::(android_[a-z_]+)\b')

# `mpapp::<X>_handler<` — the wrapper-alias spelling; rewrite internal
# handlers to mpapp::internal::, leave the 4 top-level ones.
HANDLER_RE = re.compile(r'\bmpapp::([a-z_]+_handler)(?=<)')


def fix_line(line: str) -> str:
    stripped = line.lstrip()
    if stripped.startswith("//") or stripped.startswith("*"):
        return line

    line = DISPATCH_RE.sub(lambda m: f"mpapp::internal::{m.group(1)}", line)

    def handler_repl(m: re.Match[str]) -> str:
        name = m.group(1)
        return f"mpapp::{name}" if name in TOP_LEVEL else f"mpapp::internal::{name}"

    line = HANDLER_RE.sub(handler_repl, line)
    return line


def main() -> int:
    total = 0
    for cpp in sorted(ANDROID.glob("*.cpp")):
        lines = cpp.read_text(encoding="utf-8").splitlines(keepends=True)
        n = 0
        for i, line in enumerate(lines):
            new = fix_line(line)
            if new != line:
                lines[i] = new
                n += 1
        if n:
            total += n
            cpp.write_text("".join(lines), encoding="utf-8")
            print(f"{cpp.name}: {n} line(s) qualified")
    print(f"\nDone. {total} line(s) qualified across src/handlers/android/.")
    print(f"top-level handlers left as mpapp:: -> {sorted(TOP_LEVEL)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
