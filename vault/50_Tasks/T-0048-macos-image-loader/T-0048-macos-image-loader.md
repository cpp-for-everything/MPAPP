---
type: task
id: T-0048
title: macOS real image_loader — NSImage + URLSession pipeline
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
  - platform/macos
---

# T-0048 — macOS real image_loader

## Goal

Land the macOS specialization of `mpapp::image_loader<platform::macos>` so a bound `image_source_ref` produces an `NSImage` consumed by `NSImageView`. After this lands, `image_handler<platform::macos>::map_source_object` swaps the mock loader for the real one.

## Per-kind wire-up table

| `image_source_kind` | AppKit pipeline |
|---|---|
| `file` | `[[NSImage alloc] initWithContentsOfFile:path]` → `NSImageView.image = image`. |
| `uri` | `NSURLSessionDataTask` with a shared `NSURLSession` whose `configuration.URLCache` honours `caching_enabled` (`requestCachePolicy = .reloadIgnoringLocalCacheData` when disabled) and `cache_validity` (per-request `Cache-Control` header). Response body → `[[NSImage alloc] initWithData:data]`. |
| `stream` | Invoke the factory → wrap bytes in `NSData` → `[[NSImage alloc] initWithData:data]`. |
| `font` | `NSAttributedString` with `NSFont(name: font_family, size: size)` + `NSForegroundColorAttributeName: NSColor(red, green, blue, alpha)` → draw into an `NSImage` of the measured size via `NSImage.drawingHandler`. |
| `resource` | Resolve `resource_name` via `[NSBundle.mainBundle URLForResource:name withExtension:nil]` → file branch. Falls back to `Assets.car` asset-catalog lookup via `[NSImage imageNamed:name]`. |

## Acceptance Criteria

- [ ] `include/mpapp/handlers/macos/image_loader.hpp` declares the specialization in Objective-C++ (`.mm`-friendly); `load(image_source_ref) -> NSImage*`.
- [ ] `src/handlers/macos/image_loader.mm` implements each `kind()` branch.
- [ ] `image_handler<platform::macos>::map_source_object` swaps loader + routes to `NSImageView::image`.
- [ ] CI: gated on the Apple-host availability (no cloud-runner yet; tracked in cross-platform foundations gaps doc).
- [ ] Rule-11 closure: screenshots of each kind on a macOS window once an Apple host comes online.

## Links

- RFC: [[RFC-0004-image-source-family]].
- Mock surface: [[T-0042-image-source-family-mock]].
- Sibling per-platform tasks: T-0045 / T-0046 / T-0047 / T-0049.
