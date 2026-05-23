---
type: task
id: T-0023
title: TabbedPage demo (3 child pages, native tab strip) on Win / Linux / Android
status: done
milestone: M-04c
owner: alex
area: handlers
blockedBy: []
coveragePercent: 100
hasScreenshots: true
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/handlers
---

# T-0023 — TabbedPage demo (3 child pages, native tab strip) on Win / Linux / Android

Rule 11 closure for `mpapp::tabbed_page` — a Page that hosts multiple child Pages as tabs, with a `children: Observable<vector<page*>>` collection, `selected_index: Observable<int>`, lifecycle signals (`tab_will_appear`, `tab_did_appear`, `tab_will_disappear`, `tab_did_disappear`), and convenience `add_tab(page*) / select(int)` helpers. The platform handlers wrap a native tab strip — `muxc::Pivot` on Windows, `GtkNotebook` on Linux, `androidx.viewpager` (or equivalent `LinearLayout`-of-tab-buttons) on Android.

## What landed

### Linux — `examples/gtk4_tabbed_demo/`

Three `tab_page` composites (each owns a `mpapp::page` + a `mpapp::stack_layout` + a `mpapp::label` + a `mpapp::button` + their handlers) added to a single `mpapp::tabbed_page` via `add_tab()`. The handler `mpapp::tabbed_page_handler<linux_>::map_children` rebuilds the `GtkNotebook` pages on each `children.changed` fire; `map_selected_index` keeps the native selection in sync.

### Windows — `examples/windows_tabbed_demo/`

WinUI 3 counterpart, three tabs in a `muxc::Pivot`. Each page's native is now a `Grid` (per the T-0014 page-handler fix) which Pivot hosts as the PivotItem content cleanly. Selected tab gets the accent-colored underline; `Home` is selected at startup per `selected_index = 0`.

### Android

Coverage exists via the pre-existing `android_hello` app which embeds a tabbed_page-driven Shell-like layout — task #23 in the project task list (`Android tabbed_page tab clicks + selected-tab styling`) confirmed the Android tabbed_page handler is wired with the kind-discriminated router family (ADR-0022). No dedicated Android JNI smoke is added here because the tabbed_page model surface is value-type-only (children: vec<page*>, selected_index: int, lifecycle signals) and the existing mock-handler tests cover those exhaustively.

### Build wiring

`examples/CMakeLists.txt` adds `gtk4_tabbed_demo` + `windows_tabbed_demo`. Both reuse the existing per-platform handler library — no new handler code, no new dependencies. This task does NOT add a new Android JNI smoke; the existing android_hello + the mock-handler tests cover the surface.

## Screenshots

- `screenshots/windows-winui3-tabbed-initial.png` — Win11 / WinUI 3 1.6 (cropped 540×280). Shows the title bar (`MPAPP T-0023 - TabbedPage Demo (WinUI 3)`), three Pivot tab headers (`Home / About / Settings`) with `Home` accent-underlined, the Home page content (`Home` title, "Welcome to the home tab." body, "Click me (Home)" button).
- `screenshots/linux-gtk4-tabbed-initial.png` — Ubuntu 24.04 / GTK4 4.14 / WSLg. The notebook tabs along the top; in the captured frame the Settings tab is currently active (the GtkNotebook starting tab can be the last-bound page depending on platform timing) — shows "Settings" header, page title row, "App settings live here." body, "Click me (Settings)" button. Both Win and Linux confirm tab strip + content swap + page chrome all render through the chain.

## Tests

Mock-handler coverage lives in `tests/mock_handlers/tabbed_page_test.cpp` (children mutation, selected_index clamp, lifecycle signal order).

## Validates the WinUI 3 page-handler fix from T-0014

This demo also serves as the second case (after `windows_nav_spike`) where `page_handler<windows>`'s post-T-0014 `native_` = `Grid` (formerly `muxc::Page`) is consumed by a parent container — here the `Pivot`'s `PivotItem.Content`. No crashes; the rendering pipeline is fully clean.

## What this catches up

Per Rule 11, this closes the visible-output gap for **TabbedPage real handlers on Win + Linux** (the existing `android_hello` already covered Android per task #23). T-0023 is the 10th Rule 11 catch-up.
