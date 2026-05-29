---
type: task
id: T-0056
title: RFC-0012 Fonts — mock surface + cross-platform verify
status: completed
milestone: M-09
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
  - phase/p2
---

# T-0056 — Fonts

## Goal

Land [[RFC-0012-fonts]]: `font` descriptor (family/size/weight/slant + builders + predicates) and `font_registry` (`add_font`/`resolve` + `configure_fonts`).

## Scope

In: `include/mpapp/fonts/{font,font_registry}.hpp` + `tests/mock_handlers/font_test.cpp` (5 cases / 29 assertions).
Out: per-platform font loader (alias→native typeface); wiring `font` into control text surfaces.

## Per-platform verification

Platform-neutral (`<string>`/`<unordered_map>`/`<optional>`).

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ ctest 439 → **444**; `[font]` 5 cases / 29 assertions |
| Windows MSVC | ✅ via windows-native gate |
| Android NDK r26 aarch64 | ✅ headers compile clean |
| Apple | ❌ no host (pure C++23/STL) |

## Acceptance Criteria

- [x] `font` builders return modified copies (original immutable); `is_bold`/`is_italic`; equality.
- [x] `font_registry` add/resolve/overwrite/miss + `count`.
- [x] `configure_fonts(registry, fn)` callback shape.
- [x] No macros; header-only; platform-neutral.

## Build evidence

```
$ ./build-wsl/tests/mock_handlers_test '[font]'   -> 29 assertions, 5 cases
$ ctest --test-dir build-wsl                      -> 444/444
$ aarch64-linux-android28-clang++ -std=c++2b -c (font headers) -> ok
```

## Links

- RFC: [[RFC-0012-fonts]]. Glyph sibling: [[RFC-0004-image-source-family]] (`font_image_source`).
