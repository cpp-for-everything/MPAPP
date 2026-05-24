#!/usr/bin/env python3
"""
Many examples shortcut `mpapp::platform::current` via a local
`using lp = mpapp::platform::current;` alias, then write
`<comp>_handler<lp>` throughout. With the default template arg now
in place, both the local alias and the `<lp>` instantiation arg can
go away — `<>` works.

Sweeps example source for the pattern; idempotent.
"""
import os, re

REPO = "D:/GitHub/MPAPP"
EX   = os.path.join(REPO, "examples")

# A local namespace-scope alias of the platform-current type, in any
# of the abbreviations examples use today.
ALIAS_RX = re.compile(
    r'^[\t ]*using\s+(lp|plat|p|current_plat)\s*=\s*mpapp::platform::current\s*;\s*\n',
    re.MULTILINE,
)
# Match `<comp>_handler<lp>` (or one of the other short names) so the
# default template arg takes over.
TPL_RX = re.compile(
    r'(\b\w+_handler)<\s*(lp|plat|p|current_plat)\s*>'
)

def main():
    touched = 0
    for root, _, files in os.walk(EX):
        if any(seg in root for seg in (".cxx", "/build/", "\\build\\", "_deps")):
            continue
        for fn in files:
            if not fn.endswith((".cpp", ".hpp", ".h", ".cc", ".mm")): continue
            path = os.path.join(root, fn)
            with open(path, encoding="utf-8") as f:
                text = f.read()
            new = TPL_RX.sub(r'\1<>', text)
            new = ALIAS_RX.sub('', new)
            if new != text:
                with open(path, "w", encoding="utf-8", newline="\n") as f:
                    f.write(new)
                touched += 1
                rel = os.path.relpath(path, REPO).replace("\\", "/")
                print(f"  rewrote {rel}")
    print(f"\nrewrote {touched} files")

if __name__ == "__main__":
    main()
