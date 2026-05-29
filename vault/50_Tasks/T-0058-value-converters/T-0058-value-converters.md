---
type: task
id: T-0058
title: Value-converters library — completes RFC-0007 converters + StringFormat
status: completed
milestone: M-09
owner: ""
area: properties
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/properties
  - phase/p2
---

# T-0058 — Value converters

## Goal

Complete the [[RFC-0007-data-binding]] converter story: a small library of ready-made converters that plug into `binding<S,T>`'s converter slot — bool inversion, bool→visibility, and runtime `StringFormat`.

## Scope

In: `include/mpapp/binding/converters.hpp` (`invert_bool_converter`, `bool_to_visibility_converter` + `invert_bool()`/`bool_to_visibility()` helpers; `format_with<T>` / `to_string_converter<T>` gated on `<format>`) + `tests/mock_handlers/converters_test.cpp` (4 cases / 20 assertions).
Out: more converters as needed (these cover the 90% MVVM cases).

## Per-platform verification

Platform-neutral. The `<format>`-based string converters are gated for Android NDK r26 (libc++ lacks `<format>`, same guard as view.hpp); the bool/visibility converters are available everywhere.

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ ctest 451 → **455**; `[converters]` 4 cases / 20 assertions (incl. format converters) |
| Windows MSVC | ✅ via windows-native gate |
| Android NDK r26 aarch64 | ✅ bool/visibility converters compile clean (format gated out) |
| Apple | ❌ no host |

## Acceptance Criteria

- [x] `invert_bool_converter` + `invert_bool()` negate both ways.
- [x] `bool_to_visibility_converter` (collapse/hide modes) + helper.
- [x] `format_with<T>(pattern)` runtime std::vformat; `to_string_converter<T>`.
- [x] Converters plug into a `binding` converter slot (integration test).
- [x] `<format>` converters gated for Android NDK; bool converters cross-platform.

## Build evidence

```
$ ./build-wsl/tests/mock_handlers_test '[converters]'  -> 20 assertions, 4 cases
$ ctest --test-dir build-wsl                           -> 455/455
$ aarch64-linux-android28-clang++ -std=c++2b -c android-converters-smoke.cpp -> ok
```

## Links

- Completes [[RFC-0007-data-binding]]'s converter open-question.
