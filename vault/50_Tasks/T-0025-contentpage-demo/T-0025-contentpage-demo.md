---
type: task
id: T-0025
title: ContentPage demo + page-handler Page→Border fix (Win + Linux)
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

# T-0025 — ContentPage demo + page-handler Page→Border fix

Rule 11 closure for `mpapp::content_page` — Page with a `title`, an `Observable<shared_ptr<view>> content`, and a `thickness padding` Observable applied to the outer chrome. Distinct from `mpapp::page` because content is a shared_ptr (for handler-allocated child management) and padding lives on the page chrome rather than the inner layout.

## Bug fix landed alongside the demo

`content_page_handler<windows>` was the last of the four Page-wrapping Windows handlers identified in the T-0014 audit (`page_handler`, `navigation_page_handler`, `content_page_handler`, `tabbed_page_handler`, `flyout_page_handler`). `tabbed_page_handler` uses `muxc::Pivot` and `flyout_page_handler` uses `muxc::SplitView`, so they were always fine. `page_handler` and `navigation_page_handler` were fixed in T-0014 (Page → Grid). `content_page_handler` was the remaining one:

- **Before**: `native_` = `muxc::Page{}` wrapping a Grid; `apply_padding(t)` wrote to `Page.Padding`.
- **After**: `native_` = `muxc::Border{}` wrapping the Grid (`native_.Child(grid_)`); `apply_padding(t)` writes to `Border.Padding` (same property name; works because Border is a Control with Padding).

The Page→Grid switch couldn't apply directly here because Grid doesn't have a Padding property in WinUI 3 — Border was the right container choice (single-child + Padding + a plain UIElement that nests anywhere). Test surface unchanged; existing mock-handler tests still pass.

## Demos

### Linux — `examples/gtk4_contentpage_demo/`

A small GTK4 program with:

- A `mpapp::content_page` set as `window.content`
- Title `"Content Page"`, padding `thickness{24.0}` (honored by the handler), content set to a shared_ptr-wrapped `stack_layout` containing a label + button
- The label says "This page is hosted in an mpapp::content_page"

### Windows — `examples/windows_contentpage_demo/`

WinUI 3 counterpart using `mpapp::content_page_handler<windows>` (the just-fixed Border-based version). Same demo shape.

### Android coverage

ContentPage is a thin model wrapper; the existing `tests/mock_handlers/content_page_test.cpp` covers title / content / padding observable behavior. No dedicated Android JNI smoke for this task — symmetry would add little value beyond the mock-handler tests.

### Build wiring

`examples/CMakeLists.txt` adds `gtk4_contentpage_demo` + `windows_contentpage_demo`. Both reuse the existing per-platform handler library — the only handler change is the `content_page_handler<windows>` Page→Border switch which compiles cleanly into `mpapp-handlers-windows`.

## Screenshots

- `screenshots/linux-gtk4-contentpage-initial.png` — Ubuntu 24.04 / GTK4 4.14 / WSLg. Shows the window with the page title at top, padded body containing the label + Click me button.
- `screenshots/windows-winui3-contentpage-initial.png` — Win11 / WinUI 3 1.6. Same UI shape rendered through `content_page_handler<windows>` (the Border-based fix).

## Validates the Page-handler audit closure

This task finishes the systematic audit of the Page-wrapping family identified in T-0014:

| Handler | Native | Status |
|---|---|---|
| `page_handler<windows>` | `muxc::Page` → `muxc::Grid` | Fixed T-0014 |
| `navigation_page_handler<windows>` | `muxc::Page` → `muxc::Grid` | Fixed T-0014 |
| `content_page_handler<windows>` | `muxc::Page` → `muxc::Border` (this task) | Fixed T-0025 |
| `tabbed_page_handler<windows>` | `muxc::Pivot` (never had bug) | OK |
| `flyout_page_handler<windows>` | `muxc::SplitView` (never had bug) | OK |

All five page-family Windows handlers now safely nest inside any UIElement parent slot. Future regressions in this area show up as either a `STATUS_APPLICATION_INTERNAL_EXCEPTION` (the symptom we tracked in T-0014's Windows Event Viewer dump) or a visible layout failure in the screenshot; both are caught by the existing demos.

## What this catches up

Per Rule 11, this closes the visible-output gap for **ContentPage real handlers on Win + Linux** AND lands the final piece of the page-handler-family audit. T-0025 is the 12th Rule 11 catch-up.
