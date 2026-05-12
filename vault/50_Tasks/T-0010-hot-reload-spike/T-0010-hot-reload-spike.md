---
type: task
id: T-0010
title: LLVM-based hot reload spike on Windows desktop
status: todo
milestone: M-02
owner: ""
area: tooling
blockedBy:
  - T-0001
  - T-0002
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
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

- [ ] `examples/hot_reload_spike/host.cpp` — host program with a `Hot<MyViewModel>` instance.
- [ ] `examples/hot_reload_spike/user_code.cpp` — the "user" code being hot-reloaded.
- [ ] Filesystem watcher detects `user_code.cpp` change.
- [ ] Clang/LLD invocation rebuilds the `.dll` in < 2 seconds.
- [ ] `LoadLibrary` of the new `.dll`, function-pointer re-routing, `Observable<int>` state preservation all work.
- [ ] Screen recording of edit → save → rebuild → swap → updated UI in `recordings/`.
- [ ] 100% coverage on the hot-reload runtime code (`mpapp::Hot<T>` + watcher).

## Notes

Reference both [cr](https://github.com/fungos/cr) (MIT, header-only) and Live++ for inspiration but **do not depend on either** — implement our own to keep licensing clean per [[RFC-0001-licensing-and-patent-strategy]].

## Links

- Milestone: [[M-02-Infrastructure]]
- Related: [[Hot Reload]], [[Build System]], [[70_References/LLVM]]
