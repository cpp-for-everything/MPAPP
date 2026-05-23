---
type: task
id: T-0029
title: "GraphicsView canvas-facade migration (Linux landing + Win/Android stubs)"
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

# T-0029 — GraphicsView canvas-facade migration (phase 1)

## Goal

ADR-0015 v2 introduced a backend-agnostic 2D canvas facade with a real
Cairo backend. The `graphics_view` handler was scaffolded but its
`draw_requested` signal had no canvas-side counterpart — the user had
no way to actually issue draw ops through the facade. This task closes
that gap end-to-end on Linux and prepares the cross-platform surface
so Windows + Android can land follow-up real implementations without
touching the API.

## Acceptance criteria (phase 1 — this commit)

- [x] **Canvas facade pixel readback.** Added `pixel_data()` +
  `pixel_stride_bytes()` to the abstract `canvas` interface in
  `include/mpapp/detail/graphics/canvas.hpp`. Documented format:
  premultiplied BGRA32 little-endian (matching Cairo's `ARGB32` byte
  ordering on the platforms MPAPP targets). Backends without a CPU-
  readable surface (the stub backend) return `(nullptr, 0)` and
  handlers gate on the null check.
- [x] **Cairo backend implementation.** `cairo_canvas::pixel_data()`
  flushes the surface then forwards to `cairo_image_surface_get_data`;
  stride is `cairo_image_surface_get_stride`. Constant-correct (flush
  takes a `cairo_surface_t* const` from the const member, which Cairo
  accepts).
- [x] **Stub backend implementation.** `stub_canvas` returns
  `(nullptr, 0)` — explicit no-readback signal for handlers.
- [x] **`cairo_render_demo`'s parallel `direct_cairo_canvas`** also
  implements the new methods so it stays compile-time conforming with
  the abstract interface.
- [x] **`graphics_view` surface.** Added
  `Observable<std::function<void(canvas&)>> drawable{}` with a forward-
  declared `canvas` to keep the public header lean. Documented usage
  in the surface header.
- [x] **Mock handler.** Added `map_drawable` + `drawable_recorder` to
  `include/mpapp/handlers/mock/graphics_view_handler.hpp` so tests can
  observe the install/clear lifecycle.
- [x] **Linux real handler.** Wired against `GtkDrawingArea`:
  constructor installs the GTK draw callback once; the callback creates
  a facade canvas sized to `(width, height)`, invokes
  `gv.drawable.get()(canvas)`, then blits the canvas's pixel buffer
  through `cairo_image_surface_create_for_data` into GTK's `cairo_t*`.
  `map_draw_count` + `map_drawable` both subscribe to changes and
  trigger `gtk_widget_queue_draw` to repaint.
- [x] **Tests.** 3 new cases in `tests/mock_handlers/graphics_view_test.cpp`:
  - `drawable defaults to an empty std::function`
  - `mock handler records drawable install + clear`
  - `drawable callback receives a canvas from make_canvas` (end-to-end
    on the surface layer: a user lambda gets a real canvas and can
    issue ops + read back stride).
- [x] **Windows ctest 346/346 + Linux ctest 351/351** (both up +3
  cases from the new drawable tests).

## Acceptance criteria (phase 2 — Win + Android blit paths)

- [x] **Windows real handler.** `native_` is now `muxc::Image` (was
  the v1-placeholder `muxc::Canvas`). `WriteableBitmap` of
  (width, height) is the Image's Source; `WriteableBitmap.PixelBuffer`
  is BGRA8 premultiplied — matches the abstract `pixel_data()` format
  exactly, so the blit is a single memcpy per row (no channel swap).
  `IBufferByteAccess::Buffer` (via `<robuffer.h>`) hands us the raw
  pointer; `WriteableBitmap.Invalidate()` flips the rendered surface.
- [x] **Android real handler.** `native_` is now
  `android.widget.ImageView` (was a placeholder `android.view.View`).
  Backing pixels live in an `android.graphics.Bitmap` allocated via
  `Bitmap.createBitmap(w, h, Config.ARGB_8888)`. The NDK-level format
  is `ANDROID_BITMAP_FORMAT_RGBA_8888` (R, G, B, A byte order in
  memory) — so the blit performs a per-pixel byte swap (B ↔ R)
  between the facade's BGRA bytes and the bitmap's RGBA bytes inside
  an `AndroidBitmap_lockPixels` / `unlockPixels` pair. `jnigraphics`
  added to the Android example's `target_link_libraries`.
- [x] **Three-platform verification.** Windows ctest 346/346,
  Linux ctest 351/351, Android APK assembles clean. The behavior
  difference between Win + Android (real blit) and Linux (already
  real from phase 1) is now zero — each platform paints whatever the
  user's `drawable` callback wrote into the facade canvas.

## Acceptance criteria (phase 3 — follow-ups still open)

- [ ] **ShapeView migration.** Replaces the existing per-platform
  shape primitives (WinUI XAML `Shape`, GTK4 direct Cairo, Android
  custom view) with canvas-facade calls so all three platforms render
  identically. Different scope from this task — ShapeView shipped
  real handlers under M-04c, so the migration is a rewrite rather
  than a fresh wiring.
- [ ] **Rule 11 closure.** Per-platform screenshots showing a
  non-trivial canvas drawing rendered through the new pipeline on
  each of Win + Linux + Android. Deferred to the next user-idle
  window.

## Notes

### Design — why off-screen + blit rather than passing GTK's `cairo_t*` through the facade

The canvas facade is backend-agnostic — its abstract API can't take a
Cairo-specific handle as a parameter. Options considered:

1. **Add a platform-specific factory** (`make_cairo_canvas(cairo_t*)`):
   zero-copy Linux path but breaks portability — Windows + Android
   don't have a Cairo context to pass.
2. **Off-screen + pixel blit** (chosen): facade renders into its own
   surface; handler reads back via `pixel_data()` + blits through the
   native draw context. One extra memory copy per paint, but the same
   pattern works on every platform.

The extra copy is bounded (one `memcpy` of `width × height × 4` bytes
per repaint) and the win is interface uniformity.

### Why `drawable` is on the surface, not the handler

User code creates a `graphics_view` and assigns a draw callback. That
callback can't live on the handler because the handler is platform-
specific — the user writes one callback that works for Win + Linux +
Android. The callback's signature uses `mpapp::detail::graphics::canvas`,
which IS the platform-independent surface. Forward-declared in
`graphics_view.hpp` to avoid pulling the full graphics header into
every surface include.

## Links

- ADR: [[ADR-0015-graphics-backend-dual]]
- Component: [[10_Architecture/Components/GraphicsView]]
- Tests: `tests/mock_handlers/graphics_view_test.cpp` (3 new cases).
