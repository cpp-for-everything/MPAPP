#!/usr/bin/env python3
"""
Rewrite example .cpp files to use `mpapp::platform::current` and the
cross-platform umbrella handler headers (`<mpapp/handlers/X_handler.hpp>`),
so a single source compiles on every supported platform.

Edits in place:
  1. `#include <mpapp/handlers/<plat>/X_handler.hpp>`
       -> `#include <mpapp/handlers/X_handler.hpp>` (deduped)
  2. `mpapp::platform::{windows,linux_,android,macos,ios}` as a template arg
       -> `mpapp::platform::current`

Idempotent — running again on already-rewritten files is a no-op.
"""
import os, re, sys

REPO = "D:/GitHub/MPAPP"
EXAMPLES = os.path.join(REPO, "examples")

# Match e.g. `#include <mpapp/handlers/linux/button_handler.hpp>` (or windows/android/macos/ios)
INC_RX = re.compile(
    r'#include\s+<mpapp/handlers/(?:windows|linux|android|macos|ios)/([A-Za-z0-9_]+\.hpp)>'
)
# Match `mpapp::platform::windows` etc. (not `current` and not `mock`).
# Conservative: only catches the 5 platform tags by name.
TAG_RX = re.compile(r'\bmpapp::platform::(windows|linux_|android|macos|ios)\b')

def rewrite(text):
    new = INC_RX.sub(lambda m: f'#include <mpapp/handlers/{m.group(1)}>', text)
    new = TAG_RX.sub('mpapp::platform::current', new)

    # Dedupe consecutive identical #include lines that the substitution
    # may have collapsed (e.g. an `#include <mpapp/handlers/windows/X.hpp>`
    # adjacent to an `<mpapp/handlers/linux/X.hpp>` — rare but possible).
    lines = new.splitlines(keepends=True)
    out = []
    seen_includes = set()
    for line in lines:
        m = re.match(r'^\s*#include\s+(<mpapp/handlers/[^>]+>)\s*$', line)
        if m:
            key = m.group(1)
            if key in seen_includes:
                continue
            seen_includes.add(key)
        out.append(line)
    return "".join(out)

def main():
    touched = 0
    skipped = 0
    for root, _, files in os.walk(EXAMPLES):
        # skip build artifacts under .cxx / build
        if any(seg in root for seg in (".cxx", "/build/", "\\build\\", "_deps")):
            continue
        for fn in files:
            if not fn.endswith((".cpp", ".hpp", ".h", ".cc", ".cxx", ".mm")):
                continue
            path = os.path.join(root, fn)
            with open(path, encoding="utf-8") as f:
                text = f.read()
            new = rewrite(text)
            if new != text:
                with open(path, "w", encoding="utf-8", newline="\n") as f:
                    f.write(new)
                touched += 1
                rel = os.path.relpath(path, REPO).replace("\\", "/")
                print(f"  rewrote {rel}")
            else:
                skipped += 1
    print(f"\nrewrote {touched} files / scanned {touched + skipped}")

if __name__ == "__main__":
    main()
