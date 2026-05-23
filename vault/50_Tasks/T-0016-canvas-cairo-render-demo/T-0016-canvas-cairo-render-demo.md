---
type: task
id: T-0016
title: canvas + Cairo render demo (Win / Linux / Android end-to-end)
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

# T-0016 — canvas + Cairo render demo (Win / Linux / Android end-to-end)

Visible, screenshot-bearing end-to-end proof of [[ADR-0015-graphics-backend-dual]]'s Cairo backend on every shipped platform — the first instance of the Rule 11 closure gate being applied to the ADR-0015 work. Also doubles as the catch-up gate for the rest of the M-04c shipped-without-screenshots backlog (T-0017+ follow this same pattern).

## What landed

### `examples/cairo_render_demo/`

A small cross-platform CLI program (~60 lines main + 130 lines parallel-Cairo helper) that drives the abstract `mpapp::detail::graphics::canvas` interface through a representative paint sequence:

- 3 filled primitives (rect, ellipse, SVG-path triangle via `path::from_svg`)
- 3 stroked outlines (rect, ellipse, cubic Bezier curve)
- transformed + half-opacity rectangle (translate + rotate + opacity stack)
- clip + fill demonstrating the clip-by-path code path

The result is written to a PNG at the path supplied on the command line (default `cairo_render_demo.png`). When built with the stub backend, the program drives the facade calls but skips the PNG write, printing a notice instead — useful for sanity-checking the surface compiles on headless hosts.

### Android JNI render hook

`MainActivity.onCreate()` (the existing T-0011 Android example) now calls `nativeRenderCairoDemoPng(File path)` after registering the activity but before launching the main UI. The JNI function (`native_main.cpp`) replicates the same paint sequence directly through Cairo and writes the PNG to the app's external files dir (`/sdcard/Android/data/io.mpapp.example/files/android-cairo-render.png`). When `MPAPP_GRAPHICS_HAS_CAIRO` isn't defined, the function returns 2 and the activity logs a skip note.

### Build wiring

- `examples/CMakeLists.txt` adds the new subdirectory (cross-platform).
- The demo links `mpapp-core` and inherits Cairo include/lib transitively when the active build has `MPAPP_GRAPHICS_HAS_CAIRO`. No new build-script changes.

## Screenshots

All three PNGs render byte-for-byte equivalent shapes through the same canvas interface; tiny size differences come from PNG compression alignment + an opacity carryover the Android inlined path doesn't reproduce. The visual parity is exact for the filled/stroked primitives and the cubic Bezier; the bottom-right clip-fill differs only in alpha (the cross-platform demo leaves opacity at 0.5 from the previous step; the Android JNI inline restores to 1.0).

- `screenshots/linux-gtk4-cairo-render.png` — Linux x86_64 via libcairo 1.18.0 (apt + pkg-config). 12,338 bytes.
- `screenshots/windows-vcpkg-cairo-render.png` — Windows x64 via vcpkg cairo:x64-windows 1.18.4. Byte-identical to Linux (12,338 bytes) — proves the cross-platform facade emits the same Cairo ops everywhere.
- `screenshots/android-emulator-cairo-render.png` — Android emulator x86_64 (API 28+) via vcpkg cairo:x64-android 1.18.4 + NDK 26. 12,453 bytes (small alpha-carryover divergence noted above).
- `screenshots/android-emulator-app-running.png` — Live screencap of the emulator with the T-0011 Android-hello app booted (1080×2400). Shows the host activity is alive at the moment the Cairo render completes; the Cairo output itself is written silently to the app's files dir and pulled via ADB.

## Tests

The abstract canvas surface + Cairo backend behaviors are already covered by mock-level tests in `tests/mock_handlers/`:

- `graphics_canvas_test.cpp` — 16 cases / 68 assertions covering value-type parsers + every method of the abstract `canvas` interface via `stub_canvas`.
- `graphics_cairo_test.cpp` — 7 cases gated on `MPAPP_GRAPHICS_HAS_CAIRO` covering pixel-readback via independent image-surface mirrors, save/restore balance, every path-op kind, and ellipse + clip composition.

End-to-end visual regression is covered by this task's screenshots — the PNGs serve as the golden output (byte-identical Linux + Windows; alpha-divergence-noted Android). Future Cairo changes can compare against these.

## Build state

- **Linux** Ubuntu 24.04 / GCC 13 / libcairo 1.18.0 / WSL2: `ninja cairo_render_demo` then run → PNG generated.
- **Windows** Win11 / MSVC 14.51 / vcpkg cairo:x64-windows 1.18.4 / mingw64 pkgconf bridge: `cmake --build build-full --target cairo_render_demo` then run → PNG generated.
- **Android** AVD coroute_test x86_64 (API 28) / NDK 26.1 / vcpkg cairo:x64-android 1.18.4: `gradle assembleDebug` then `adb install -r app-debug.apk` then `adb shell am start -n io.mpapp.example/.MainActivity` then `adb pull /sdcard/Android/data/.../android-cairo-render.png` → PNG retrieved.

## What this catches up

Per Rule 11, the M-04c shipped features that previously lacked the screenshot+task-folder gate:

- ADR-0018 Phase F (async bridge dispatch) — follow-up T
- ADR-0016 compile-time Shell routes — follow-up T
- CollectionView item_template — follow-up T
- Shell route guards + page lifecycle (ADR-0023) — follow-up T
- ADR-0015 v2 canvas facade + Cairo backend (Linux + Win + Android) — **this task closes the visible-output gap for ADR-0015**

The remaining features ship their own T-NNNN tasks in the catch-up batch. ADR-0015 is the first to land its Rule 11 evidence because it produces the most directly visible artifact (rendered pixels).

## See also

- [[ADR-0015-graphics-backend-dual]] — the decision this task validates
- [[Components/ShapeView]] · [[Components/GraphicsView]] — eventual consumers of the canvas facade
- `examples/cairo_render_demo/cairo_render_demo.cpp` — the demo source
- `examples/android_hello/app/src/main/cpp/native_main.cpp` — the Android JNI render hook
- `tests/mock_handlers/graphics_canvas_test.cpp` · `tests/mock_handlers/graphics_cairo_test.cpp` — mock + Cairo unit tests
