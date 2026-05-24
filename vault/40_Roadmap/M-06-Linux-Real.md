---
type: milestone
id: M-06
title: Linux real platform — GTK4 handlers via WSLg
phase: P5
status: planned
deliverables:
  - GTK4 handlers for every mocked control
  - WSLg primary dev surface validated
  - Hyper-V VM fallback documented
  - Hot reload on Linux desktop
exitCriteria:
  - "Every Controls Inventory row at mpappStatus: linux-real"
  - "platformLinux: true on every component"
  - "Native Linux build green in CI on ubuntu-latest"
tags:
  - type/milestone
  - phase/p5
  - status/planned
  - platform/linux
---

# M-06 — Linux Real Platform

> [!info] Status
> **planned**. Starts after [[M-05-Android-Real]] closes.

## Scope

GTK4 handlers. WSLg is the primary dev surface on the user's Windows host; native Linux build verified in CI.

## Exit Criteria

- [ ] Every component has a working `*_handler<platform::linux>` against GTK4.
- [ ] Every component's `platformLinux: true`.
- [ ] WSLg-based dev loop documented.
- [ ] Native Linux CI on `ubuntu-latest` green.
- [ ] Hot reload working on Linux desktop.

## Risks

> [!warning]
> - GTK4 LGPL constraints (dynamic linking only) per [[RFC-0001-licensing-and-patent-strategy]].
> - WSLg GTK4 may have edge cases vs native; document divergences.

## Tasks

Linked via [[_Bases/Tasks.base]] filtered by `milestone == "M-06"`.

## See in code

- Real GTK4 handlers: [`src/handlers/linux/`](../../src/handlers/linux/) — 62 `<component>_handler.cpp` files using GTK4's C ABI (`GtkButton`, `GtkLabel`, `GtkListBox`, `GtkDrawingArea`, etc.). LGPL dynamic linkage (Rule 9). Functionally shipped via M-04b's parallel-worker phase.
- Sample apps: [`examples/gtk4_hello/`](../../examples/gtk4_hello/), [`examples/gtk4_collectionview_layout_demo/`](../../examples/gtk4_collectionview_layout_demo/), [`examples/gtk4_shapeview_demo/`](../../examples/gtk4_shapeview_demo/).
- Cairo backend on Linux (the system libcairo bridges into [[ADR-0015-graphics-backend-dual]]'s canvas facade): [`src/detail/graphics/cairo_backend.cpp`](../../src/detail/graphics/cairo_backend.cpp).
- Linux WebKitGTK 6.x WebView via LGPL dynamic link: [`src/handlers/linux/web_view_handler.cpp`](../../src/handlers/linux/web_view_handler.cpp).
- Formal milestone closure pending (handler-status sweep + GTK4 UI smoke tests + Linux hot-reload).

## Related

- [[Platform Interop]]
- [[70_References/GTK4]]
- [[70_References/WSLg]]
- [[Hot Reload]]
- [[RFC-0001-licensing-and-patent-strategy]]
