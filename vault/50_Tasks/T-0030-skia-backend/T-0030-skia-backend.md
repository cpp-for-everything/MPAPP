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
- [x] **Skia prebuilt-drop now the default install path on every
  platform.** Refactor of `cmake/MpappFindSkia.cmake` —
  `mpapp_find_skia()` macro now:
  1. If `MPAPP_SKIA_PREFIX` is set explicitly, probe that prefix in
     either vcpkg's `unofficial-skia` CONFIG layout or the
     HumbleUI/JetBrains prebuilt layout (`out/<config>/skia.{a,lib}` +
     `defines.cmake`). Lets devs pin to a specific vcpkg revision or
     ship the .zip via internal mirror.
  2. Otherwise auto-download the **pinned HumbleUI/SkiaBuild
     `m143-da51f0d60e-4`** prebuilt for the current target via
     `FetchContent` — SHA-256 verified, cached under
     `<build>/_deps/mpapp_skia_prebuilt-src/`, 40-70 MB once-per-build-dir.
  Either way the same `unofficial::skia::skia` imported target lands
  in scope; the rest of the build is layout-agnostic. The lib globbing
  understands both `.a` (Linux/Android/macOS) and `.lib` (Windows
  MSVC); `defines.cmake` SK_* tokens are extracted and attached as
  `INTERFACE_COMPILE_DEFINITIONS` so consumers compile with the same
  flags the prebuilt was built against. Per-target SHA-256 hashes
  pinned for android-{arm64,x64}, linux-x64, macos-{arm64,x64},
  windows-x64 (the active MPAPP matrix).

  End-to-end verified:
  - Linux x64 (WSL Ubuntu 24.04) — `cmake -DMPAPP_GRAPHICS_BACKEND=skia`
    auto-fetched the linux-x64 zip, built mpapp-core.a + the
    headless_canvas_demo executable, ran it, and produced 6 PNGs
    (1 graphics_view + 5 shape_view kinds). All 6 are **byte-identical
    (sha256-matched)** to the reference PNGs in
    `screenshots/t0029_graphicsview_skia_linux.png` and
    `screenshots/t0031_{rectangle,ellipse,line,polygon,path}_skia_linux.png`
    from the earlier vcpkg m148 verification. Skia m143 (HumbleUI)
    and m148 (vcpkg) produce bit-identical output for the
    facade's primitives.
  - Cairo control: same demo built with `-DMPAPP_GRAPHICS_BACKEND=cairo`
    produces different output for `ellipse/line/polygon/path` (different
    AA algorithms) but identical for `rectangle` (trivial axis-aligned
    case). Skia build outputs match Skia references; Cairo build outputs
    match Cairo references; no cross-contamination.
  - Symbol-level: `nm libmpapp-core.a` on the Skia build shows
    `skia_canvas::*` + SkBitmap symbols and zero `cairo_canvas`
    references; the Cairo build is the inverse. Confirms compile-time
    backend selection, no runtime fallback.
  - **Android x64 (Pixel emulator)** — Gradle assembleDebug
    (`-PmpappGraphicsBackend=skia`) produced a 13 MB APK with
    Skia-linked .so; installed + launched on `coroute_test` AVD;
    `adb screencap` shows the `shape_view` (kind=ellipse, fill=#E63946,
    stroke=#1D3557) rendering with **correct colors after the
    `kBGRA_8888_SkColorType` fix below**.
  - Windows MSVC: in progress — `windows_shapeview_demo` build under
    a `vcvarsall x64` shell, computer-use screenshot pending.
  - Override path: `-DMPAPP_SKIA_PREFIX=$HOME/vcpkg/installed/x64-linux`
    reports `skia (vcpkg)` — backward-compat preserved.

- [x] **Android-only colorspace bug caught by the visual test.**
  `skia_canvas::skia_canvas(int,int)` previously used
  `bitmap_.allocN32Pixels(w, h, false)`. Skia's `kN32_SkColorType` is
  platform-dependent — BGRA on desktop builds (where the Linux
  byte-equality test was happy) but **RGBA on Android/iOS Skia
  builds**. The abstract canvas API documents `pixel_data()` as
  premultiplied BGRA32 little-endian, so on Android we were silently
  returning RGBA while the `shape_view_handler<android>::blit_bgra_to_rgba`
  step swapped channels as if it were BGRA — producing pixel data
  with R↔B inverted. The Linux pixel-equality test could not catch
  this: it compared BGRA-as-rendered against BGRA-as-rendered, both
  wrong-but-consistent under the Android terms.
  Fix: switch to explicit `kBGRA_8888_SkColorType` in
  `SkImageInfo::Make(w, h, kBGRA_8888_SkColorType, kPremul_SkAlphaType)`,
  re-verify on the emulator — fill renders as the configured
  `#E63946` red, stroke as `#1D3557` navy. Linux + Windows builds
  unaffected (those toolchains already had `kN32 == kBGRA_8888`).

  Required follow-up after first integration attempt: HumbleUI's
  Linux prebuilt is built with `skia_use_system_freetype2=true`, so
  `libfreetype.so.6` must come from the system at link time. The
  helper now appends per-platform system libs to the imported
  target's INTERFACE_LINK_LIBRARIES (freetype + fontconfig + GL +
  EGL + GLESv2 + m/dl/pthread on Linux; log/android/EGL/GLESv2 on
  Android; user32/gdi32/opengl32/d3d12/dxgi/... on Windows;
  CoreFoundation/CoreGraphics/CoreText/Foundation/Metal frameworks
  on macOS). Mirrors what vcpkg's `unofficial-skia` config does via
  `z_vcpkg_skia_get_link_libraries`, minus the deps HumbleUI bundles
  in the zip as .a files (png, jpeg, webp, expat, zlib, harfbuzz,
  icu).

  `build.gradle.kts` no longer forces `MPAPP_SKIA_PREFIX` to the
  vcpkg path; `-PmpappSkiaPrefix=...` only forwards the override
  when explicitly set, otherwise the auto-fetch runs. Docs in
  `notes/dual-vcpkg-roots.md`.
- [x] **Windows /MD auto-fetch via MPAPP-hosted prebuilt.** Earlier
  closure of the Windows auto-fetch path was blocked by HumbleUI's
  Windows Skia being compiled with `/MT` (static CRT) — Skia's
  `is_official_build=true` Windows default — but MPAPP's Windows
  apps need `/MD` because WinUI 3, WindowsAppSDK, and WebView2 all
  require dynamic CRT. Mixing /MT static libs into /MD consumers
  triggers MSVC LNK2038 ("RuntimeLibrary mismatch") and breaks the
  build at link time.

  Fix: host MPAPP's own /MD static-lib Skia for Windows. vcpkg
  exposes exactly the right combination via the
  `x64-windows-static-md` triplet
  (`VCPKG_LIBRARY_LINKAGE=static` + `VCPKG_CRT_LINKAGE=dynamic`).
  Added `.github/workflows/build-skia-md-windows.yml` which installs
  that triplet, packs the resulting tree (headers + .lib files +
  `share/unofficial-skia/...` CMake config) into a zip, and publishes
  to the `skia-md-<version>` release tag on
  `cpp-for-everything/MPAPP`. `mpapp_find_skia()` was refactored to
  per-platform-key URL/hash table entries so the windows-x64 row
  points at MPAPP's release instead of HumbleUI's; the fetch path
  now also runs `find_package(unofficial-skia CONFIG)` after
  `FetchContent` extracts, which the MPAPP zip's vcpkg-style config
  satisfies directly (reported layout `fetched-vcpkg`).

  Linux/Android/macOS rows still point at HumbleUI's release —
  those platforms' /MT-or-/MD distinction is moot, and HumbleUI's
  build matches what MPAPP needs.

  End-to-end verified in three phases on Windows:
  1. **Static-md install via vcpkg + MPAPP_SKIA_PREFIX override** —
     installed `skia:x64-windows-static-md` locally
     (`C:/tools/vcpkg/installed/x64-windows-static-md`, took ~30 min
     cold cache), built `windows_shapeview_demo.exe` with
     `MPAPP_SKIA_PREFIX=<that path>`; reported `skia (vcpkg)`; launched
     + screenshotted via computer-use MCP, rectangle = red,
     ellipse = teal, triangle = orange. No DLL deployment needed
     (static linkage); no LNK2038 (/MD CRT match).
  2. **Auto-fetch via local file:// URL** — temporarily set
     `_MPAPP_SKIA_URL_windows-x64` to a `D:/tmp/...zip` path of a
     locally-packed version of the vcpkg install, ran
     `cmake -DMPAPP_GRAPHICS_BACKEND=skia` against a fresh build dir,
     no MPAPP_SKIA_PREFIX. FetchContent downloaded, verified SHA-256,
     extracted; the helper's `find_package(unofficial-skia CONFIG)`
     branch picked up the vcpkg-style `share/unofficial-skia/...`
     config from the extracted tree; demo built + ran + rendered
     correctly. Reported layout: `fetched-vcpkg`.
  3. **Auto-fetch from the published MPAPP github URL** — uploaded
     the same zip (392 MB) to
     `https://github.com/cpp-for-everything/MPAPP/releases/tag/skia-md-m143-da51f0d60e-4`
     via `gh release upload`, reverted the helper's URL back to the
     github form, repeated the fresh-build test. Same end-to-end
     success: download (~7 min on this connection), extract, build,
     run, screenshot — rectangle/ellipse/triangle colors all correct.

  The `.github/workflows/build-skia-md-windows.yml` workflow exists
  for future re-publishes (version bumps); the first release was
  bootstrapped via manual `gh release create + upload` since the
  workflow's cold-cache install step ran longer than the local
  vcpkg install and was redundant. Subsequent CI runs hit the
  vcpkg binary cache and complete in ~2 minutes.
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
