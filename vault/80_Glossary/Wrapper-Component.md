---
type: glossary
term: "Wrapper Component"
tags:
  - type/glossary
---

# Wrapper Component

The user-facing class for each MPAPP control (`mpapp::button`, `mpapp::label`, `mpapp::stack_layout`, …). Inherits its platform-agnostic surface (`mpapp::internal::basic_<name>` — see [[Basic-Surface]]) publicly and embeds the platform-current handler by value. The wrapper's default constructor auto-binds the embedded handler to the surface: it sets the surface's handler pointer to the embedded value and calls every `map_<property>(*this)` on the handler, so app code never has to wire the handler manually.

App code:

```cpp
mpapp::button b;
b.text = "Save";
b.clicked.subscribe(slot, []{ /* ... */ });
```

No separate handler variable, no `set_handler()` call, no `map_<property>()` calls.

Introduced by [[ADR-0024-wrapper-component-pattern]] as the answer to the per-call-site handler-wiring boilerplate that the original separate-handler pattern required. The wrapper layer is also where any custom constructors (e.g. `navigation_page(page* root)`) forward into the surface and then run the auto-bind step.

Two component categories do **not** have a wrapper: program-entry classes (`mpapp::application` — already constructed inside `mpapp::run<App>`'s handler-driven callback) and static attached-property facilities (`mpapp::bindable_layout` — no instances). Both are documented exceptions in their per-component notes.

## See in code

- Canonical wrapper: [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp) — `class button : public internal::basic_button` with ctor that calls `set_handler(embedded_handler_); embedded_handler_.map_text(*this); embedded_handler_.map_clicked(*this);` and a private `internal::button_handler<platform::current> embedded_handler_` member.
- Generator: [`tools/dev/migrate-component.py`](../../tools/dev/migrate-component.py) — produced the wrapper for every component (excluding the two exceptions).
- Public alias: `template <class P = platform::current> using <name>_handler = internal::<name>_handler<P>;` next to every wrapper — keeps `mpapp::<name>_handler<>` and `mpapp::<name>_handler<platform::mock>` valid spellings for tests + advanced flows.

## See also

- [[Basic-Surface]] — the layer the wrapper inherits.
- [[Handler]] — the embedded handler.
- [[Component]] — the two-layer split as a whole.
- [[ADR-0024-wrapper-component-pattern]] — the design decision.
