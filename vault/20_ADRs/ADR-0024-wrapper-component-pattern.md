---
type: adr
id: ADR-0024
title: Wrapper-component pattern — auto-binding wrapper around a platform-agnostic surface
status: accepted
decisionDate: 2026-05-25
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: handlers
tags:
  - type/adr
  - status/accepted
  - area/handlers
  - area/widgets
---

# ADR-0024 — Wrapper-component pattern: auto-binding wrapper around a platform-agnostic surface

> [!success] Status
> **accepted** on 2026-05-25.

## Context

Every public component (`mpapp::button`, `mpapp::label`, `mpapp::stack_layout`, …) originally held its platform handler by **pointer** and required the user to wire the handler at every call site:

```cpp
// pre-ADR-0024 boilerplate at every use site
mpapp::button         btn{};
mpapp::button_handler<mpapp::platform::current> btn_handler{};
btn.set_handler(btn_handler);
btn_handler.map_text(btn);
btn_handler.map_clicked(btn);
btn.text = "Save";
```

Across the 14-example suite this added 5+ boilerplate lines per widget. With ~10 widgets in a non-trivial page (label, entry, button, layout, switch, …) that is 50+ lines of repetition before any view-model wiring. The boilerplate also makes XAML-codegen output harder to read.

The naïve fix — **embed the handler in the component by value** — is what user-facing code wants ergonomically, but it breaks two contracts:

1. [[ADR-0008-mock-first-implementation]] guarantees that `mock_handlers_test` can build + link against the **mock library only**, with no per-platform handler library on the link line. Embedding the platform handler in `mpapp::button` would force `mock_handlers_test` to drag in `libmpapp-handlers-linux.a` (or its Windows/Android cousin) on the host, because `button`'s ctor would constructor-call into the platform handler's symbols.
2. Some components have nested public types ([`box_view::color`](../../include/mpapp/box_view.hpp), [`shape_view::shape_kind`](../../include/mpapp/shape_view.hpp), `scroll_view::scroll_orientation`, …) whose names must stay in the public `mpapp::` namespace, and `std::formatter` specialisations that name those types. A whole-class namespace move would break those specialisations.

A third constraint comes from app-shell code: components like `navigation_page` have non-default constructors (`explicit navigation_page(page* root)`) that the wrapper must preserve, while still auto-binding the handler in the default ctor.

We need a single design that gives apps the no-boilerplate ergonomic while preserving mock-test link isolation and nested-type visibility.

## Decision

We will **split every leaf component into two layers**:

* **`mpapp::internal::basic_<name>`** — the platform-agnostic *surface*. Owns the `Observable<T>` properties + `mpapp::signal<...>` events, inherits the CRTP base (`control<basic_<name>>` / `view`), and holds the handler **by pointer**. Tests construct this directly and attach a mock handler externally; they do not link the per-platform handler library.

* **`mpapp::<name>`** — the user-facing *wrapper*. Inherits `internal::basic_<name>` publicly (so every Observable, signal, and method on the surface is visible), and **embeds an `internal::<name>_handler<platform::current>` by value**. The wrapper's constructor sets the surface's handler pointer to the embedded value and calls every `map_<property>(*this)` on it. App code reads as

  ```cpp
  mpapp::button b;
  b.text = "Save";
  b.clicked.subscribe(slot, []{ /* ... */ });
  ```

  with no separate handler variable and no `map_<property>` calls.

The accompanying public alias

```cpp
template <class Platform = platform::current>
using <name>_handler = internal::<name>_handler<Platform>;
```

keeps `mpapp::<name>_handler<>` and `mpapp::<name>_handler<platform::mock>` valid spellings for the rare advanced flows (mostly tests + a few helper functions in examples) that want to hold a handler reference directly.

The wrapper class also acts as the binding point for any **custom constructors** the surface exposes: e.g. `navigation_page(page* root)` lives on `basic_navigation_page`; the wrapper inherits it via direct call from a forwarding ctor that also runs `bind_handler()`.

### Header organisation

Each migrated component owns two headers:

| Header | Lives in | Purpose |
|---|---|---|
| `include/mpapp/internal/basic_<name>.hpp` | `mpapp::internal::` | Surface class definition. Includes only the CRTP base + observable / signal / platform headers. Does *not* include the handler umbrella. Public types (enums, formatters, `struct color`, …) stay in `mpapp::` via a surgical namespace split inside this file. |
| `include/mpapp/<name>.hpp` | `mpapp::` | Includes the surface header + `handlers/<name>_handler.hpp` (platform-current umbrella) + defines `class <name> : public internal::basic_<name>` (wrapper) + the `<name>_handler` template alias. |

The split breaks the circular include that would otherwise occur (`<name>.hpp` → umbrella → per-platform handler → `<name>.hpp` again): per-platform handler headers `#include "../../internal/basic_<name>.hpp"` instead, which never re-enters `<name>.hpp`.

### Skipped categories

Two narrow categories do **not** get a wrapper:

* **Program-entry classes** (`mpapp::application`). `mpapp::run<App>` already creates an `application_handler` externally and calls `handler.run_app<App>(argc, argv)`, which constructs the user's `App` subclass inside the platform's UI-thread callback. Embedding a handler in `application` via the wrapper pattern would produce two handlers (external + embedded). The existing external-handler design is the right one for the program-entry case.

* **Static attached-property facilities** (`mpapp::bindable_layout`). `bindable_layout()` is `delete`d; every method is static; the handler attaches to a *layout host*, not to a `bindable_layout` instance. There is no instance to wrap.

Both exceptions are explicit and documented in their component notes.

## Consequences

### Positive

- App code drops 4–5 boilerplate lines per widget. A 10-widget page goes from ~60 lines of handler wiring to ~10 lines of property assignment.
- `mock_handlers_test` stays link-isolated from per-platform handler libraries — the surface's POINTER handler holds no platform symbols.
- The template alias `mpapp::<name>_handler<>` keeps backward compatibility, so the migration is mostly **deletion** at call sites, not rewrites.
- Public nested types stay in `mpapp::` namespace; existing user code referencing `mpapp::color`, `mpapp::shape_kind`, etc. compiles unchanged.
- The wrapper inherits the surface publicly, so every `Observable`, `signal`, and surface method is visible on the wrapper without a `using` declaration.

### Negative

- Two headers per component instead of one (`<name>.hpp` + `internal/basic_<name>.hpp`).
- Per-platform handler headers now live in `namespace mpapp::internal {}`, and their `map_<property>(<name>&)` signatures became `map_<property>(basic_<name>&)`. Existing handler code was mechanically rewritten — see [[#Implementation Notes]].
- A handler header is no longer free to use `mpapp::detail::` unqualified — the namespace move means unqualified `detail::` would search `mpapp::internal::detail::` first. Call sites use `::mpapp::detail::` to disambiguate.
- Mock tests that previously instantiated `mpapp::<name>` directly (`mpapp::page p;`) now instantiate `mpapp::internal::basic_<name>` (`internal::basic_page p;`) when they need surface-only semantics. This is a one-time test sweep.

### Neutral

- Components inherit the deprecation attribute through the wrapper layer, not the surface, so `class [[deprecated]] frame : public internal::basic_frame { … }` works without warnings from framework code that holds `basic_frame`.
- The CRTP base remains parameterised on the *surface* class (`control<basic_<name>>`), not the wrapper. Wrapper-specific CRTP would add complexity without value.

## Alternatives Considered

- **Single-class with embedded handler from the start (no surface/wrapper split).** Rejected — the embedded handler would force `mock_handlers_test` to link the per-platform handler library, breaking the [[ADR-0008-mock-first-implementation]] link-isolation contract. Symptom: undefined references to `mpapp::internal::button_handler<linux_>::button_handler()` in `mock_handlers_test`. This was the first design attempted and rolled back.

- **Keep the separate-handler pattern, add an `attach()` free function for convenience.** Considered. Rejected — still requires the user to declare a handler member alongside every component, and helper functions can't preserve type information across heterogeneous widget compositions (the `bind_button(btn, handler, ...)` helpers in the routes-demo example were already this).

- **Re-export public nested types from `mpapp::` via `using` declarations after a whole-namespace move.** Considered. Rejected — `std::formatter<mpapp::color>` specialisations *also* refer to the public name; moving `color` to `mpapp::internal::color` would break those specialisations without a corresponding adjustment to every formatter, fanning the change out across the codebase. The surgical split (types stay in `mpapp::`, class moves to `mpapp::internal::`) handles this with no formatter changes.

- **`std::unique_ptr<handler>` member in the surface to defer handler construction.** Considered. Rejected — the `unique_ptr` instantiation would still require the per-platform handler's destructor symbol at the surface header's translation unit, re-introducing the link-isolation problem.

## Implementation Notes

The migration was driven by a custom code-generator and applied to 58 of 60 leaf components. The full work, including the test + example sweeps, landed in commit `4754ac1` ("refactor: migrate 58 components to wrapper pattern with internal::basic_X surface").

### Tooling

- [`tools/dev/migrate-component.py`](../../tools/dev/migrate-component.py) — the generator. For each component name it:
  - Surgically extracts the class definition from `<name>.hpp` (the brace counter strips string + char literals + `//` comments so `'{'` and `'}'` inside literals — e.g. `hybrid_web_view::process_inbound` parses `payload.front() == '{'` — do not confuse the matching).
  - Writes `internal/basic_<name>.hpp` with public types kept in `mpapp::` and the class moved to `mpapp::internal::` (renamed `<name>` → `basic_<name>`).
  - Rewrites `<name>.hpp` as the wrapper, detecting per-component setter names (`set_handler`, `set_sv_handler`, …) and binding styles (`map_<property>` vs the layout-family `bind(*this)`).
  - Updates every per-platform handler hpp/cpp/mm (`namespace mpapp` → `namespace mpapp::internal`, `<name>&` → `basic_<name>&`, dispatch-cast adjustment, `mpapp/<name>.hpp` → `mpapp/internal/basic_<name>.hpp` include path repair).
  - Cascades sibling-component renames into already-migrated handler files (`signal<page*>` → `signal<basic_page*>` after `page` is migrated).
  - Sweeps `tests/mock_handlers/*.cpp` so `mpapp::<name>` and bare `<name>` (under `using namespace mpapp`) variable declarations become `internal::basic_<name>`. `replace_type_word` excludes member-access (`.label`) and short string literals (`"label"`) so Observable property names + `record_change("label", …)` strings stay intact.
- [`tools/dev/sweep-button-wrapper.py`](../../tools/dev/sweep-button-wrapper.py) — the example sweep used during the button pilot, before the migrate script existed.

### Canonical reference component

- Surface: [`include/mpapp/internal/basic_button.hpp`](../../include/mpapp/internal/basic_button.hpp) — the smallest valid surface (one Observable + one signal + pointer-handler accessor trio).
- Wrapper: [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp) — minimal wrapper auto-binding `map_text` + `map_clicked` in its ctor and exposing the `button_handler<P>` template alias.
- Mock handler: [`include/mpapp/handlers/mock/button_handler.hpp`](../../include/mpapp/handlers/mock/button_handler.hpp) — `internal::button_handler<platform::mock>`, `map_text(basic_button&)` / `map_clicked(basic_button&)`.
- Real handler (Linux): [`src/handlers/linux/button_handler.cpp`](../../src/handlers/linux/button_handler.cpp) — same shape; `dispatch_button` casts to `::mpapp::internal::basic_button*` for the ADR-0013 widget-dispatch registry.

### Migration roster

All 58 migrated leaf components have a `vault/10_Architecture/Components/<Name>.md` entry under the "## Wrapper + surface" section pointing at their `internal/basic_<name>.hpp` and `<name>.hpp`. Five categories were handled in dependency order:

| Category | Count | Notes |
|---|---|---|
| Simple controls | 8 | label, button, entry, switch_, slider, stepper, check_box, radio_button |
| Simple views | 29 | activity_indicator, image, progress_bar, date_picker, time_picker, picker, search_bar, box_view, shape_view, graphics_view, indicator_view, flyout_view, refresh_view, scroll_view, tabbed_view, toolbar, title_bar, collection_view, list_view, table_view, web_view, menu_bar, menu_bar_item, menu_flyout, menu_flyout_item, menu_flyout_separator, menu_flyout_sub_item, swipe_item_menu_item, templated_view |
| Cells + layouts | 8 | text_cell, entry_cell, switch_cell, view_cell, image_cell, stack_layout, grid_layout, image_button |
| Composites + page family | 10 | content_view, swipe_item_view, swipe_view, page, content_page, flyout_page, navigation_page, tabbed_page, shell, window — migrated in dependency order so sibling-rename cascade picks up parents that were just migrated |
| Edge cases | 3 | border, frame (deprecated alias of border), hybrid_web_view (web_view subclass with JSON-RPC bridge), editor |

Two skipped (see [[#Decision]] § Skipped categories): `application`, `bindable_layout`.

### Build evidence

- All 351 ctest assertions pass on Linux WSL (gcc 14.2, GTK4 4.14.5), including the 305 mock-handler test cases (1207 assertions).
- The mock_handlers_test executable links only `libmpapp-core.a` + Catch2 — the per-platform handler library is *not* on its link line, proving the surface holds no platform symbols.
- `gtk4_hello`, `gtk4_async_bridge_demo`, `gtk4_routes_demo`, etc. compile + link clean on Linux WSL with the swept example sources.

## References

- [[ADR-0008-mock-first-implementation]] — the link-isolation contract the wrapper preserves.
- [[ADR-0009-public-api-template-wrappers-only]] — the `Observable<T>` / `Command<>` template-wrapper philosophy this ADR extends to entire components.
- [[ADR-0013-data-driven-widget-dispatch]] — the `dispatch_<name>` registry; this ADR rewrote every dispatch cast to `::mpapp::internal::basic_<name>*`.
- [[10_Architecture/Handlers]] — pattern overview (updated to reflect this ADR).
- [[80_Glossary/Wrapper-Component]] — term entry.
- [[80_Glossary/Basic-Surface]] — term entry.
- [[CLAUDE]] Rule 6 (mock-first) — unchanged; this ADR preserves the contract.
