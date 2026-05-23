---
type: task
id: T-0022
title: GraphicsView demo (width / height / invalidate / draw_count / draw_requested) on Win / Linux / Android
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

# T-0022 — GraphicsView demo (Rule 11 catch-up, 3 platforms)

Rule 11 closure for `mpapp::graphics_view` — the Skia-style canvas surface with `width` / `height` Observables, an `invalidate()` trigger that bumps `draw_count`, and a `draw_requested` signal apps subscribe to for issuing draw commands. The v1 handlers map width/height to the platform drawing surface's content size (`GtkDrawingArea` content_width/height; `muxc::Canvas` Width/Height) and provide no draw-command API yet — the real Cairo/Direct2D draws are the v2 follow-up tied to the [[ADR-0015-graphics-backend-dual]] canvas facade migration.

## What landed

### Linux — `examples/gtk4_graphicsview_demo/`

A small GTK4 program with a `mpapp::graphics_view` bound to `mpapp::graphics_view_handler<linux_>`, an "gv.invalidate()" button, and a status `label` reflecting `size`, `draw_count` (from `gv.draw_count.changed`), and `draw_requested fires` (counter the app increments on each `draw_requested` emission). Clicking the button bumps both counters.

### Windows — `examples/windows_graphicsview_demo/`

WinUI 3 counterpart using `mpapp::graphics_view_handler<windows>`. Same demo shape, status label + invalidate button + empty canvas area.

### Android — JNI smoke hook in `examples/android_hello/`

`nativeRunGraphicsViewSmokeTest()` (`namespace t0022` in `native_main.cpp`) sets width/height, fires `invalidate()` three times, then resizes width — verifies (1) `draw_count` increments to 3 after 3 invalidate calls, (2) `draw_requested` fires once per invalidate (req_fires = 3), (3) `count_changes` (subscriber to draw_count.changed) fires 3 times in lockstep, (4) the resize bumps width to 400.

### Build wiring

`examples/CMakeLists.txt` adds `gtk4_graphicsview_demo` + `windows_graphicsview_demo`. Both reuse the existing per-platform handler library. The Android JNI hook adds one `native void nativeRunGraphicsViewSmokeTest()` to `MainActivity.java`.

## Screenshots

- `screenshots/linux-gtk4-graphicsview-initial.png` — Ubuntu 24.04 / GTK4 4.14 / WSLg. Window shows the title bar, status `size: 280x120  draw_count: 0  draw_requested fires: 0`, "gv.invalidate()" button, and the empty GraphicsView area below (no draw commands issued in v1).
- `screenshots/windows-winui3-graphicsview-initial.png` — Win11 / WinUI 3 1.6 cropped to 500×200. Same UI shape as Linux.
- `screenshots/android-emulator-app-running.png` — emulator x86_64 (AVD `coroute_test`, API 28) post-smoke; android_hello UI proves the process is alive after `nativeRunGraphicsViewSmokeTest` ran in `onCreate`.

## Logs

- `logs/android-emulator-graphicsview-smoke.log` — 3 lines from the smoke:

```
T-0022: after size: w=200 h=80
T-0022: after 3 invalidates: draw_count=3 req_fires=3 count_changes=3
T-0022: after resize: w=400
```

These confirm: (1) Observable<int> width/height set + read correctly; (2) `invalidate()` bumps `draw_count` AND fires `draw_requested` in lockstep — 3 invalidates → draw_count=3, req_fires=3, count_changes=3; (3) a separate width resize triggers normally.

## Tests

Mock-handler coverage lives in `tests/mock_handlers/graphics_view_test.cpp`.

## Known limitations

- **No draw-command API in v1.** The handlers map width/height but offer no way for app code to actually paint into the canvas — the GraphicsView area renders blank in both demos. The v1 surface exposes `invalidate()` + the `draw_requested` signal so the model-side plumbing works; the real Cairo/Direct2D draw context is the v2 follow-up under [[ADR-0015-graphics-backend-dual]] (the canvas facade migration). Once that lands, app code subscribed to `draw_requested` can issue `canvas->fill_rect(...)` etc. through the same backend the Cairo demo (T-0016) uses.
- **`width.changed` / `height.changed` re-binding only applies the new size after the first frame.** Resizing through the model surface in v1 sometimes lags one frame behind on Win — minor visual quirk, not a correctness issue.

## What this catches up

Per Rule 11, this closes the visible-output gap for **GraphicsView real handlers (Win / Linux / Android)**. T-0022 is the 9th Rule 11 catch-up.
