---
type: task
id: T-0031
title: "ShapeView canvas-facade migration (Linux landing + shared renderer)"
status: in-progress
milestone: M-04c-handler-heavy-port
owner: claude
area: handlers
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/handlers
  - platform/linux
  - platform/windows
  - platform/android
---

# T-0031 — ShapeView canvas-facade migration (phase 1)

## Goal

ShapeView shipped real handlers under M-04c with three platform-
specific render paths: Cairo direct calls (Linux), WinUI XAML
`muxs::Shape` primitives (Windows), and an Android custom view
(`MppShapeView` with `Canvas.drawOval/drawLine/drawRect`). Each
re-implemented hex color parsing, kind dispatch, and fill-then-
stroke semantics. ADR-0015 v2 (and now T-0029) gave us a backend-
agnostic canvas facade — this task starts unifying the shape
renderer so all three platforms share one source of truth.

## Acceptance criteria (phase 1 — this commit)

- [x] **Shared shape renderer.** New header
  `include/mpapp/detail/graphics/shape_renderer.hpp` + impl
  `src/detail/graphics/shape_renderer.cpp` carry
  `render_shape_view(canvas&, const shape_view&, int w, int h)` —
  one function that:
  - Parses fill / stroke hex via the facade's `color::from_hex()`
    (replaces three near-identical per-platform parsers).
  - Honors kind / data / fill / stroke / stroke_thickness / opacity
    consistently across backends.
  - Insets the bounding box by half the stroke thickness so the
    stroke draws fully inside (w, h) — matches the legacy Linux
    behavior.
  - Falls back to the bounding rect when polygon/path data doesn't
    parse via `path::from_svg`.
  - Returns lines as a path stroke (no fill, matching MAUI
    Line.Fill = no-op).
- [x] **Linux migration.** `src/handlers/linux/shape_view_handler.cpp`
  GTK draw callback now creates a facade canvas, calls
  `render_shape_view`, and blits the BGRA32 pixels through GTK's
  `cairo_t*` via `cairo_image_surface_create_for_data` — same off-
  screen + blit pattern T-0029 uses for `graphics_view`. The
  ~100-line per-kind cairo dispatch is gone, replaced by ~15 lines
  of canvas-extract + cairo-paint.
- [x] **Build wiring.** Root `CMakeLists.txt` adds
  `shape_renderer.cpp` to `MPAPP_CORE_SOURCES`. Android example's
  `cpp/CMakeLists.txt` adds it to `MPAPP_GRAPHICS_SOURCES` so the
  helper is available once Win + Android handlers migrate.
- [x] **Three-platform verification.** Windows ctest 346/346,
  Linux ctest 351/351, Android APK assembles clean. Win + Android
  ShapeView handlers are unchanged in this phase — they still use
  XAML `Shape` / `MppShapeView` — so observable behavior on those
  platforms is bit-identical to before. Only Linux's render path
  changed.

## Acceptance criteria (phase 2 — Win + Android migrations)

- [ ] **Windows migration.** Swap `muxc::Border` + `muxs::Shape`
  for `muxc::Image` + `WriteableBitmap`, render through
  `render_shape_view`, blit (same pattern as
  `graphics_view_handler<platform::windows>::repaint`). Removes
  ~80 lines of per-kind XAML primitive setup.
- [ ] **Android migration.** Swap the custom `MppShapeView` Java
  class for an `ImageView` + `Bitmap.ARGB_8888`, render through
  `render_shape_view`, blit with the B↔R per-pixel swap (same
  pattern as `graphics_view_handler<platform::android>::repaint`).
  Delete `MppShapeView.java` once C++ stops referencing it.
- [ ] **Rule 11 closure.** Per-platform screenshot of a non-trivial
  shape rendered through the new pipeline — fundamentally needed
  for the migration since the goal is identical-across-platforms
  output, and we want to verify that's actually true.

## Notes

### Why Linux first

The Linux real handler was the easiest to migrate cleanly because
its existing draw path already terminated in a Cairo context — the
new pipeline (off-screen Cairo via facade → memcpy into GTK's Cairo
context) is more or less the same Cairo code, just routed through
the facade boundary. That validates the shared `render_shape_view`
helper against a single platform before Win + Android need it.

### Why not the existing `widget_size` Observable approach

ShapeView surfaces have no width / height Observable today; the
existing handlers all read the native widget's allocated size at
draw time (Linux: GTK provides w, h; Windows: XAML layout; Android:
`View.getWidth/getHeight`). The migration keeps that contract — the
shared `render_shape_view` is parameterized by (w, h) and each
handler decides how to source them. Adding explicit width/height
observables would be a public API extension; deferred until there's
a concrete user request.

### Shape semantics preserved across the cutover

- `rectangle` → bounding-box stroke + fill (inset by stroke/2).
- `ellipse` → same, via `fill_ellipse` / `stroke_ellipse`.
- `line` → parses 4 numbers from `data` ("x1 y1 x2 y2" with any
  separators); stroke only.
- `polygon` / `path` → `path::from_svg(data)`; falls back to bounding
  rect on empty/invalid parse. The legacy Linux v1 already fell back
  to the bounding rect, so this is a *strict improvement* — once SVG
  parses successfully you get the real shape, not a rectangle.

## Links

- ADR: [[ADR-0015-graphics-backend-dual]]
- Component: [[10_Architecture/Components/ShapeView]]
- Helper: `include/mpapp/detail/graphics/shape_renderer.hpp` (new)
- Related: [[T-0029]] (GraphicsView canvas-facade migration —
  established the off-screen + blit pattern this task reuses).
