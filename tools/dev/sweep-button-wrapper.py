#!/usr/bin/env python3
"""Sweep examples to use mpapp::button wrapper pattern.

For each example main.cpp:
  - Collect mpapp::button_handler<> NAME_{}; member declarations
  - Delete those member declarations
  - Delete corresponding `<button>.set_handler(NAME);` calls
  - Delete corresponding `NAME.map_text(<button>);` calls
  - Delete corresponding `NAME.map_clicked(<button>);` calls
  - Delete the `#include <mpapp/handlers/button_handler.hpp>` line

The `bind_button` helper in async_bridge_demo + routes_demo (gtk4 +
windows variants) is rewritten by hand — too case-specific for a sweep.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXAMPLES = ROOT / "examples"


# `mpapp::button_handler<...>` declaration: e.g.
#   mpapp::button_handler<>          send_btn_handler_{};
#   mpapp::button_handler<wp>        btn_sync_handler_{};
# Matches any template-arg payload (empty, single, or qualified tag).
DECL_RE = re.compile(
    r"^\s*mpapp::button_handler<[^>]*>\s+(\w+)\s*\{\s*\}\s*;\s*$"
)


def sweep(path: Path) -> bool:
    text = path.read_text(encoding="utf-8")
    lines = text.splitlines(keepends=False)

    handler_names: list[str] = []
    for line in lines:
        m = DECL_RE.match(line)
        if m:
            handler_names.append(m.group(1))

    if not handler_names:
        return False

    out: list[str] = []
    drop_count = 0
    for line in lines:
        stripped = line.rstrip()

        m = DECL_RE.match(line)
        if m:
            drop_count += 1
            continue

        if any(re.fullmatch(rf"\s*\w+\.set_handler\(\s*{re.escape(h)}\s*\)\s*;\s*", stripped)
               for h in handler_names):
            drop_count += 1
            continue

        if any(re.fullmatch(rf"\s*{re.escape(h)}\.map_text\(\s*\w+\s*\)\s*;\s*", stripped)
               for h in handler_names):
            drop_count += 1
            continue

        if any(re.fullmatch(rf"\s*{re.escape(h)}\.map_clicked\(\s*\w+\s*\)\s*;\s*", stripped)
               for h in handler_names):
            drop_count += 1
            continue

        if re.fullmatch(r"\s*#include\s*<mpapp/handlers/button_handler\.hpp>\s*", stripped):
            drop_count += 1
            continue

        out.append(line)

    if drop_count == 0:
        return False

    new_text = "\n".join(out)
    if text.endswith("\n"):
        new_text += "\n"
    path.write_text(new_text, encoding="utf-8")
    print(f"{path.relative_to(ROOT)}: handlers={handler_names} dropped_lines={drop_count}")
    return True


def main() -> int:
    changed = 0
    for main_cpp in sorted(EXAMPLES.rglob("main.cpp")):
        if sweep(main_cpp):
            changed += 1

    for native in sorted(EXAMPLES.rglob("native_main.cpp")):
        if sweep(native):
            changed += 1

    print(f"\nDone. Files modified: {changed}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
