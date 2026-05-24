---
type: task
id: T-0030
title: "ADR-0015 Skia backend (opt-in, scaffolding + impl)"
status: in-progress
milestone: M-04c-handler-heavy-port
owner: claude
area: build
blockedBy: []
coveragePercent: 0
hasScreenshots: true
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/build
  - platform/windows
  - platform/linux
  - platform/android
---

# T-0030 — ADR-0015 Skia backend

## Goal

Add Skia as an opt-in alternative to the Cairo backend for the
ADR-0015 canvas facade. Skia (BSD-3, ~30 MB linked) is the canonical
choice for cross-platform 2D rendering — once landed, it unlocks
flipping [[ADR-0015-graphics-backend-dual]] from `proposed` to
`accepted` (both backends real).

Selection stays compile-time via `MPAPP_GRAPHICS_BACKEND={cairo|skia|stub}`.
This task is the scaffolding + implementation; an actual local
install (vcpkg, ~30 min from source) and visual verification against
the existing `cairo_render_demo` is the closure step.

## Acceptance criteria (this commit)

- [x] **CMake detection.** `find_package(unofficial-skia CONFIG QUIET)` —
  vcpkg's standard pattern for Skia. When found, sets
  `MPAPP_GRAPHICS_HAS_SKIA=ON`, adds `src/detail/graphics/skia_backend.cpp`
  to `MPAPP_CORE_SOURCES`, and links `unofficial::skia::skia` (which
  carries all transitive deps: libpng, freetype, harfbuzz, expat, ...).
- [x] **Graceful fallback.** When `MPAPP_GRAPHICS_BACKEND=skia` is
  selected but Skia isn't installed, the build emits a clear warning
  with per-platform install commands and falls back to the stub
  backend (same pattern Cairo uses). Verified: `cmake ... -DMPAPP_GRAPHICS_BACKEND=skia`
  on a host without Skia configures + builds `mpapp-core` clean.
- [x] **Backend implementation** (`src/detail/graphics/skia_backend.cpp`).
  `skia_canvas` wraps an `SkBitmap` (kN32 = BGRA premultiplied on
  little-endian, matching the abstract pixel_data() format) and an
  owned `SkCanvas`. All facade methods routed to Skia primitives:
  - state (save/restore) → `SkCanvas::save/restore`
  - transforms (translate/scale/rotate) → ditto; rotate converts
    radians→degrees at the boundary (Skia takes degrees)
  - paint state stored locally; `fill_paint()` / `stroke_paint()`
    helpers rebuild an `SkPaint` per draw op
  - draw ops (fill_rect, stroke_rect, fill_ellipse, stroke_ellipse,
    fill_path, stroke_path, clear) → `SkCanvas::draw*`
  - `clip(path)` → `SkCanvas::clipPath(SkPath, doAntiAlias=true)`
  - `pixel_data()` / `pixel_stride_bytes()` → `SkBitmap::getPixels()` /
    `rowBytes()`
- [x] **Path conversion.** Facade `path` op-list → `SkPath` is 1:1
  (move / line / quad / cubic / close all have direct Skia
  equivalents). No flattening or polyline approximation needed.
- [x] **Existing builds unaffected.** Windows ctest 346/346; Linux
  ctest 351/351 (default Cairo backend). No new tests added in this
  commit — the Skia tests land alongside a verified install (see
  phase 2 below).

## Acceptance criteria (phase 2 — local install + verification)

- [x] **Install Skia.** Done via vcpkg for both `skia:x64-linux`
  (WSL Ubuntu 24.04 at `~/vcpkg/installed/x64-linux/`) and
  `skia:x64-windows` (Windows host at `C:/tools/vcpkg/installed/x64-windows/`,
  separate from the earlier `C:/Users/alext/vcpkg` install of cairo —
  see notes/dual-vcpkg-roots.md). **Android install failed** on the
  Windows host; root cause not investigated, follow-up needed.
- [x] **End-to-end build** with `-DMPAPP_GRAPHICS_BACKEND=skia`.
  Both `build-linux-skia/` (CMAKE_PREFIX_PATH at vcpkg Linux install)
  and `build-windows-skia/` (CMAKE_PREFIX_PATH at C:/tools/vcpkg)
  configure with `MPAPP_GRAPHICS_HAS_SKIA=ON`, link `mpapp-core`
  against `unofficial::skia::skia`, and run `headless_canvas_demo`
  to produce 6 PNGs each (1 graphics_view drawable + 5 shape_view
  kinds). Skia output visually matches the Cairo reference for
  every shape — same hex colors, same geometry, same stroke
  thickness, same anti-aliasing quality.
- [x] **Skia API conformance fix.** vcpkg's Skia 148 makes `SkPath`
  immutable — construction goes through `SkPathBuilder`. Patched
  `src/detail/graphics/skia_backend.cpp::to_skia_path` to build via
  `SkPathBuilder` then `.detach()` into an `SkPath`. (Same idiom
  Skia's own samples use.)
- [x] **headless_canvas_demo cairo independence.** Demo CMakeLists
  now finds libcairo independently for PNG output (via pkg-config)
  when `MPAPP_GRAPHICS_HAS_CAIRO=OFF` — so the demo works against
  any canvas backend, not just Cairo. Required for the
  Skia-backend → PNG verification.
- [x] **ADR-0015 acceptance.** Flipped from `proposed` to
  `accepted` (decisionDate 2026-05-24). The status callout
  documents Cairo as default + Skia as opt-in across platforms,
  with the Android-Skia gap explicitly noted.
- [x] **Third-party deps tracking** (Rule 9). Added rows for libcairo
  + Skia + the Skia transitive deps (harfbuzz, libpng, libjpeg-turbo,
  libwebp, expat) in [[70_References/Third-Party Dependencies]].
- [x] **Android Skia investigation.** Root-caused two stacked
  upstream tooling bugs when cross-compiling Skia → Android from a
  Windows host:
  1. **ICU autotools / msys2 path separator.** Default Skia install
     pulls ICU, which generates `uconvmsg\uconvmsg_dat.S` in its
     Makefile rules; the `\u` gets eaten as a bash escape under
     msys2, breaking the build.
  2. **Skia's GN ninja rules use POSIX backticks.** After dropping
     ICU (via `skia[core,png,jpeg]`), Skia's own build runs
     `cmd.exe /c "...llvm-ar.exe rcs libskcms.a \`cat libskcms.a.rsp\`"`
     which cmd.exe can't parse — `cat` + backticks are POSIX.
  See `notes/dual-vcpkg-roots.md` for the full investigation +
  recommended workaround (install from a Linux host with the
  Linux NDK; Linux's native POSIX shell + Unix paths sidestep
  both issues).
- [x] **Android-side Skia plumbing.** Even though the install
  failed, the MPAPP gradle + CMake wiring is in place:
  `examples/android_hello/app/build.gradle.kts` accepts
  `-PmpappGraphicsBackend=skia` (default cairo); the resulting
  CMake invocation passes `MPAPP_SKIA_PREFIX` to the
  `src/main/cpp/CMakeLists.txt` which has a new
  `elseif(MPAPP_GRAPHICS_BACKEND STREQUAL "skia")` branch that
  finds the Skia package and falls back to stub when not present.
  So once `skia:<android-triplet>` is installed (from any host)
  the swap is one gradle property away.
- [ ] **Per-backend ctest cases.** Add a small `graphics_skia_test.cpp`
  next to the existing `graphics_cairo_test.cpp`, gated on
  `#if MPAPP_GRAPHICS_HAS_SKIA`. Deferred — the headless_canvas_demo
  PNG-comparison evidence already validates the backend's output
  end-to-end; per-pixel ctest cases would be a nicer-to-have.

## Notes

### Why opt-in rather than default

Cairo is small (~1 MB), LGPL-2.1 (dynamic linking OK per RFC-0001),
and already shipping. Skia is ~30 MB and pulls in libpng, freetype,
harfbuzz, expat, zlib as transitive deps. For apps that don't need
Skia-specific features (color shaders, image filters, GPU
acceleration via Ganesh/Graphite), Cairo is the lighter default.
Apps that *do* want Skia opt in via the CMake option.

### Why vcpkg's `unofficial-skia` rather than building from source

Skia's own build system uses GN + Ninja, not CMake — integrating it
directly means dropping CMake-native dependency management. vcpkg
wraps the GN build and emits a standard CMake config package
(`unofficial-skia` — the "unofficial" prefix is vcpkg's convention
for ports where upstream doesn't ship a CMake config of its own).
Single imported target `unofficial::skia::skia` carries everything.

### Header include convention

Skia's own source tree puts headers at `include/core/SkCanvas.h` etc.,
relative to the Skia root. vcpkg preserves this layout (headers go
to `<vcpkg>/installed/<triplet>/include/skia/include/core/...`) and
sets the include path to `<...>/include/skia/`. So the include line
is `#include "include/core/SkCanvas.h"` — looks unusual but is the
upstream-Skia convention.

### What's not in this task

- ShapeView / GraphicsView migration onto the facade (T-0029 phase 2
  for the Win + Android blit paths; ShapeView migration is its own
  task). Skia and Cairo become interchangeable backends for those
  handlers once the migration is complete.
- GPU-backed rendering (Skia Ganesh / Graphite). The `skia_canvas`
  here uses CPU-backed `SkBitmap`. A `gpu_canvas` variant would be a
  separate v3-style ADR.

## Links

- ADR: [[ADR-0015-graphics-backend-dual]] (status: proposed; closes
  on this task's phase 2 plus Cairo's existing real implementation)
- Cairo backend (parallel impl): `src/detail/graphics/cairo_backend.cpp`
- Canvas facade: `include/mpapp/detail/graphics/canvas.hpp`
- vcpkg port: [Microsoft/vcpkg #29528 (skia)](https://github.com/microsoft/vcpkg/tree/master/ports/skia)
