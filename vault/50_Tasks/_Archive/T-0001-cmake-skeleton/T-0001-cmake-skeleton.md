---
type: task
id: T-0001
title: Bootstrap CMake skeleton with mpapp-core target
status: done
milestone: M-02
owner: ""
area: build
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/build
  - phase/p1
---

# T-0001 — Bootstrap CMake skeleton

## Goal

Produce a CMake 3.28+ skeleton with C++23 enabled, a `mpapp-core` library target that compiles a single source file, and a smoke test that runs successfully. This is the foundation every other phase sits on.

## Acceptance Criteria

- [ ] `CMakeLists.txt` at repo root.
- [ ] `mpapp-core` library target builds on Windows host (MSVC and Clang).
- [ ] `mpapp-core` builds with `-Wall -Wextra -Werror` (or MSVC equivalents).
- [ ] C++23 enabled (`CMAKE_CXX_STANDARD 23`).
- [ ] `include/mpapp/mpapp.hpp` umbrella header exists and is empty-but-valid.
- [ ] A trivial `tests/smoke_test.cpp` builds, links, and passes via Catch2.
- [ ] `mpapp build` (from T-0006) invokes CMake correctly for this target.
- [ ] Unit test coverage: 100% (trivial target, but the gate stays).
- [ ] Screenshots not applicable (no UI yet).
- [ ] Build logs in `logs/`.
- [ ] Test files in `tests/` linked here.
- [ ] On close: `coveragePercent: 100`, status `done`, `git mv` to `_Archive/`.

## Notes

Working notes go in `notes/` subfolder.

## Links

- Milestone: [[M-02-Infrastructure]]
- Related: [[Build System]], [[CLAUDE]] rule 11

## Closure notes

- **Closed:** 2026-05-12
- **Merged commits:** `01c5ad1` (initial implementation), `314414b` (merge into main), `211100c` (fold T-0002 spike into root `tests/CMakeLists.txt` for single-build coverage).
- **Delivered:** CMake 3.28 skeleton with C++23, `mpapp-core` static library, Catch2 v3.5.4 via FetchContent, warnings-as-errors (`/W4 /WX /permissive-` on MSVC; `-Wall -Wextra -Wpedantic -Werror` elsewhere), `include/mpapp/mpapp.hpp` umbrella header, `src/mpapp.cpp`, and a Catch2 smoke test passing under ctest.
- **Coverage:** trivial public surface — `100%`.
