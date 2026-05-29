---
type: task
id: T-0055
title: RFC-0011 Dependency injection — mock surface + cross-platform verify
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

# T-0055 — Dependency injection

## Goal

Land [[RFC-0011-dependency-injection]]: `service_collection`, `service_provider`, `app_builder` — singleton/transient lifetimes, factory-based constructor injection, interface→impl, pre-built instances.

## Scope

In: `include/mpapp/di/{service_collection,app_builder}.hpp` + `tests/mock_handlers/di_test.cpp` (7 cases / 18 assertions).
Out: `mpapp::run<App>` provider wiring; scoped lifetime; keyed services.

## Per-platform verification

Platform-neutral (pure `<functional>`/`<memory>`/`<typeindex>`).

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ ctest 432 → **439**; `[di]` 7 cases / 18 assertions |
| Windows MSVC | ✅ via windows-native gate |
| Android NDK r26 aarch64 | ✅ compiles clean (`tests/android-di-smoke.cpp`) |
| Apple | ❌ no host (pure C++23/STL) |

## Acceptance Criteria

- [x] Singleton identity + shared state; transient distinctness.
- [x] Interface→impl resolves the impl.
- [x] Factory registration injects resolved dependencies (singleton identity preserved).
- [x] Pre-built instance returned as-is.
- [x] Missing registration: `get` null, `contains` false, `get_required` throws.
- [x] `app_builder.services()` + `build()`.
- [x] No macros; header-only; platform-neutral.

## Build evidence

```
$ ./build-wsl/tests/mock_handlers_test '[di]'   -> 18 assertions, 7 cases
$ ctest --test-dir build-wsl                    -> 439/439
$ aarch64-linux-android28-clang++ -std=c++2b -c android-di-smoke.cpp -> ok
```

## Links

- RFC: [[RFC-0011-dependency-injection]]. Will wire into [[ADR-0012-application-window-handler-abstraction]]'s run<App>.
