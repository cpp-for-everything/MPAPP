---
type: glossary
term: "Dispatcher"
tags:
  - type/glossary
---

# Dispatcher

Per-thread message queue used to marshal work onto a specific thread. `main_dispatcher()` is the UI-thread dispatcher. See [[Async Executor and Event Loops]].

## See in code

- [`include/mpapp/executor.hpp`](../../include/mpapp/executor.hpp) — declares `main_dispatcher()` + the `task<T>` / `ui_task<T>` coroutine types per [[ADR-0019-async-executor-native-dispatcher]].
- [`src/executor/mock.cpp`](../../src/executor/mock.cpp) — deterministic `test_dispatcher` used by unit tests; real per-platform sources (Win IOCP, Linux io_uring, ...) land per the M-04+ roadmap.
- [`tests/executor_test.cpp`](../../tests/executor_test.cpp) — exercises dispatch ordering + `task<T>` await semantics.
