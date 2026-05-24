#!/usr/bin/env python3
"""
Add `= platform::current` default to every handler forward declaration
in surface headers, so user code can write `button_handler<>` instead
of `button_handler<mpapp::platform::current>`. Then sweep all examples
to drop the now-redundant `<mpapp::platform::current>` from instantiations.

This is the smaller of two refactors the user asked for:
  - Default template arg (this script — minimal cascade)
  - Move handlers into `internal::` + add `mpapp::<comp>_handler` aliases
    (deferred to a follow-up; needs ~400-file ripple)
"""
import os, re

REPO = "D:/GitHub/MPAPP"

# Forward declarations in surface headers look like:
#   template <class Platform>
#   class button_handler;
# (sometimes with a comment block before, sometimes on one line.)
FWD_RX = re.compile(
    r'(template\s*<\s*class\s+Platform\s*)(>\s*\n\s*class\s+\w+_handler\s*;)',
    re.MULTILINE,
)

def update_forward_decls():
    """Add `= platform::current` default to every surface-header forward decl."""
    touched = 0
    inc = os.path.join(REPO, "include", "mpapp")
    for fn in os.listdir(inc):
        if not fn.endswith(".hpp"): continue
        path = os.path.join(inc, fn)
        with open(path, encoding="utf-8") as f:
            text = f.read()
        new, n = FWD_RX.subn(r'\1 = platform::current\2', text)
        if n:
            with open(path, "w", encoding="utf-8", newline="\n") as f:
                f.write(new)
            touched += 1
    print(f"forward-decls: updated {touched} surface headers")
    return touched

# Drop `<mpapp::platform::current>` from `button_handler<...>` in examples.
# Match the full qualified form most users write.
EX_TPL_RX = re.compile(
    r'(\bmpapp::\w+_handler)<\s*mpapp::platform::current\s*>'
)

def update_examples():
    """Sweep example .cpp/.hpp to drop `<mpapp::platform::current>` from handler instantiations."""
    touched = 0
    ex = os.path.join(REPO, "examples")
    for root, _, files in os.walk(ex):
        if any(seg in root for seg in (".cxx", "/build/", "\\build\\", "_deps")):
            continue
        for fn in files:
            if not fn.endswith((".cpp", ".hpp", ".h", ".cc", ".mm")): continue
            path = os.path.join(root, fn)
            with open(path, encoding="utf-8") as f:
                text = f.read()
            new, n = EX_TPL_RX.subn(r'\1<>', text)
            if n:
                with open(path, "w", encoding="utf-8", newline="\n") as f:
                    f.write(new)
                touched += 1
                rel = os.path.relpath(path, REPO).replace("\\", "/")
                print(f"  rewrote {rel} ({n} instantiations)")
    print(f"\nexamples: rewrote {touched} files")
    return touched

if __name__ == "__main__":
    update_forward_decls()
    print()
    update_examples()
