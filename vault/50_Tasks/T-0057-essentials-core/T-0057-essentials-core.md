---
type: task
id: T-0057
title: RFC-0013 Essentials core — mock surface + cross-platform verify
status: completed
milestone: M-09
owner: ""
area: process
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/process
  - phase/p2
---

# T-0057 — Essentials core

## Goal

Land the [[RFC-0013-essentials]] device-API core: `preferences`, `secure_storage`, `connectivity`, `device_info` — each an interface + in-memory mock. Establishes the interface+mock+DI-backend pattern for the whole Essentials axis.

## Scope

In: `include/mpapp/essentials/{preferences,secure_storage,connectivity,device_info}.hpp` + `tests/mock_handlers/essentials_test.cpp` (4 cases / 30 assertions).
Out (follow-ups): per-platform real backends for the 4 core APIs; the remaining Essentials APIs (geolocation, sensors, battery, clipboard, file picker, share, app info, …); async (`task<T>`) backends.

## Per-platform verification

Interfaces + in-memory mocks are platform-neutral; real backends are per-platform follow-ups.

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ ctest 444 → **448**; `[essentials]` 4 cases / 30 assertions |
| Windows MSVC | ✅ via windows-native gate |
| Android NDK r26 aarch64 | ✅ headers compile clean (`tests/android-essentials-smoke.cpp`) |
| Apple | ❌ no host (pure C++23/STL) |

## Acceptance Criteria

- [x] `preferences` typed get/set (string/long/double/bool) + default-on-miss + contains/remove/clear.
- [x] `secure_storage` set/get/remove (returns existed) / remove_all / contains.
- [x] `connectivity` access + is_online + change-signal (same-value no-op).
- [x] `device_info` value semantics + `current_device_info()` non-unknown platform/idiom.
- [x] Interface + in-memory mock per API (DI-injectable); no macros.

## Build evidence

```
$ ./build-wsl/tests/mock_handlers_test '[essentials]'  -> 30 assertions, 4 cases
$ ctest --test-dir build-wsl                           -> 448/448
$ aarch64-linux-android28-clang++ -std=c++2b -c android-essentials-smoke.cpp -> ok
```

## Links

- RFC: [[RFC-0013-essentials]]. Backends injected via [[RFC-0011-dependency-injection]].
