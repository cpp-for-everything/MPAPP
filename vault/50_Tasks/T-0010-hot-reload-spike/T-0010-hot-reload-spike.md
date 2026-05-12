---
type: task
id: T-0010
title: LLVM-based hot reload spike on Windows desktop
status: in-progress
milestone: M-02
owner: ""
area: tooling
blockedBy:
  - T-0001
  - T-0002
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/tooling
  - platform/windows
  - phase/p1
---

# T-0010 — Hot reload spike

## Goal

Prove the C++ hot-reload approach by:

1. Compiling a tiny "user code" `.cpp` into a `.dll` via Clang/LLD.
2. Loading the `.dll` into a host program via `LoadLibrary`.
3. Modifying the user code, recompiling, swapping the new `.dll` in.
4. Verifying that `Observable<int>` state survives the swap and a `Command<>` function pointer routes to the new code.

This is a desktop-only spike — emulator hot reload is a separate task in P3+.

## Acceptance Criteria

- [x] `examples/hot_reload_spike/host.cpp` — host program demonstrating the dev loop.
- [x] `examples/hot_reload_spike/user_code.cpp` — the "user" code being hot-reloaded.
- [x] mtime-poll watcher detects `user_code.cpp` change (`runtime::poll()`).
- [x] Clang/LLD invocation rebuilds the `.dll` (single `clang++ -shared` call).
- [x] `LoadLibraryEx` of the new `.dll` + `FreeLibrary` of the old + function-pointer re-acquisition.
- [ ] **Screen recording** of edit -> save -> rebuild -> swap -> updated UI in `recordings/`.
      Blocked by the worker environment — see [[notes/recording-blocker]] and
      [[notes/swap-log.txt]] for a text capture of the equivalent flow.
- [x] 100% public-API coverage on the hot-reload runtime via
      [[tests/hot_reload_test.cpp]] (full rebuild+swap test gated on
      `clang++` via the `[.clang]` Catch tag; tag-free tests cover the
      `Hot<T>` surface and the runtime's error-reporting paths and run
      in CI today).

State preservation across swaps (per the design in [[Hot Reload]]) is
intentionally out of scope for this spike — `Hot<T>` is an empty tag
base for now.

## Implementation summary

- `include/mpapp/hot_reload.hpp` — `mpapp::Hot<T>` + `mpapp::hot_reload::runtime`.
- `src/hot_reload/windows.cpp` — Windows backend using `LoadLibraryEx` /
  `FreeLibrary` + `clang++ -std=c++23 -shared` via `std::system`.
- `examples/hot_reload_spike/` — host program, user code, README,
  Windows-only CMakeLists.
- Root CMake adds `MPAPP_BUILD_EXAMPLES` option and includes the Windows
  hot-reload source in `mpapp-core` when `WIN32`.

No public-API macros — `grep -rn 'MPAPP_[A-Z_]*(' include/mpapp/`
returns no matches.

## Notes

Reference both [cr](https://github.com/fungos/cr) (MIT, header-only) and
Live++ for inspiration but **do not depend on either** — implementation
is in-house to keep licensing clean per
[[RFC-0001-licensing-and-patent-strategy]].

## Links

- Milestone: [[M-02-Infrastructure]]
- Related: [[Hot Reload]], [[Build System]], [[70_References/LLVM]]
- Tests: `vault/50_Tasks/T-0010-hot-reload-spike/tests/hot_reload_test.cpp`
- Swap log: `vault/50_Tasks/T-0010-hot-reload-spike/notes/swap-log.txt`
- Recording blocker: `vault/50_Tasks/T-0010-hot-reload-spike/notes/recording-blocker.md`
