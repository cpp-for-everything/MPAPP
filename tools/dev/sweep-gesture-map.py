#!/usr/bin/env python3
"""Bulk-add `map_gestures` to every migrated component's per-platform
+ mock handler, with Linux .cpp implementations that delegate to
`linux_gestures::attach(...)`.

Discovers migrated components from `include/mpapp/internal/basic_*.hpp`
(skipping `button` which is the pilot already wired by hand).

Per component, for each per-platform handler:
  * If hpp file does not declare `map_gestures`, inject
    `void map_gestures(basic_<name>&) noexcept {}` inside the class
    right before the first `private:` (or before the closing `};` if
    the class is all-public).
  * On Linux, additionally write the real definition into the .cpp:
    `void <name>_handler<platform::linux_>::map_gestures(basic_<name>& b)`
    that calls `linux_gestures::attach(static_cast<GtkWidget*>(native_), b);`,
    plus a `#include "mpapp/handlers/linux/gesture_attach.hpp"` near
    the top if not already present.
  * The hpp variant for Linux is the *declaration*, matching the
    canonical `map_text`/`map_clicked` pattern (the .cpp owns the
    definition).

Idempotent: if `map_gestures` is already declared / defined in a
target file, the script skips that file.

The wrapper ctor's `embedded_handler_.map_gestures(*this);` call is
emitted by `migrate-component.py`'s updated WRAPPER_TEMPLATE — invoke
that script with `--regen-wrapper` afterwards to refresh wrappers.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
INCL = ROOT / "include" / "mpapp"
SRC = ROOT / "src" / "handlers"
INTERNAL = INCL / "internal"

PLATFORMS = ["linux", "windows", "android", "macos", "ios"]


def migrated_components() -> list[str]:
    out = []
    for p in sorted(INTERNAL.glob("basic_*.hpp")):
        out.append(p.stem[len("basic_"):])
    return out


def handler_base(name: str) -> str:
    return name.rstrip("_")


def has_map_gestures(text: str) -> bool:
    return bool(re.search(r"\bmap_gestures\s*\(", text))


def inject_decl_before_private(text: str, decl: str) -> str | None:
    """Insert `decl` (already terminated with a newline) inside the
    class, immediately before the first `private:` access specifier.
    If no `private:` exists, return None (caller falls back to inserting
    before the closing `};`)."""
    m = re.search(r"^(\s*)private:\s*$", text, flags=re.MULTILINE)
    if not m:
        return None
    indent = m.group(1)
    before = text[: m.start()]
    after = text[m.start() :]
    indented = "\n".join(
        (indent + line) if line.strip() else line
        for line in decl.splitlines()
    )
    return before + indented + "\n\n" + after


def inject_decl_before_class_close(text: str, decl: str) -> str | None:
    """Fallback: insert before the class's closing `};`. Picks the LAST
    `};` followed by an end-of-namespace marker."""
    m = re.search(r"\n(\s*)\}\s*;\s*\n\s*\}\s*//\s*namespace\s+mpapp",
                  text)
    if not m:
        return None
    indent = (m.group(1) or "") + "    "  # one level deeper than `};`
    indented = "\n".join(
        (indent + line) if line.strip() else line
        for line in decl.splitlines()
    )
    return text[: m.start()] + "\n" + indented + text[m.start():]


def add_decl(name: str, hpp_path: Path, noexcept_inline: bool) -> bool:
    """Add `void map_gestures(basic_<name>&)` to the class. If
    `noexcept_inline` is True, emit the no-op inline body; otherwise
    just a declaration (Linux real-handler hpp). Returns True iff the
    file was modified."""
    if not hpp_path.exists():
        return False
    text = hpp_path.read_text(encoding="utf-8")
    if has_map_gestures(text):
        return False
    if noexcept_inline:
        decl = (
            "// RFC-0003 stub: per-platform real gesture wire-up is\n"
            "// pending the platform's real-handler task. No-op today\n"
            "// so the wrapper ctor's unconditional\n"
            "// `embedded_handler_.map_gestures(*this);` links.\n"
            f"void map_gestures(basic_{name}& /*x*/) noexcept {{}}\n"
        )
    else:
        decl = (
            "// RFC-0003: walks `x.gesture_recognizers` and installs\n"
            "// matching GtkGesture* controllers via\n"
            "// `mpapp::internal::linux_gestures::attach`.\n"
            f"void map_gestures(basic_{name}& x);\n"
        )
    new_text = inject_decl_before_private(text, decl)
    if new_text is None:
        new_text = inject_decl_before_class_close(text, decl)
    if new_text is None:
        return False
    hpp_path.write_text(new_text, encoding="utf-8")
    return True


def add_linux_impl(name: str) -> bool:
    """Append the Linux real implementation to
    `src/handlers/linux/<base>_handler.cpp`. Returns True iff modified."""
    base = handler_base(name)
    cpp_path = SRC / "linux" / f"{base}_handler.cpp"
    if not cpp_path.exists():
        return False
    text = cpp_path.read_text(encoding="utf-8")
    if has_map_gestures(text):
        return False

    # Insert `#include "mpapp/handlers/linux/gesture_attach.hpp"` after
    # the existing `#include "mpapp/handlers/linux/<base>_handler.hpp"`
    # line. Skip if already included.
    include_line = '#include "mpapp/handlers/linux/gesture_attach.hpp"'
    if include_line not in text:
        text = re.sub(
            rf'(#include "mpapp/handlers/linux/{re.escape(base)}_handler\.hpp"\n)',
            rf"\1\n{include_line}\n",
            text,
            count=1,
        )

    # Append the implementation inside `namespace mpapp::internal {}`.
    # Insert before the closing `} // namespace mpapp::internal`.
    impl = (
        f"\nvoid {base}_handler<platform::linux_>::map_gestures(basic_{name}& x) {{\n"
        f"    if (native_ == nullptr) return;\n"
        f"    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);\n"
        f"}}\n"
    )
    new_text, n = re.subn(
        r"(\n\}\s*//\s*namespace\s+mpapp::internal\s*\n)",
        impl + r"\1",
        text,
        count=1,
    )
    if n == 0:
        return False
    cpp_path.write_text(new_text, encoding="utf-8")
    return True


def main() -> int:
    names = [n for n in migrated_components() if n != "button"]
    print(f"Sweeping {len(names)} components (excluding button — pilot).")

    decl_changes = 0
    impl_changes = 0
    for name in names:
        base = handler_base(name)
        # Real Linux declaration (no inline body — impl in .cpp).
        if add_decl(name,
                    INCL / "handlers" / "linux" / f"{base}_handler.hpp",
                    noexcept_inline=False):
            decl_changes += 1
        # No-op stubs for other platforms + mock.
        for plat in ["windows", "android", "macos", "ios", "mock"]:
            if add_decl(name,
                        INCL / "handlers" / plat / f"{base}_handler.hpp",
                        noexcept_inline=True):
                decl_changes += 1
        # Linux real implementation.
        if add_linux_impl(name):
            impl_changes += 1

    print(f"hpp injections: {decl_changes}")
    print(f"linux .cpp impls: {impl_changes}")
    print("\nNext: re-run migrate-component.py with --regen-wrapper to refresh "
          "every wrapper ctor with the new map_gestures call.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
