---
type: task
id: T-0021
title: ShapeView demo (kind / data / fill / stroke / opacity) on Win / Linux / Android
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

# T-0021 — ShapeView demo on Win / Linux / Android

Rule 11 closure for `mpapp::shape_view` — the 2D vector primitive (rectangle / ellipse / line / polygon / path) with paint observables. Three platform demos render the same three-shape vertical stack (red rectangle, teal ellipse, orange path-as-bounding-box) and the Android JNI smoke validates the model-level observable signal-firings.

## What landed

### Linux — `examples/gtk4_shapeview_demo/`

A small GTK4 program with three `mpapp::shape_view` instances bound to `mpapp::shape_view_handler<linux_>`:

- `rectangle` with fill `#E63946`, stroke `#1D3557`, stroke_thickness 3.0
- `ellipse` with fill `#2A9D8F`, stroke `#1D3557`, stroke_thickness 3.0
- `path` with SVG-path data `M20 10 L60 10 L40 50 Z`, fill `#F4A261`, stroke `#1D3557`, stroke_thickness 3.0

The Linux handler renders the rectangle and ellipse correctly via Cairo through `GtkDrawingArea::draw_func`. The path falls back to a bounding-rect render in v1 (same code path as the Windows handler — see Known limitations).

### Windows — `examples/windows_shapeview_demo/`

WinUI 3 counterpart, same demo shape, using `mpapp::shape_view_handler<windows>`. The Windows handler wraps a `muxc::Border` containing a `muxs::Rectangle` / `muxs::Ellipse` / `muxs::Line` depending on kind; polygon + path fall back to `muxs::Rectangle` (same v1 limitation as Linux).

### Android — JNI smoke hook in `examples/android_hello/`

`nativeRunShapeViewSmokeTest()` (`namespace t0021` in `examples/android_hello/app/src/main/cpp/native_main.cpp`) exercises the model-level surface — kind / data / fill / stroke / stroke_thickness / opacity Observables — and confirms each one's `changed` signal fires exactly once on a real change, and zero times on a no-op set (Observable's value-equality short-circuit).

### Handler fix landed alongside the demo

Both Linux and Windows shape_view_handlers had a sizing gap: `GtkDrawingArea` and `muxc::Border` have no intrinsic size, so a `shape_view` placed in a `stack_layout` (or any layout that respects child measure) got a zero-allocation and rendered invisibly. Patched the constructors to set a sensible default content size (200×80) so the demos and any app using ShapeView without explicit width/height in v1 actually renders. Apps that want a specific size can override once the handlers honor `view.width` / `view.height`; documented as known follow-up.

### Build wiring

`examples/CMakeLists.txt` adds `gtk4_shapeview_demo` (Linux) and `windows_shapeview_demo` (Win). Both reuse the per-platform handler library. The Android JNI hook adds one `native void nativeRunShapeViewSmokeTest()` to `MainActivity.java`.

## Screenshots

- `screenshots/linux-gtk4-shapeview-initial.png` — Ubuntu 24.04 / GTK4 4.14 / WSLg. The window shows the title bar, three label/shape pairs: red rectangle with navy outline, teal ellipse with navy outline, orange "path (triangle)" panel (the SVG path falls back to bounding-rect in v1). Captured via PrintWindow(PW_RENDERFULLCONTENT).
- `screenshots/windows-winui3-shapeview-initial.png` — Win11 / WinUI 3 1.6. Same three shapes, rendered via `muxs::Rectangle` / `muxs::Ellipse` inside a `muxc::Border` host. Cropped to 1100×410.
- `screenshots/android-emulator-app-running.png` — emulator post-smoke; the `android_hello` UI proves the process is alive after `nativeRunShapeViewSmokeTest` ran in `onCreate`. The actual ShapeView evidence is the logcat artifact below.

## Logs

- `logs/android-emulator-shapeview-smoke.log` — 4 lines proving the model-level surface:

```
T-0021: after 6 sets: changes=6
T-0021: after no-change fill: changes=6
T-0021: kind=ellipse
T-0021: data=M0 0 L10 0 L5 10 Z
```

These confirm: (1) each of the 6 observable sets (kind / data / fill / stroke / stroke_thickness / opacity) fires its `changed` signal exactly once, total = 6; (2) a no-op set (assigning the same fill string) doesn't fire — `changes` stays at 6, proving Observable's value-equality short-circuit; (3) kind reads back as `ellipse` after the assignment; (4) data reads back as the SVG path string.

## Tests

Mock-handler coverage lives in `tests/mock_handlers/shape_view_test.cpp`.

## Build state

- **Linux** Ubuntu 24.04 / GCC 13 / GTK4 4.14 / WSL2: `cmake --build build-linux --target gtk4_shapeview_demo`.
- **Windows** Win11 / MSVC 14.51 / WinUI 3 1.6: `cmake --build build-full --target windows_shapeview_demo`.
- **Android** AVD `coroute_test` x86_64 (API 28): APK installed via `adb install -r`; logcat captured via `adb logcat -d -s MPAPP:* | grep T-0021`.

## Known limitations

- **Path / polygon are bounding-rect renders in v1.** Both `shape_view_handler<windows>` and `shape_view_handler<linux_>` fall back to `Rectangle` / a Cairo bounding-rect path for `shape_kind::polygon` and `shape_kind::path`. Full SVG-path rendering with curve operators is a v2 follow-up — likely landing as part of the [[ADR-0015-graphics-backend-dual]] migration onto the canvas facade (which already has full SVG-path support via `path::from_svg`).
- **view.width / view.height are not consumed by the handler.** The constructor sets a fixed 200×80 default; apps that need a different size for a specific shape can't set it through the public surface yet. Same v2 follow-up — handler should subscribe to view.width.changed / view.height.changed and re-apply.

## What this catches up

Per Rule 11, this closes the visible-output gap for **ShapeView real handlers (Win / Linux / Android)** — the previously-shipped feature without a dedicated demo.

T-0021 is the 8th Rule 11 catch-up.
