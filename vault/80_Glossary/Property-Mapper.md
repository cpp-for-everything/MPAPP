---
type: glossary
term: "Property Mapper"
tags:
  - type/glossary
---

# Property Mapper

A `constexpr` array of entries mapping property names to setter functions inside a handler. The MPAPP analog of MAUI's property-mapper dictionary. See [[Handlers]].

## See in code

- [`include/mpapp/handlers/mock/button_handler.hpp`](../../include/mpapp/handlers/mock/button_handler.hpp) — `map_text(button&)` is the canonical mapper signature: subscribes to the surface's `text.changed` and writes through to the handler's recording (or native) state.
- [`src/handlers/windows/button_handler.cpp`](../../src/handlers/windows/button_handler.cpp) — the real-handler version: same `map_text` shape, but the body pokes `muxc::Button::Content` instead of recording.
- [`include/mpapp/handlers/mock/`](../../include/mpapp/handlers/mock/) — every mock handler exposes one `map_<property>` per bindable surface member, plus a `map_<signal>` for events; the platform handlers mirror the same set.
