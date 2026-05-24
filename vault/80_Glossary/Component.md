---
type: glossary
term: "Component"
tags:
  - type/glossary
---

# Component

A single MAUI-equivalent control type in MPAPP. Documented per-component in [[Components/]]. The unit of porting work.

## See in code

- [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp) — canonical component header; one `Observable<T>` member per bindable property + a `clicked` signal.
- [`include/mpapp/`](../../include/mpapp/) — one header per component (`label.hpp`, `entry.hpp`, `collection_view.hpp`, ...); see [[Controls Inventory]] for the full list + per-platform porting status.
- [`include/mpapp/handlers/mock/button_handler.hpp`](../../include/mpapp/handlers/mock/button_handler.hpp) — every component ships a mock handler first (per [[ADR-0008-mock-first-implementation]]) so the surface contract is testable before any platform-real code lands.
