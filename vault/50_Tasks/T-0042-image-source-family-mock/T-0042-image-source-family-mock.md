---
type: task
id: T-0042
title: RFC-0004 ImageSource family — mock surface + 5 concrete sources + mock loader
status: completed
milestone: M-04b
owner: ""
area: widgets
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/widgets
  - area/handlers
  - phase/p2
---

# T-0042 — ImageSource family mock surface

## Goal

Land the mock-first slice of [[RFC-0004-image-source-family]]: the polymorphic `internal::basic_image_source` base + 5 concrete sources (file / uri / stream / font / resource) + the `mpapp::image_source_ref` type alias + a recording mock `image_loader<platform::mock>`. Per-platform real loaders + the `mpapp::image::source` migration are subsequent tasks.

## Scope

In:

- `include/mpapp/internal/basic_image_source.hpp` — abstract base, `image_source_kind` enum (file/uri/stream/font/resource), `mpapp::image_source_ref` alias.
- `include/mpapp/image_sources/` — five concrete headers:
  - `file_image_source.hpp` (`Observable<std::string> file`)
  - `uri_image_source.hpp` (`Observable<std::string> uri`, `Observable<std::chrono::milliseconds> cache_validity` default 24h, `Observable<bool> caching_enabled` default true)
  - `stream_image_source.hpp` (`std::function<std::vector<std::byte>()> factory`)
  - `font_image_source.hpp` (`Observable<std::string> glyph`, `Observable<std::string> font_family`, `Observable<double> size` default 16, `Observable<color> tint`)
  - `resource_image_source.hpp` (`Observable<std::string> resource_name`)
- `include/mpapp/handlers/mock/image_loader.hpp` — `image_loader<platform::mock>::load(image_source_ref)` records `load(<kind>)=<key>` entries.
- `tests/mock_handlers/image_source_test.cpp` — 8 test cases covering each source kind + loader dispatch + null/empty handling + shared-ownership semantics.

Out (follow-up tasks):

- `mpapp::image::source` migration from `Observable<std::string>` to `Observable<image_source_ref>` — captured as T-0043.
- Per-platform real loaders (Linux GDK-Pixbuf, Windows BitmapImage, Android BitmapFactory, macOS NSImage, iOS UIImage) — captured as T-0045 through T-0049 (T-0044 was used for the RFC-0005 resource-dictionary mock surface).
- Disk cache + cache eviction policy for `uri_image_source` — needs a dedicated RFC.

## Acceptance Criteria

- [x] All 5 concrete sources derive `internal::basic_image_source`, report the correct `kind()`, and carry the bindable configuration listed above.
- [x] `image_source_ref` allows shared ownership (`use_count` > 1 when handed off to multiple consumers).
- [x] `image_loader<platform::mock>::load` dispatches on `kind()` + records one entry per call; null source records `load(null)`; empty stream factory records `load(stream:no-factory)`.
- [x] `tests/mock_handlers/image_source_test.cpp` covers each kind + the loader contract. 21 assertions, 8 test cases.
- [x] `ctest --test-dir build-wsl` is green: 370 → 378 total assertions.
- [x] Mock test target still links `libmpapp-core.a` + Catch2 only — sources add no platform-handler dependency.

## Build evidence

```
$ ctest --test-dir build-wsl
100% tests passed, 0 tests failed out of 378
Total Test time (real) =  18.23 sec

$ ./build-wsl/tests/mock_handlers_test '[image_source]'
All tests passed (21 assertions in 8 test cases)
```

## Links

- RFC: [[RFC-0004-image-source-family]].
- Sibling family-RFC precedent: [[RFC-0003-gesture-recognizers]] + [[T-0033-gesture-recognizers-tap-slice]].
- Follow-ups: T-0043 (image::source migration, done), T-0045/46/47/48/49 (per-platform real loaders) — to be opened.
- Affected components: [[Components/Image]], [[Components/ImageButton]], [[Components/ImageCell]].
