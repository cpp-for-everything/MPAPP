---
type: task
id: T-0045
title: Linux real image_loader — GdkPixbuf / gdk-pixbuf-loader pipeline
status: todo
milestone: M-04
owner: ""
area: handlers
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/handlers
  - phase/p4
  - platform/linux
---

# T-0045 — Linux real image_loader

## Goal

Land the Linux specialization of `mpapp::image_loader<platform::linux_>` so a bound `image_source_ref` actually produces a renderable `GdkTexture` / `GdkPaintable` consumed by the GTK4 `GtkImage` widget. Replaces the mock loader's call-log with real bitmap creation. After this lands, `image_handler<platform::linux_>::map_source_object` can swap the mock loader for the real one.

## Per-kind wire-up table

| `image_source_kind` | GdkPixbuf pipeline |
|---|---|
| `file` | `gdk_pixbuf_new_from_file(path, &err)` → `gdk_texture_new_for_pixbuf` → `gtk_image_set_from_paintable`. |
| `uri` | `g_file_new_for_uri(uri)` → `g_file_load_bytes_async` (respects `caching_enabled` + `cache_validity` via libsoup cache, see §Cache). Decoded into `gdk_pixbuf_new_from_stream` on completion. |
| `stream` | Invoke the factory → wrap result bytes in a `GMemoryInputStream` → `gdk_pixbuf_new_from_stream`. |
| `font` | Use Pango to render the glyph at `size` in `font_family`, tint via cairo `cairo_set_source_rgba`, then `gdk_texture_new_for_pixbuf` on a `cairo_image_surface_t`. |
| `resource` | Resolve `resource_name` via the per-app GResource bundle (`g_resources_lookup_data`) — falls back to the `MPAPP_RESOURCE_ROOT` env var for dev runs that haven't generated a GResource yet. |

## Acceptance Criteria

- [ ] `include/mpapp/handlers/linux/image_loader.hpp` declares the `image_loader<platform::linux_>` specialization mirroring the mock surface (same `load(image_source_ref)` signature, returns a `GdkPaintable*`).
- [ ] `src/handlers/linux/image_loader.cpp` implements each `kind()` branch end-to-end.
- [ ] `image_handler<platform::linux_>::map_source_object(basic_image&)` swaps the mock loader for the real one and routes its output to `gtk_image_set_from_paintable`.
- [ ] CI: Linux WSL job still 100% green; an example app loads a PNG file + a stream-factory PNG end-to-end.
- [ ] Rule-11 closure: recording / screenshot of each kind rendering on a GTK4 window.

## Notes — cache

`uri_image_source.cache_validity` + `caching_enabled` map onto libsoup's `SoupCache`. Default `SOUP_CACHE_PRIVATE` keyed under `~/.cache/mpapp/uri-images/`; eviction drives off the file's `last-modified`. The RFC explicitly defers the cache-eviction policy to a follow-up RFC (Bindings/cache) — this task just wires the libsoup default + honours `caching_enabled=false` by injecting a `SOUP_REQUEST_NO_CACHE` header.

## Links

- RFC: [[RFC-0004-image-source-family]].
- Mock surface: [[T-0042-image-source-family-mock]] (closed).
- Sibling per-platform tasks: T-0046 / T-0047 / T-0048 / T-0049.
