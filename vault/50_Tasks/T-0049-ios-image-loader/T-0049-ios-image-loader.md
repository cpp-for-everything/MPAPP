---
type: task
id: T-0049
title: iOS real image_loader — UIImage + URLSession pipeline
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
  - platform/ios
---

# T-0049 — iOS real image_loader

## Goal

Land the iOS specialization of `mpapp::image_loader<platform::ios>` so a bound `image_source_ref` produces a `UIImage` consumed by `UIImageView`. After this lands, `image_handler<platform::ios>::map_source_object` swaps the mock loader for the real one.

## Per-kind wire-up table

| `image_source_kind` | UIKit pipeline |
|---|---|
| `file` | `[UIImage imageWithContentsOfFile:path]` → `UIImageView.image = image`. |
| `uri` | `NSURLSessionDataTask` with shared `NSURLSession` honouring `caching_enabled` + `cache_validity` (same pattern as macOS T-0048). Response body → `[UIImage imageWithData:data]`. |
| `stream` | Invoke factory → wrap bytes in `NSData` → `[UIImage imageWithData:data]`. |
| `font` | `NSAttributedString` rendered via `UIGraphicsImageRenderer` into a `UIImage` of the measured size. `UIFont(name:size:)`, `NSForegroundColorAttributeName: UIColor(red, green, blue, alpha)`. |
| `resource` | Resolve `resource_name` via `[UIImage imageNamed:name]` — falls back through main bundle's asset catalog automatically; honours `@2x` / `@3x` variants. Direct file path via `[NSBundle.mainBundle URLForResource]` as a secondary fallback. |

## Acceptance Criteria

- [ ] `include/mpapp/handlers/ios/image_loader.hpp` declares the specialization in Objective-C++; `load(image_source_ref) -> UIImage*`.
- [ ] `src/handlers/ios/image_loader.mm` implements each `kind()` branch.
- [ ] `image_handler<platform::ios>::map_source_object` swaps loader + routes to `UIImageView::image`.
- [ ] CI: gated on the Apple-host availability.
- [ ] Rule-11 closure: screen-recording of each kind on the iOS simulator.

## Links

- RFC: [[RFC-0004-image-source-family]].
- Mock surface: [[T-0042-image-source-family-mock]].
- Sibling per-platform tasks: T-0045 / T-0046 / T-0047 / T-0048.
