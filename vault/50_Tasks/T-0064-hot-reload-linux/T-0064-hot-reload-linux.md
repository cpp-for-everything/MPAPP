---
type: task
id: T-0064
title: Hot-reload — Linux dlopen runtime (cross-platform parity with Windows)
status: done
milestone: M-10
owner: ""
area: tooling
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/tooling
  - phase/p2
---

# T-0064 — Hot-reload on Linux

## Goal

Deliver the goal's "Hot-reload for quick iterations during development" on Linux
(Windows already had it): a real `dlopen`-based `mpapp::hot_reload::runtime`
implementation so the same public surface watches a source, recompiles it, and
swaps the image in-process on every desktop OS — no ifdefs in the header.

## Scope

In: `src/hot_reload/linux.cpp` (mirror of windows.cpp — `system(compiler …
-shared -fPIC)` rebuild, `dlopen(RTLD_NOW|RTLD_LOCAL)` + `dlsym("compute")`
load, `dlclose` swap). Wired into `mpapp-core` (root CMake `elseif(UNIX AND NOT
APPLE)`) + `${CMAKE_DL_LIBS}`. `hot_reload_spike` example + the runtime test
un-gated for Linux. `tests/mock_handlers/hot_reload_linux_test.cpp` (2 cases /
8 assertions). Out: macOS (`.dylib`, blind — same dlopen code would apply);
state preservation across swaps (spike-level, out of scope).

## Per-platform verification

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ runtime **recompiles + dlopen-swaps live** — ctest `[hot_reload]` 8 assertions (compute 5→10, edit, poll→swap, 5→50); spike host builds. |
| Windows MSVC | ✅ pre-existing (`src/hot_reload/windows.cpp`, LoadLibraryEx); unaffected. |
| Android | n/a — dev-host tool, not a device runtime. |
| Apple | ❌ no host — the same POSIX dlopen path would serve macOS (`.dylib`). |

## Acceptance Criteria

- [x] Real Linux runtime: recompile + dlopen swap, new behavior observed.
- [x] Wired into mpapp-core + libdl; spike example builds on Linux.
- [x] ctest covers missing-source error + full rebuild/swap.

## Links

- Header: `include/mpapp/hot_reload.hpp`. Spike: `examples/hot_reload_spike`.
