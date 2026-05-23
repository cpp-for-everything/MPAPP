---
type: adr
id: ADR-0015
title: "Dual 2D graphics backend (Cairo + Skia, compile-time selectable)"
status: proposed
decisionDate: 2026-05-21
deciders: []
supersedes: ""
supersededBy: ""
area: handlers
tags:
  - type/adr
  - status/proposed
  - area/handlers
---

# ADR-0015 — Dual 2D graphics backend (Cairo + Skia)

> [!info] Status
> **proposed** — unblocks ShapeView and GraphicsView real handlers, and any future custom-drawing surface (e.g. canvas-based controls, decorations, ink).

## Context

ShapeView and GraphicsView in MAUI both depend on a 2D vector backend. MAUI itself delegates to per-platform native surfaces (mux::Shapes / Android Canvas / Quartz / WebKit Canvas). MPAPP could do the same, but five different code paths multiplies the surface area we have to keep in sync, and the divergences leak into user-visible behavior (e.g. dashed-stroke offset rules differ between Direct2D and CoreGraphics).

Two cross-platform alternatives stand out:

- **Cairo** — LGPL-2.1, ~5 MB binary add, pure CPU rasterization, mature API designed for vector + image-filter work. Already present on every Linux desktop through GTK4 (zero add on our primary Linux target).
- **Skia** — BSD-3, ~30–50 MB binary add, GPU-accelerated via Vulkan / Metal / D3D11 / OpenGL ES, modern C++ API, used by Chrome / Flutter / Android / Fuchsia.

The MPAPP profile (UI framework, MAUI parity, mock-first, finite binary budget) favors the smaller dependency. But apps targeting canvas-animation or large-surface drawing workloads benefit from Skia's GPU pipeline.

## Decision

We will ship **both** backends, **selected at compile time** via the `MPAPP_GRAPHICS_BACKEND` CMake option (values: `cairo` (default) or `skia`). Only the selected backend's translation units are compiled into the resulting binary; the runtime ships with one backend, not both, so the size cost is the actual cost of the chosen library, not their sum.

The graphics surface (`shape_view_handler<platform::current>`, `graphics_view_handler<platform::current>`) presents the same C++ API regardless of backend. A thin internal facade (`mpapp::detail::graphics::canvas`) is the integration point both backends implement.

Default is **Cairo** because:
- Smaller everywhere; free on Linux through GTK4.
- LGPL-compliant via dynamic linking + a published rebuild path (RFC-0001 §Linux pattern, also adopted for WebKitGTK in ADR-TBD).
- Sufficient for ShapeView + GraphicsView's MAUI-parity surface.

Apps opt into Skia at build time by passing `-DMPAPP_GRAPHICS_BACKEND=skia` to CMake. The choice is per-build, not per-app-runtime.

## Consequences

### Positive

- Apps with modest graphics needs ship a small binary by default.
- Apps with animation-heavy or canvas-game workloads can opt into Skia without forking the framework.
- The graphics facade is the API users see; backends are an implementation detail. Migrating between them later is mechanical.
- License story is clean: Cairo route uses the dynamic-linking pattern already adopted for WebKitGTK; Skia route is plain BSD-3.

### Negative

- We maintain two backend implementations. Each new ShapeView feature (e.g. radial gradients, image filters) has to land twice. Mitigation: keep the facade narrow, defer features that aren't in the MAUI ShapeView surface.
- Apps switching backends mid-development may discover behavioral nuances (e.g. font rendering, sub-pixel alignment). Document divergences as known differences per widget.
- CI runs both backend builds on each PR for the platforms covered — modest CI cost.

### Neutral

- macOS/iOS real handlers (future) can pick either backend or use CoreGraphics natively — that decision lives in [[ADR-0005-ios-macos-separate-interop]]'s scope.

## Alternatives Considered

- **Skia only** — rejected; ~30 MB everywhere including Linux is steep for the typical app.
- **Cairo only** — rejected per the user's request to support apps that need GPU acceleration.
- **Per-platform native (Direct2D / GtkDrawingArea / Canvas / Quartz)** — rejected; multiplies divergence and code, no longer a "graphics backend" choice but a "no portable backend" choice.

## Implementation Notes

The facade landed in W21. Layout:

- `include/mpapp/detail/graphics/canvas.hpp` — value types (`color`, `point`, `size`, `rect`, `path`, `line_cap`, `line_join`) + the abstract `canvas` base class + the `make_canvas(width, height)` factory. `color::from_hex` parses `#RRGGBB`/`#RRGGBBAA`; `path::from_svg` parses the M / L / Q / C / Z subset of SVG path syntax. Both parsers are fail-quiet (return defaults on error) — the paint hot-path doesn't throw.
- `include/mpapp/detail/graphics/stub_canvas.hpp` — the default backend. Records every method call as a string for inspection. Doubles as the test fixture for code that uses the canvas surface without a real graphics dependency.
- `src/detail/graphics/canvas.cpp` — backend-independent: implementations of the value-type parsers.
- `src/detail/graphics/stub_backend.cpp` — the `make_canvas` factory for the stub backend.
- `src/detail/graphics/cairo_backend.cpp` — real Cairo implementation (in-memory `cairo_image_surface_t`). Wires every `canvas` op to its Cairo equivalent: state stack maps to `cairo_save`/`cairo_restore`, fill/stroke colors capture the latest `set_*` value and apply through `cairo_set_source_rgba` immediately before each draw, ellipse rendering uses the translate-scale-arc trick, quad Bezier ops are upconverted to Cairo's cubic form. LGPL-2.1 via dynamic link against system libcairo per RFC-0001 §Linux.
- CMake option `MPAPP_GRAPHICS_BACKEND` defaults to `cairo` on Linux (where pkg-config finds libcairo via GTK4's existing dependency) and `stub` on Windows + Android (until each bundles its own Cairo). Explicit override via `-DMPAPP_GRAPHICS_BACKEND={cairo,stub,skia}`. When set to `cairo` but Cairo isn't found, falls back to stub with a CMake warning so the build stays green. When set to `skia`, falls back to stub with a warning — Skia backend is still TBD.

Tests live in `tests/mock_handlers/graphics_canvas_test.cpp` (16 backend-agnostic cases via stub_canvas) + `tests/mock_handlers/graphics_cairo_test.cpp` (7 cases gated on `#if defined(MPAPP_GRAPHICS_HAS_CAIRO)`). Cairo tests cover make_canvas → real-render, clear / fill_rect pixel readback via an independent image-surface mirror, save/restore state-stack balance, every path-op kind without crashing, ellipse + clip composition. Linux ctest runs at 23 graphics cases / 92 assertions; Windows + Android compile the cairo test file but `#if` it out, so they run 16 cases. ctest total: 340 (Win) → 347 (Linux, with extra Cairo cases).

What's still to do for full v2:
- **Real Skia backend** — vendor Skia (~30 MB) opt-in. BSD-3 license. Same factory pattern as Cairo, different `make_canvas` body.
- **Cairo on Windows + Android** — vcpkg pull on Windows, NDK prebuilt or build-from-source on Android. The cairo_backend.cpp itself is portable C++ + Cairo C ABI; the work is purely dependency-vendoring + license-compliance plumbing.
- **ShapeView + GraphicsView migration** — those handlers currently draw with per-platform native primitives (v1). v2 routes them through `canvas` so a single code path renders on every backend. Opt-in initially (per-handler flag) so existing behavior stays intact.

## References

- [[ADR-0006-interop-parity]] — backend choice does not change observable behavior across platforms.
- [[RFC-0001-licensing-and-patent-strategy]] §Linux — LGPL pattern reused for Cairo on non-Linux.
- [[Components/ShapeView]] · [[Components/GraphicsView]]
- [[40_Roadmap/M-04c-handler-heavy-port]]
