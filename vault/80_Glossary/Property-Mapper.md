---
type: glossary
term: "Property Mapper"
tags:
  - type/glossary
---

# Property Mapper

A `constexpr` array of entries mapping property names to setter functions inside a handler. The MPAPP analog of MAUI's property-mapper dictionary. See [[Handlers]].

## See in code

- [`include/mpapp/handlers/mock/button_handler.hpp`](../../include/mpapp/handlers/mock/button_handler.hpp) — `map_text(basic_button&)` is the canonical mapper signature: subscribes to the surface's `text.changed` and writes through to the handler's recording (or native) state. Takes the [[Basic-Surface]] type (not the [[Wrapper-Component]]) so the mapper stays link-isolated from per-platform handler libraries (per [[ADR-0008-mock-first-implementation]]).
- [`src/handlers/windows/button_handler.cpp`](../../src/handlers/windows/button_handler.cpp) — the real-handler version: same `map_text(basic_button&)` shape, but the body pokes `muxc::Button::Content` instead of recording.
- [`include/mpapp/handlers/mock/`](../../include/mpapp/handlers/mock/) — every mock handler exposes one `map_<property>` per bindable surface member, plus a `map_<signal>` for events; the platform handlers mirror the same set. Layout-family handlers (stack_layout, grid_layout) instead expose a single `bind(basic_<name>&)` entry point that internally dispatches to per-property `apply_<X>` helpers.
- Auto-bind via the wrapper: [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp)'s wrapper ctor calls every `embedded_handler_.map_<property>(*this)` so app code never invokes the mapper manually. See [[ADR-0024-wrapper-component-pattern]].
