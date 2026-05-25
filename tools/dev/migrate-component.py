#!/usr/bin/env python3
"""Migrate a single MPAPP component from the separate-handler pattern to
the wrapper-component pattern (Button pilot replicated for all leaf
components).

For component NAME:
  - Generate `include/mpapp/internal/basic_NAME.hpp` with the surface
    class renamed `NAME` -> `basic_NAME`, moved into namespace
    `mpapp::internal`.
  - Rewrite `include/mpapp/NAME.hpp` as a thin wrapper: include the
    internal/basic_NAME.hpp + the per-platform handler umbrella; define
    `mpapp::NAME : public internal::basic_NAME` that embeds the handler
    by value and auto-binds in its constructor; define the
    `template <class P = platform::current> using NAME_handler =
    internal::NAME_handler<P>;` alias.
  - Update each per-platform handler header
    (`include/mpapp/handlers/{linux,windows,android,macos,ios}/NAME_handler.hpp`)
    + the mock header
    (`include/mpapp/handlers/mock/NAME_handler.hpp`) to:
        * Include `../../internal/basic_NAME.hpp` instead of `../../NAME.hpp`
        * Move from `namespace mpapp { ... }` to `namespace mpapp::internal { ... }`
        * Change every `NAME& b` parameter into `basic_NAME& b`
  - Update each per-platform handler .cpp / .mm (under
    `src/handlers/...`) to:
        * Move to `namespace mpapp::internal { ... }`
        * Change every `NAME& b` parameter into `basic_NAME& b`
        * Update the dispatch_NAME function: cast to
          `::mpapp::internal::basic_NAME*` instead of `::mpapp::NAME*`
        * Update `mpapp::detail::` references in the file body to
          `::mpapp::detail::` (otherwise the moved namespace would
          search for `mpapp::internal::detail::`).

Usage:
  python tools/dev/migrate-component.py NAME [NAME ...]
  python tools/dev/migrate-component.py --all-bucket-a    # everything fitting the simple-leaf pattern

Generated files have a SPDX header that mirrors the source's banner.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import Optional

ROOT = Path(__file__).resolve().parents[2]
INCL = ROOT / "include" / "mpapp"
SRC = ROOT / "src" / "handlers"
TESTS = ROOT / "tests" / "mock_handlers"
PLATFORMS = ["linux", "windows", "android", "macos", "ios"]


# ---------------- Helpers ---------------------------------------------------

def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def replace_word(text: str, old: str, new: str) -> str:
    """Replace occurrences of `old` as a whole identifier with `new`."""
    return re.sub(rf"\b{re.escape(old)}\b", new, text)


def replace_type_word(text: str, old: str, new: str) -> str:
    """Like `replace_word`, but skip the following non-type contexts:

      * Member access (`.old`, `->old`) — `c.label` reads an Observable
        property called `label`, not the surrounding class type.

      * Short string literals where the token IS the entire string
        (`"old"`) — these are usually property-name labels passed to
        `record_change("label", …)` in mock handlers; renaming them
        breaks the test assertions.

    Strings that merely contain `old` (`"the old way"`) are not
    detected; treat that as a known limitation — the dual-quote case
    above covers the realistic mock-handler property-name pattern."""
    return re.sub(
        rf'(?<![\w.])(?<!->)(?<!"){re.escape(old)}(?!")\b',
        new,
        text,
    )


# ---------------- map_X discovery ------------------------------------------

MAP_RE = re.compile(r"\bvoid\s+map_([a-zA-Z_]\w*)\s*\(\s*\w+\s*&\s*\w+\s*\)")


def discover_map_methods(handler_text: str) -> list[str]:
    """Return the set of `map_X` method names in a handler header.
    Order-preserving (uses appearance order)."""
    seen: list[str] = []
    for m in MAP_RE.finditer(handler_text):
        name = m.group(1)
        if name not in seen:
            seen.append(name)
    return seen


# ---------------- Generators -----------------------------------------------

def _find_class_block(lines: list[str], name: str) -> tuple[int, int, int]:
    """Locate (forward_decl_start, class_def_start, class_def_end_inclusive)
    in `lines`. The forward-decl range covers a `template <...> class
    NAME_handler;` line (and the template line above it) if present
    immediately before the class definition (with only blank/comment
    lines between).

    Raises ValueError if the class line cannot be found."""
    # Accept the optional `[[deprecated(...)]]` attribute between `class`
    # and the name (frame.hpp has it, anticipating others might).
    class_re = re.compile(
        rf"^\s*class\s+(?:\[\[[^\]]*\]\]\s+)*{re.escape(name)}\s*(?:[:;{{]|$)"
    )
    class_def_start = -1
    for i, line in enumerate(lines):
        if class_re.match(line):
            class_def_start = i
            break
    if class_def_start < 0:
        raise ValueError(f"class {name} not found")

    # Walk forward to matching `};` accounting for nested braces.
    # Strip string literals, char literals, and `//` comments BEFORE
    # counting so the `'{'` char literal in something like
    #   if (payload.front() == '{') { ... }
    # doesn't double-increment the depth.
    str_re   = re.compile(r'"(?:[^"\\]|\\.)*"')
    char_re  = re.compile(r"'(?:[^'\\]|\\.)*'")
    line_cmt = re.compile(r"//.*$")
    depth = 0
    class_def_end = -1
    started = False
    for i in range(class_def_start, len(lines)):
        line = lines[i]
        line = str_re.sub('""', line)
        line = char_re.sub("''", line)
        line = line_cmt.sub("", line)
        for ch in line:
            if ch == '{':
                depth += 1
                started = True
            elif ch == '}':
                depth -= 1
                if started and depth == 0:
                    class_def_end = i
                    break
        if class_def_end >= 0:
            break
    if class_def_end < 0:
        # Probably `class NAME;` forward decl (no body). Treat as a
        # single-line block.
        class_def_end = class_def_start

    # Walk back from class_def_start to find an attached forward decl
    # `template <...> class NAME_handler;`. Skip blank lines and `//`
    # comment lines.
    forward_decl_start = class_def_start
    handler_decl_re = re.compile(
        rf"^\s*class\s+{re.escape(name.rstrip('_'))}_handler\s*;")
    i = class_def_start - 1
    blanks: list[int] = []
    while i >= 0:
        s = lines[i]
        stripped = s.strip()
        if stripped == "":
            blanks.append(i)
            i -= 1
            continue
        if stripped.startswith("//"):
            blanks.append(i)
            i -= 1
            continue
        if handler_decl_re.match(s):
            forward_decl_start = i
            # Check the line above for a `template <...>` line and
            # absorb it too.
            j = i - 1
            while j >= 0:
                t = lines[j].strip()
                if t == "" or t.startswith("//"):
                    j -= 1
                    continue
                if t.startswith("template"):
                    forward_decl_start = j
                break
            break
        break

    return forward_decl_start, class_def_start, class_def_end


def generate_basic_hpp(name: str, original: str) -> str:
    """Generate `internal/basic_NAME.hpp` by surgically splitting the
    original `NAME.hpp`:

      * Public types (structs, enums, helpers) inside `namespace mpapp {}`
        stay in `mpapp::` — external code and `std::formatter`
        specialisations reference them by their public names.

      * The component class (and its `NAME_handler` forward decl) move
        to a fresh `namespace mpapp::internal {}` block, with the class
        renamed `NAME` -> `basic_NAME` so it can serve as the surface
        the wrapper inherits and the handlers map.

    For the simple case (the class is the only thing inside the
    namespace), this collapses to the same output as the naive `replace
    namespace + rename` approach used for the button pilot — no extra
    empty `namespace mpapp {}` block is emitted.
    """
    text = original

    # Bump the include guard so it does not collide with the wrapper header.
    text = re.sub(
        r"#ifndef\s+MPAPP_([A-Z0-9_]+)_HPP",
        rf"#ifndef MPAPP_INTERNAL_BASIC_\1_HPP",
        text,
    )
    text = re.sub(
        r"#define\s+MPAPP_([A-Z0-9_]+)_HPP",
        rf"#define MPAPP_INTERNAL_BASIC_\1_HPP",
        text,
    )
    text = re.sub(
        r"#endif\s+//\s*MPAPP_([A-Z0-9_]+)_HPP",
        rf"#endif // MPAPP_INTERNAL_BASIC_\1_HPP",
        text,
    )

    # Adjust relative includes — we are one directory deeper now.
    text = re.sub(
        r'#include\s+"(?!\.\.)([^"]+\.hpp)"',
        r'#include "../\1"',
        text,
    )

    lines = text.splitlines(keepends=False)

    # Find the public namespace open/close.
    ns_open_re = re.compile(r"^namespace\s+mpapp\s*\{")
    ns_close_re = re.compile(r"^\}\s*//\s*namespace\s+mpapp\s*$")
    ns_open = -1
    ns_close = -1
    for i, line in enumerate(lines):
        if ns_open < 0 and ns_open_re.match(line):
            ns_open = i
        elif ns_open >= 0 and ns_close < 0 and ns_close_re.match(line):
            ns_close = i
    if ns_open < 0 or ns_close < 0:
        raise ValueError(
            f"could not find `namespace mpapp {{ ... }}` block in source")

    forward_start, class_start, class_end = _find_class_block(lines, name)

    # Slice the namespace body around the class block.
    pre_class = lines[ns_open + 1 : forward_start]       # types, comments
    class_block = lines[forward_start : class_end + 1]   # forward + class
    post_class = lines[class_end + 1 : ns_close]         # rarely non-empty

    # Drop trailing-blank-line noise from the segments before/after the
    # class so we don't emit `namespace mpapp { \n }`-only blocks.
    def trim_blanks(seg: list[str]) -> list[str]:
        out = list(seg)
        while out and out[0].strip() == "":
            out.pop(0)
        while out and out[-1].strip() == "":
            out.pop()
        return out

    pre_class = trim_blanks(pre_class)
    post_class = trim_blanks(post_class)
    class_block = trim_blanks(class_block)

    # Rename `class NAME` -> `class basic_NAME` (and standalone NAME
    # tokens, e.g. constructor / destructor mentions) within the class
    # block. The handler template name `NAME_handler` is NOT a word-
    # boundary match for plain `NAME` because the underscore is a word
    # char, so we don't accidentally rename it.
    class_block = [replace_word(l, name, f"basic_{name}") for l in class_block]

    # For every sibling component that's ALREADY been migrated (its
    # `internal/basic_<other>.hpp` exists on disk), rewrite each
    # standalone use of `<other>` inside this class's body to its surface
    # form. Three patterns matter:
    #   * inheritance: `public other`        -> `public internal::basic_other`
    #   * member type: `Observable<other*>`  -> `Observable<basic_other*>`
    #   * method args: `void add_tab(other* p)` -> `void add_tab(basic_other* p)`
    # Identifier names like `current_page` are NOT touched because the
    # `\b...\b` boundary fails between the trailing `_` of `current_` and
    # the leading `p` of `page` only if both sides are word chars — in
    # `current_page` both are, so the boundary doesn't exist, and the
    # match fails. `_page` similarly stays intact.
    internal_dir = INCL / "internal"
    if internal_dir.exists():
        for basic_path in internal_dir.glob("basic_*.hpp"):
            other = basic_path.stem[len("basic_"):]
            if other == name:
                continue
            # First the inheritance line — uses the qualified
            # `internal::basic_<other>` form (not just `basic_<other>`)
            # because the class is in `mpapp::internal::` itself and
            # would otherwise pick up `internal::internal::...`.
            class_block = [
                re.sub(
                    rf"(?P<lead>:\s*(?:public|private|protected)\s+)"
                    rf"{re.escape(other)}\b",
                    rf"\g<lead>internal::basic_{other}",
                    l,
                )
                for l in class_block
            ]
            # Then every remaining standalone `<other>` -> `basic_<other>`
            # (no namespace qualifier — we're already in
            # `mpapp::internal::` when the class body is parsed, so the
            # bare `basic_<other>` resolves locally). Type-only rewrite
            # so we don't accidentally rename `c.label` member accesses
            # (`label` is also the name of a property on entry_cell).
            class_block = [
                replace_type_word(l, other, f"basic_{other}")
                for l in class_block
            ]

    rebuilt: list[str] = []
    # Preamble: everything before the public namespace opens.
    rebuilt.extend(lines[: ns_open])
    if pre_class:
        rebuilt.append("namespace mpapp {")
        rebuilt.append("")
        rebuilt.extend(pre_class)
        rebuilt.append("")
        rebuilt.append("} // namespace mpapp")
        rebuilt.append("")
    rebuilt.append("namespace mpapp::internal {")
    rebuilt.append("")
    rebuilt.extend(class_block)
    rebuilt.append("")
    rebuilt.append("} // namespace mpapp::internal")
    if post_class:
        rebuilt.append("")
        rebuilt.append("namespace mpapp {")
        rebuilt.append("")
        rebuilt.extend(post_class)
        rebuilt.append("")
        rebuilt.append("} // namespace mpapp")
    rebuilt.append("")
    # Postamble: anything after the public namespace close (formatters etc.).
    rebuilt.extend(lines[ns_close + 1 :])

    out = "\n".join(rebuilt)
    if not out.endswith("\n"):
        out += "\n"
    return out


WRAPPER_TEMPLATE = '''\
// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/{Cap}.md
//
// `mpapp::{name}` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_{name}` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::{name} x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_{name} x;
//     mpapp::{handler_base}_handler<mpapp::platform::mock> h;
//     h.map_{first_map}(x);

#ifndef MPAPP_{GUARD}_HPP
#define MPAPP_{GUARD}_HPP

#include "internal/basic_{name}.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_{name}` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/{handler_base}_handler.hpp"

namespace mpapp {{

class {name} : public internal::basic_{name} {{
public:
    {name}() {{
        {setter}(embedded_handler_);
{ctor_maps}\
    }}

    {name}(const {name}&)            = delete;
    {name}& operator=(const {name}&) = delete;
    {name}({name}&&)                 = delete;
    {name}& operator=({name}&&)      = delete;

private:
    internal::{handler_base}_handler<platform::current> embedded_handler_;
}};

// Template alias so `mpapp::{handler_base}_handler<>` (host-current) and
// `mpapp::{handler_base}_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using {handler_base}_handler = internal::{handler_base}_handler<Platform>;

}} // namespace mpapp

#endif // MPAPP_{GUARD}_HPP
'''


def generate_wrapper_hpp(name: str, handler_base: str,
                         ctor_body: list[str], setter: str) -> str:
    cap = "".join(part.capitalize() for part in name.split("_") if part)
    guard = name.upper()
    ctor_maps = "".join(f"        {line}\n" for line in ctor_body)
    # Pull a hint method out of the body for the example comment.
    if ctor_body and ctor_body[0].startswith("embedded_handler_.map_"):
        first_map = ctor_body[0][len("embedded_handler_.map_"):].split("(")[0]
    else:
        first_map = "text"
    return WRAPPER_TEMPLATE.format(
        name=name,
        Cap=cap,
        GUARD=guard,
        handler_base=handler_base,
        setter=setter,
        ctor_maps=ctor_maps,
        first_map=first_map,
    )


# ---------------- Handler-header & cpp transformer -------------------------

def _migrated_components() -> list[str]:
    """List of components for which `internal/basic_<name>.hpp` exists
    on disk — i.e. components that have been migrated. Used so handler
    transforms can rewrite any standalone reference to a migrated
    sibling component to its surface form."""
    internal_dir = INCL / "internal"
    if not internal_dir.exists():
        return []
    out: list[str] = []
    for p in internal_dir.glob("basic_*.hpp"):
        out.append(p.stem[len("basic_"):])
    return out


def transform_handler_hpp(name: str, text: str) -> str:
    """In-place transformations for a per-platform / mock handler header."""
    # Include path: ../../NAME.hpp -> ../../internal/basic_NAME.hpp
    text = text.replace(
        f'#include "../../{name}.hpp"',
        f'#include "../../internal/basic_{name}.hpp"',
    )
    # Namespace.
    text = re.sub(
        r"^namespace\s+mpapp\s*\{",
        "namespace mpapp::internal {",
        text,
        flags=re.MULTILINE,
    )
    text = re.sub(
        r"^\}\s*//\s*namespace\s+mpapp\s*$",
        "} // namespace mpapp::internal",
        text,
        flags=re.MULTILINE,
    )

    # Replace every standalone `NAME` identifier with `basic_NAME`. The
    # word boundary means `NAME_handler` and `current_NAME` stay intact
    # (the connecting `_` keeps the identifier glued together).
    text = replace_word(text, name, f"basic_{name}")
    # Same for any sibling component that's already been migrated —
    # `signal<page*>` -> `signal<basic_page*>`, etc. — so handler types
    # match the surface members declared in basic_NAME.hpp. Use the
    # type-only rewrite so member-access like `c.label` (property name)
    # doesn't get rewritten to `c.basic_label`.
    for other in _migrated_components():
        if other == name:
            continue
        text = replace_type_word(text, other, f"basic_{other}")
    return text


def transform_handler_cpp(name: str, text: str) -> str:
    """In-place transformations for a per-platform handler .cpp / .mm."""
    # Namespace primary.
    text = re.sub(
        r"^namespace\s+mpapp\s*\{",
        "namespace mpapp::internal {",
        text,
        flags=re.MULTILINE,
    )
    text = re.sub(
        r"^\}\s*//\s*namespace\s+mpapp\s*$",
        "} // namespace mpapp::internal",
        text,
        flags=re.MULTILINE,
    )

    # `mpapp::detail::` -> `::mpapp::detail::` (qualified for the moved ns).
    text = re.sub(
        r"(?<![:\w])mpapp::detail::",
        "::mpapp::detail::",
        text,
    )

    # Some handler .cpp files include the original `mpapp/NAME.hpp` for
    # the dispatch_NAME function (which needs the class as a complete
    # type). Re-point that to `mpapp/internal/basic_NAME.hpp` BEFORE the
    # word-boundary rewrite, otherwise the rewrite turns the include
    # into the nonexistent `mpapp/basic_NAME.hpp`.
    text = text.replace(
        f'"mpapp/{name}.hpp"',
        f'"mpapp/internal/basic_{name}.hpp"',
    )

    # Standalone `NAME` -> `basic_NAME` (as in transform_handler_hpp).
    text = replace_word(text, name, f"basic_{name}")
    # Cascade migrated-sibling renames in TYPE positions only — `c.label`
    # accesses an Observable property called `label`, not the `label`
    # component type, so leave it alone.
    for other in _migrated_components():
        if other == name:
            continue
        text = replace_type_word(text, other, f"basic_{other}")

    # The replace above turned `mpapp::NAME*` (and `::mpapp::NAME*`) into
    # `mpapp::basic_NAME*` — but `basic_NAME` actually lives in
    # `mpapp::internal::`, so add the `internal::` qualifier in both
    # spellings (with or without the leading `::`).
    text = re.sub(
        rf"(?P<lead>::|(?<![:\w]))mpapp::basic_{re.escape(name)}(?P<tail>\b|\*)",
        rf"\g<lead>mpapp::internal::basic_{name}\g<tail>",
        text,
    )
    # Repair any include line that picked up the wrong `mpapp/basic_X.hpp`
    # path because an earlier transform pass replaced `NAME` before this
    # special-casing existed. The file lives in `mpapp/internal/`.
    text = text.replace(
        f'"mpapp/basic_{name}.hpp"',
        f'"mpapp/internal/basic_{name}.hpp"',
    )
    return text


# ---------------- Migration orchestration ----------------------------------

class ComponentMigration:
    def __init__(self, name: str):
        self.name = name
        # Handler files use the class basename — for `switch_` (whose
        # trailing underscore avoids the C++ keyword), the handler is
        # named `switch_handler.hpp` (not `switch__handler.hpp`). Strip
        # one trailing underscore for the handler filename + type name.
        self.handler_base = name.rstrip("_")
        self.component_hpp = INCL / f"{name}.hpp"
        self.internal_hpp = INCL / "internal" / f"basic_{name}.hpp"
        self.mock_hpp = INCL / "handlers" / "mock" / f"{self.handler_base}_handler.hpp"
        self.platform_hpps = {
            p: INCL / "handlers" / p / f"{self.handler_base}_handler.hpp"
            for p in PLATFORMS
        }
        self.platform_cpps: dict[str, Path] = {}
        for p in PLATFORMS:
            cpp = SRC / p / f"{self.handler_base}_handler.cpp"
            mm = SRC / p / f"{self.handler_base}_handler.mm"
            if cpp.exists():
                self.platform_cpps[p] = cpp
            elif mm.exists():
                self.platform_cpps[p] = mm

    # ---- discovery ------------------------------------------------------

    def collect_map_methods(self) -> list[str]:
        """Discover the wrapper ctor's auto-bind set. Source of truth is
        the Linux handler, because that's the real platform contract.
        The mock handler can model extra methods (e.g. `map_is_password`
        on `entry`) that real platforms don't implement yet — those must
        not enter the wrapper or the wrapper-ctor would call non-existent
        methods on the embedded Linux handler.

        Tests that want to exercise mock-only methods construct
        `internal::basic_NAME` + the mock handler directly and call
        whichever `map_X` they want."""
        order = ["linux", "windows", "android", "macos", "ios"]
        for plat in order:
            path = self.platform_hpps.get(plat)
            if path and path.exists():
                methods = discover_map_methods(read(path))
                if methods:
                    return methods
        # No real platform — fall back to mock (rare; deferred-platform
        # components).
        if self.mock_hpp.exists():
            return discover_map_methods(read(self.mock_hpp))
        return []

    def has_bind_method(self) -> bool:
        """Layout-family handlers (stack_layout, grid_layout, …) auto-
        bind via a single `bind(component&)` entry point and dispatch
        to internal `apply_X` helpers rather than the per-property
        `map_X` pattern used by simple controls. Detect this so the
        wrapper ctor calls `bind(*this)` instead of a list of map_X
        calls that don't exist on the real platform handler."""
        bind_re = re.compile(rf"\bvoid\s+bind\s*\(\s*\w+\s*&\s*\w+\s*\)")
        for plat in ["linux", "windows", "android"]:
            path = self.platform_hpps.get(plat)
            if path and path.exists() and bind_re.search(read(path)):
                return True
        return False

    # ---- generation -----------------------------------------------------

    def make_internal_hpp(self) -> None:
        if not self.component_hpp.exists():
            raise FileNotFoundError(self.component_hpp)
        original = read(self.component_hpp)
        write(self.internal_hpp, generate_basic_hpp(self.name, original))

    def make_wrapper_hpp(self) -> None:
        setter = self.detect_setter_name()
        if self.has_bind_method():
            ctor_body = ["embedded_handler_.bind(*this);"]
        else:
            maps = self.collect_map_methods()
            # `map_gestures` is added separately below as the
            # RFC-0003 gesture wire-up — drop it from the auto-
            # discovered map list so it's not emitted twice.
            maps = [m for m in maps if m != "gestures"]
            ctor_body = [f"embedded_handler_.map_{m}(*this);" for m in maps]
        # RFC-0003: walk basic_view::gesture_recognizers and install
        # the per-platform native input listeners. The call exists on
        # every per-platform + mock handler (added by sweep-gesture-
        # map.py); real implementations land per platform in
        # T-0037 / T-0038 / T-0039 / T-0040 / T-0041.
        ctor_body.append("embedded_handler_.map_gestures(*this);")
        write(self.component_hpp,
              generate_wrapper_hpp(self.name, self.handler_base,
                                   ctor_body, setter))

    def detect_setter_name(self) -> str:
        """Some components avoid the `set_handler` / `view::set_handler`
        name clash by using a 2-letter prefix: `set_sv_handler` on
        `shape_view`, `set_cv_handler` on `collection_view`, etc. Read
        `internal/basic_NAME.hpp` and grab whatever `set_..._handler`
        token is present so the wrapper ctor calls the right setter
        (otherwise the call would fall through to `view::set_handler`
        and the platform handler argument would fail to convert)."""
        if not self.internal_hpp.exists():
            return "set_handler"
        text = read(self.internal_hpp)
        # Find `void set_<X>(<NAME>_handler<...>&)`. The component-
        # specific X is what we want (so `set_sv_handler` -> use that
        # whole token).
        m = re.search(r"\b(set_\w*_handler|set_handler)\s*\(", text)
        return m.group(1) if m else "set_handler"

    def update_handlers(self) -> None:
        for path in [self.mock_hpp, *self.platform_hpps.values()]:
            if path.exists():
                write(path, transform_handler_hpp(self.name, read(path)))
        for path in self.platform_cpps.values():
            write(path, transform_handler_cpp(self.name, read(path)))

    def update_tests(self) -> None:
        """Sweep mock tests so any use of `mpapp::NAME` (the wrapper,
        which now drags in the platform handler library) gets rewritten
        to the surface `mpapp::internal::basic_NAME`. Handles variable
        declarations + template arguments (`make_unique<NAME>`, etc.).
        Idempotent.

        Bare `NAME` (without `mpapp::`) is only rewritten in files with
        `using namespace mpapp;`, and only in syntactic positions that
        prove it's the type (preceded by `<` for template args, or at
        decl-position followed by an identifier + brace/comma/semicolon).
        Property-name access like `.label` and `c.label = ...` is left
        alone — those refer to Observable members named `label`, not the
        `mpapp::label` type."""
        if not TESTS.exists():
            return
        name = self.name
        wrapper_qual = f"mpapp::{name}"
        surface_qual = f"mpapp::internal::basic_{name}"
        bare_surface = f"internal::basic_{name}"
        for test_path in TESTS.glob("*.cpp"):
            text = read(test_path)
            orig = text

            # Always: qualified `mpapp::NAME` (type position, template
            # arg position, etc.) -> `mpapp::internal::basic_NAME`.
            # Negative lookahead for `::` so we don't accidentally
            # rewrite `mpapp::NAME_handler` or `mpapp::NAME_something`.
            text = re.sub(
                rf"\bmpapp::{re.escape(name)}(?![\w:])",
                surface_qual,
                text,
            )

            if "using namespace mpapp" in text:
                # Bare `NAME` in template arg position: `<NAME>` or
                # `<NAME, ...>` or `<NAME>(`. Match `<` (template open)
                # or `,` (subsequent template arg) preceding NAME, and
                # `>` or `,` following.
                text = re.sub(
                    rf"(<|,\s*){re.escape(name)}(\s*[,>])",
                    rf"\1{bare_surface}\2",
                    text,
                )
                # Bare `NAME` in variable-declaration position: start of
                # line (after whitespace) + NAME + (optional pointer or
                # reference) + space + identifier + brace / paren /
                # comma / semicolon / equals. Covers:
                #   navigation_page nav(&home);
                #   page* popped = nav.pop();
                #   page& cur = nav.top();
                # Excludes `.NAME` member access by anchoring at line-
                # start.
                text = re.sub(
                    rf"^(\s*){re.escape(name)}(\s*[*&]?\s+\w+\s*[\{{,;(=])",
                    rf"\1{bare_surface}\2",
                    text,
                    flags=re.MULTILINE,
                )

            if text != orig:
                write(test_path, text)
                print(f"  [tests] {test_path.relative_to(ROOT)}")

    # ---- driver ---------------------------------------------------------

    def regen_wrapper(self) -> None:
        """Force-regenerate just the wrapper `NAME.hpp` from the existing
        `internal/basic_NAME.hpp`. Useful when the wrapper template
        changes (e.g. to detect a different setter name) and the
        already-migrated component needs a refreshed wrapper."""
        if not self.internal_hpp.exists():
            raise RuntimeError(f"{self.name}: internal/basic_{self.name}.hpp "
                               f"missing — run a full migrate first")
        print(f"[regen-wrapper] {self.name}")
        self.make_wrapper_hpp()

    def retransform_handlers(self) -> None:
        """Re-apply handler transforms on already-migrated components.
        Use after `transform_handler_hpp` / `transform_handler_cpp`
        evolve — the transforms are idempotent (word-boundary regexes
        skip already-rewritten tokens), so this is safe to run on a
        previously-migrated component."""
        if not self.internal_hpp.exists():
            raise RuntimeError(f"{self.name}: not migrated yet")
        print(f"[retransform-handlers] {self.name}")
        self.update_handlers()

    def migrate(self) -> None:
        if self.internal_hpp.exists():
            print(f"[skip-codegen] {self.name}: already migrated; "
                  f"running test sweep only")
            self.update_tests()
            return
        # Safety: refuse to migrate from a wrapper. If we got here with
        # `<name>.hpp` already a wrapper but `internal/basic_<name>.hpp`
        # missing (e.g. someone deleted the latter), we'd re-derive the
        # internal header from the wrapper and produce nonsense.
        if self.component_hpp.exists():
            head = read(self.component_hpp)
            if "embedded_handler_" in head:
                print(f"[abort] {self.name}: <{self.name}.hpp> looks like "
                      f"the wrapper but internal/basic_{self.name}.hpp is "
                      "missing — restore both from git and retry.")
                return
        print(f"[migrate] {self.name}")
        self.make_internal_hpp()
        self.update_handlers()       # do handlers BEFORE wrapper so the
                                     # wrapper's auto-bind sees the new
                                     # `basic_NAME&` signatures.
        self.make_wrapper_hpp()
        self.update_tests()


BUCKET_A_SIMPLE_CONTROLS = [
    "label",
    "entry",
    "switch_",
    "slider",
    "stepper",
    "check_box",
    "radio_button",
]

BUCKET_A_SIMPLE_VIEWS = [
    "activity_indicator",
    "image",
    "progress_bar",
    "date_picker",
    "time_picker",
    "picker",
    "search_bar",
    "box_view",
    "shape_view",
    "graphics_view",
    "indicator_view",
    "flyout_view",
    "refresh_view",
    "scroll_view",
    "tabbed_view",
    "toolbar",
    "title_bar",
    "collection_view",
    "list_view",
    "table_view",
    "web_view",
    "menu_bar",
    "menu_bar_item",
    "menu_flyout",
    "menu_flyout_item",
    "menu_flyout_separator",
    "menu_flyout_sub_item",
    "swipe_item_menu_item",
    "templated_view",
]

BUCKET_A_CELLS_LAYOUTS = [
    "text_cell",
    "entry_cell",
    "switch_cell",
    "view_cell",
    "image_cell",
    "stack_layout",
    "grid_layout",
    "image_button",
]

# Composites that hold child views — migrate in dependency order
# (leaves first, then bases, then the things that inherit those bases,
# then the top-level window). The page family in particular needs
# `page` migrated before its specialisations (flyout_page, tabbed_page,
# navigation_page, shell) inherit it as `internal::basic_page`.
BUCKET_C_LEAF_COMPOSITES = [
    "content_view",
    "swipe_item_view",
    "swipe_view",
]

BUCKET_C_PAGE_FAMILY = [
    "page",
    "content_page",
    "flyout_page",
    "navigation_page",
    "tabbed_page",
    "shell",
]

BUCKET_C_TOP_LEVEL = [
    "window",
]

# Edge cases — non-leaf classes or components with cross-deps.
BUCKET_D_EDGE_CASES = [
    "application",
    "border",
    "frame",
    "editor",
    "bindable_layout",
    "hybrid_web_view",
]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("names", nargs="*")
    ap.add_argument("--bucket-controls", action="store_true")
    ap.add_argument("--bucket-views", action="store_true")
    ap.add_argument("--bucket-cells-layouts", action="store_true")
    ap.add_argument("--all-bucket-a", action="store_true")
    ap.add_argument("--bucket-c-leaves", action="store_true")
    ap.add_argument("--bucket-c-pages", action="store_true")
    ap.add_argument("--bucket-c-window", action="store_true")
    ap.add_argument("--bucket-d", action="store_true")
    ap.add_argument("--regen-wrapper", action="store_true",
                    help="force-regenerate wrapper NAME.hpp from existing "
                         "internal/basic_NAME.hpp (use after the wrapper "
                         "template changes).")
    ap.add_argument("--retransform-handlers", action="store_true",
                    help="re-apply transform_handler_hpp/cpp on already-"
                         "migrated components (idempotent — safe to re-run).")
    args = ap.parse_args()

    names = list(args.names)
    if args.bucket_controls:
        names.extend(BUCKET_A_SIMPLE_CONTROLS)
    if args.bucket_views:
        names.extend(BUCKET_A_SIMPLE_VIEWS)
    if args.bucket_cells_layouts:
        names.extend(BUCKET_A_CELLS_LAYOUTS)
    if args.all_bucket_a:
        names.extend(BUCKET_A_SIMPLE_CONTROLS)
        names.extend(BUCKET_A_SIMPLE_VIEWS)
        names.extend(BUCKET_A_CELLS_LAYOUTS)
    if args.bucket_c_leaves:
        names.extend(BUCKET_C_LEAF_COMPOSITES)
    if args.bucket_c_pages:
        names.extend(BUCKET_C_PAGE_FAMILY)
    if args.bucket_c_window:
        names.extend(BUCKET_C_TOP_LEVEL)
    if args.bucket_d:
        names.extend(BUCKET_D_EDGE_CASES)

    if not names:
        ap.error("nothing to migrate — pass names or a bucket flag")

    for name in names:
        mig = ComponentMigration(name)
        if args.regen_wrapper:
            mig.regen_wrapper()
        elif args.retransform_handlers:
            mig.retransform_handlers()
        else:
            mig.migrate()
    return 0


if __name__ == "__main__":
    sys.exit(main())
