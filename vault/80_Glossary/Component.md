---
type: glossary
term: "Component"
tags:
  - type/glossary
---

# Component

A single MAUI-equivalent control type in MPAPP. Documented per-component in [[Components/]]. The unit of porting work.

Since [[ADR-0024-wrapper-component-pattern]] every leaf component is split into **two layers**:

| Layer | Class | Lives in | Header |
|---|---|---|---|
| [[Basic-Surface\|Surface]] | `mpapp::internal::basic_<name>` | `mpapp::internal::` | `include/mpapp/internal/basic_<name>.hpp` |
| [[Wrapper-Component\|Wrapper]] | `mpapp::<name>` | `mpapp::` | `include/mpapp/<name>.hpp` |

App code constructs the wrapper, which auto-binds the platform handler in its ctor. Tests construct the surface and attach a mock handler externally so the test target stays link-isolated from per-platform handler libraries.

Public nested types (`color`, `corner_radius`, `shape_kind`, …) live in `mpapp::` — only the class definition moves to `mpapp::internal::`, via a surgical namespace split inside the surface header.

Two narrow categories opt out of the wrapper layer: program-entry classes (`mpapp::application`) and static attached-property facilities (`mpapp::bindable_layout`). Both are documented exceptions in their per-component notes and in [[ADR-0024-wrapper-component-pattern]].

## See in code

- Canonical wrapper: [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp) — one `Observable<T>` per bindable property + a `clicked` signal, all visible via the public surface inheritance.
- Canonical surface: [`include/mpapp/internal/basic_button.hpp`](../../include/mpapp/internal/basic_button.hpp) — same members + a pointer-typed handler accessor trio.
- [`include/mpapp/`](../../include/mpapp/) — one wrapper header per component (`label.hpp`, `entry.hpp`, `collection_view.hpp`, …); see [[Controls Inventory]] for the full list + per-platform porting status.
- [`include/mpapp/internal/`](../../include/mpapp/internal/) — one surface header per migrated component; the directory exists because the wrapper/surface split needs a stable home for the `internal::basic_<name>` types.
- [`include/mpapp/handlers/mock/button_handler.hpp`](../../include/mpapp/handlers/mock/button_handler.hpp) — every component ships a mock handler first (per [[ADR-0008-mock-first-implementation]]) so the surface contract is testable before any platform-real code lands.

## See also

- [[Basic-Surface]] — the platform-agnostic layer.
- [[Wrapper-Component]] — the auto-binding layer.
- [[Handler]] — the platform bridge.
- [[Handlers]] — the architecture overview.
- [[Mock-Implementation]] — how tests exercise the surface.
- [[ADR-0024-wrapper-component-pattern]] — the two-layer decision.
