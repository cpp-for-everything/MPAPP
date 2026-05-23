---
type: task
id: T-0024
title: FlyoutPage demo (master/detail with toggle) on Win / Linux / Android
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

# T-0024 — FlyoutPage demo (master/detail with toggle)

Rule 11 closure for `mpapp::flyout_page` — page-level master/detail container with two `Observable<page*>` slots (`flyout` for the pane, `detail` for the main area) and an `is_presented: Observable<bool>` toggling the flyout open/closed. Distinct from `flyout_view` (wave-2, view-level).

## What landed

### Linux — `examples/gtk4_flyout_demo/`

A small GTK4 program with:

- A `simple_page` composite (page + stack_layout + label + their handlers) used as the flyout pane (`Menu` / `(flyout pane)`).
- A `detail_page_with_button` composite (adds a "Toggle flyout" button) used as the detail pane.
- A `mpapp::flyout_page` bound via `fp_handler_.map_flyout` / `map_detail` / `map_is_presented`. Starts with `is_presented = true` so the flyout pane is visible.
- The detail's "Toggle flyout" button subscribes a callback that calls `fp_.toggle()` — flipping `is_presented` open/closed.

The Linux handler uses `GtkPaned` (or split-style container) to render the two pages side-by-side.

### Windows — `examples/windows_flyout_demo/`

WinUI 3 counterpart using `mpapp::flyout_page_handler<windows>` which wraps a `muxc::SplitView` with PanePlacement=Left, OpenPaneLength=240. The flyout pane content + detail content are each set to their corresponding page's Grid (post-T-0014 page_handler fix). `is_presented` ↔ `SplitView.IsPaneOpen`.

### Android coverage

The Android `flyout_page_handler` was completed earlier as part of M-04c's page-family port; no dedicated JNI smoke is added here because the flyout_page surface is value-type-only (two page* slots + a bool) and the existing mock-handler tests cover it exhaustively.

### Build wiring

`examples/CMakeLists.txt` adds `gtk4_flyout_demo` + `windows_flyout_demo`. Both reuse the existing per-platform handler library.

## Screenshots

- `screenshots/linux-gtk4-flyout-initial.png` — Ubuntu 24.04 / GTK4 4.14 / WSLg. Shows the master/detail split — `Menu` pane on the left containing "(flyout pane)" text, `Detail` pane on the right with title row + "Detail content here. Click the button to toggle the flyout." body + "Toggle flyout" button.
- `screenshots/windows-winui3-flyout-initial.png` — Win11 / WinUI 3 1.6 (cropped 700×280). Same SplitView layout — Menu pane on the left, Detail pane on the right, "Toggle flyout" button visible.

## Tests

Mock-handler coverage lives in `tests/mock_handlers/flyout_page_test.cpp` (flyout/detail set/clear, is_presented toggle, layout_behavior, present/dismiss/toggle helpers).

## Validates the WinUI 3 page-handler fix from T-0014 (third case)

This demo also serves as the third case (after `windows_nav_spike` and `windows_tabbed_demo`) where `page_handler<windows>`'s post-T-0014 `native_` = `Grid` is consumed by a parent container — here the `SplitView`'s `Pane` and `Content` slots. No crashes; both pages render cleanly.

## What this catches up

Per Rule 11, this closes the visible-output gap for **FlyoutPage real handlers on Win + Linux**. T-0024 is the 11th Rule 11 catch-up.
