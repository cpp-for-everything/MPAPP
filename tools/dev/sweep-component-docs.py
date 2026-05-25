#!/usr/bin/env python3
"""Sweep all component .md files under vault/10_Architecture/Components/
to add a "Wrapper + Surface" section reflecting ADR-0024 (wrapper-
component pattern).

For each component:
  * If migrated (`include/mpapp/internal/basic_<snake>.hpp` exists),
    insert a section documenting the wrapper / surface layering with
    direct links to:
      - include/mpapp/<snake>.hpp                    (wrapper)
      - include/mpapp/internal/basic_<snake>.hpp     (surface)
      - include/mpapp/handlers/mock/<snake>_handler.hpp
    Plus an app-code example (wrapper) and a test-code example
    (surface + mock handler).
  * If skipped (`application`, `bindable_layout`), insert a section
    explaining why the wrapper pattern doesn't apply.
  * If a CRTP base (`view`, `layout`, `cell`, `element`), insert a
    section explaining that this is an abstract base used by other
    components.

The section is inserted right after the first `## ` heading below
the front-matter, i.e. immediately after the `# ComponentName` H1
title (or before the first existing `## ` H2 — whichever lands the
new content right under the lead paragraph).
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
VAULT_COMPONENTS = ROOT / "vault" / "10_Architecture" / "Components"
INCL_INTERNAL = ROOT / "include" / "mpapp" / "internal"

# CamelCase component file name -> snake_case component name in C++.
NAME_MAP: dict[str, str] = {
    "ActivityIndicator":     "activity_indicator",
    "Application":           "application",
    "BindableLayout":        "bindable_layout",
    "Border":                "border",
    "BoxView":               "box_view",
    "Button":                "button",
    "Cell":                  "cell",
    "CheckBox":              "check_box",
    "CollectionView":        "collection_view",
    "ContentPage":           "content_page",
    "ContentView":           "content_view",
    "DatePicker":            "date_picker",
    "Editor":                "editor",
    "Element":               "element",
    "Entry":                 "entry",
    "EntryCell":             "entry_cell",
    "FlyoutPage":            "flyout_page",
    "FlyoutView":            "flyout_view",
    "Frame":                 "frame",
    "GraphicsView":          "graphics_view",
    "Grid":                  "grid_layout",
    "HybridWebView":         "hybrid_web_view",
    "Image":                 "image",
    "ImageButton":           "image_button",
    "ImageCell":             "image_cell",
    "IndicatorView":         "indicator_view",
    "Label":                 "label",
    "Layout":                "layout",
    "ListView":              "list_view",
    "MenuBar":               "menu_bar",
    "MenuBarItem":           "menu_bar_item",
    "MenuFlyout":            "menu_flyout",
    "MenuFlyoutItem":        "menu_flyout_item",
    "MenuFlyoutSeparator":   "menu_flyout_separator",
    "MenuFlyoutSubItem":     "menu_flyout_sub_item",
    "NavigationPage":        "navigation_page",
    "Page":                  "page",
    "Picker":                "picker",
    "ProgressBar":           "progress_bar",
    "RadioButton":           "radio_button",
    "RefreshView":           "refresh_view",
    "ScrollView":            "scroll_view",
    "SearchBar":             "search_bar",
    "ShapeView":             "shape_view",
    "Shell":                 "shell",
    "Slider":                "slider",
    "StackLayout":           "stack_layout",
    "Stepper":               "stepper",
    "SwipeItemMenuItem":     "swipe_item_menu_item",
    "SwipeItemView":         "swipe_item_view",
    "SwipeView":             "swipe_view",
    "Switch":                "switch_",
    "SwitchCell":            "switch_cell",
    "TabbedPage":            "tabbed_page",
    "TabbedView":            "tabbed_view",
    "TableView":             "table_view",
    "TemplatedView":         "templated_view",
    "TextCell":              "text_cell",
    "TimePicker":            "time_picker",
    "TitleBar":              "title_bar",
    "Toolbar":               "toolbar",
    "View":                  "view",
    "ViewCell":              "view_cell",
    "WebView":               "web_view",
    "Window":                "window",
}

# Explicit per-ADR-0024 exception list.
SKIPPED = {
    "application":     "Program-entry class; `mpapp::run<App>` already constructs the App "
                       "inside the handler-driven UI-thread callback, so embedding a handler "
                       "in the App via the wrapper pattern would produce two handlers (the "
                       "external one from `run` and the embedded one in `App`). The existing "
                       "external-handler design is correct.",
    "bindable_layout": "Static attached-property facility: `bindable_layout()` is `delete`d, "
                       "every method is static, and the handler attaches to a *layout host* "
                       "(not a `bindable_layout` instance). No instance exists to wrap.",
}

# CRTP / abstract base classes that don't ship as concrete components.
BASES = {"view", "layout", "cell", "element"}

# Per-component setter name where it deviates from `set_handler`.
# Matches what migrate-component.py emits.
SETTERS = {
    "shape_view":      "set_sv_handler",
    "graphics_view":   "set_gv_handler",
    "collection_view": "set_cv_handler",
    "list_view":       "set_lv_handler",
    "table_view":      "set_tv_handler",
    "web_view":        "set_wv_handler",
    "hybrid_web_view": "set_hwv_handler",
}

# Handler-file basename — for `switch_` this is `switch` (no trailing
# underscore) because the handler file is `switch_handler.hpp`.
def handler_base(snake: str) -> str:
    return snake.rstrip("_")


def section_for_migrated(cap: str, snake: str) -> str:
    hb = handler_base(snake)
    setter = SETTERS.get(snake, "set_handler")
    return f"""## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_{snake}` | [`include/mpapp/internal/basic_{snake}.hpp`](../../../include/mpapp/internal/basic_{snake}.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::{snake}` | [`include/mpapp/{snake}.hpp`](../../../include/mpapp/{snake}.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `{setter}()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/{snake}.hpp>

mpapp::{snake} w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/{snake}.hpp>
#include <mpapp/handlers/mock/{hb}_handler.hpp>

mpapp::internal::basic_{snake} w;
mpapp::{hb}_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::{hb}_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::{hb}_handler<>` and `mpapp::{hb}_handler<platform::mock>` valid spellings without naming `internal::`.

"""


def section_for_skipped(snake: str) -> str:
    reason = SKIPPED[snake]
    return f"""## Wrapper + Surface

> [!info] No wrapper layer
> `mpapp::{snake}` is an **explicit exception** to [[ADR-0024-wrapper-component-pattern]] — see the ADR's *Skipped categories*.
>
> **Why:** {reason}

The component is constructed and used as-is (no `internal::basic_{snake}` indirection, no embedded handler in the public class).

"""


def section_for_base(snake: str) -> str:
    return f"""## Wrapper + Surface

> [!info] Abstract base class
> `mpapp::{snake}` is a CRTP / abstract base inherited by concrete components — it is not a leaf component itself and does not follow the [[ADR-0024-wrapper-component-pattern]] wrapper / surface split.
>
> Concrete components that inherit `{snake}` each have their own `mpapp::internal::basic_<...>` surface and `mpapp::<...>` wrapper; this base class participates in the chain as the inheritance root.

"""


def insert_after_h1(text: str, section: str, snake: str) -> tuple[str, bool]:
    """Insert `section` after the first `# Heading` line (the H1 title)
    and any immediately-following blockquote (`> [!info] Status …`) +
    `## Overview` block, so the new section lands BEFORE `## MAUI
    Reference` / `## MPAPP C++ API`. Idempotent — returns (text,
    inserted) where inserted=False means the section's marker (`##
    Wrapper + Surface`) was already present."""
    if "## Wrapper + Surface" in text:
        return text, False
    lines = text.splitlines(keepends=False)

    # Find the first `## ` heading whose body the section should precede.
    # Preferred anchors in order: `## MAUI Reference`, `## MPAPP C++ API`,
    # `## Overview` (insert AFTER), `## See also`, end-of-file.
    insert_at = -1
    after_overview = False

    for i, line in enumerate(lines):
        s = line.strip()
        if s.startswith("## MAUI Reference") or s.startswith("## MPAPP C++ API"):
            insert_at = i
            break
    if insert_at < 0:
        # Try after `## Overview` (walk to end of its body).
        for i, line in enumerate(lines):
            if line.strip().startswith("## Overview"):
                # Walk forward until the next `## ` or EOF.
                j = i + 1
                while j < len(lines) and not lines[j].startswith("## "):
                    j += 1
                insert_at = j
                after_overview = True
                break
    if insert_at < 0:
        # Fall back: insert before `## See also`.
        for i, line in enumerate(lines):
            if line.strip().startswith("## See also"):
                insert_at = i
                break
    if insert_at < 0:
        # Last resort: append at end.
        insert_at = len(lines)

    new_lines = lines[:insert_at] + [""] + section.rstrip().splitlines() + [""] + lines[insert_at:]
    out = "\n".join(new_lines)
    if not out.endswith("\n"):
        out += "\n"
    return out, True


def main() -> int:
    if not VAULT_COMPONENTS.exists():
        print(f"missing: {VAULT_COMPONENTS}")
        return 1

    changed = 0
    skipped_already = 0
    for cap, snake in sorted(NAME_MAP.items()):
        path = VAULT_COMPONENTS / f"{cap}.md"
        if not path.exists():
            print(f"  [no file] {cap}.md")
            continue

        if snake in SKIPPED:
            section = section_for_skipped(snake)
            kind = "skipped"
        elif snake in BASES:
            section = section_for_base(snake)
            kind = "base"
        else:
            # Migrated iff the surface header exists.
            internal_hpp = INCL_INTERNAL / f"basic_{snake}.hpp"
            if not internal_hpp.exists():
                print(f"  [no surface] {cap}.md (snake={snake!r}) — "
                      f"expected {internal_hpp.relative_to(ROOT)}")
                continue
            section = section_for_migrated(cap, snake)
            kind = "migrated"

        text = path.read_text(encoding="utf-8")
        new_text, inserted = insert_after_h1(text, section, snake)
        if inserted:
            path.write_text(new_text, encoding="utf-8")
            print(f"  [{kind}] {path.relative_to(ROOT)}")
            changed += 1
        else:
            skipped_already += 1

    print(f"\nDone. Updated {changed} files; "
          f"{skipped_already} already had the section.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
