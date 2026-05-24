---
type: glossary
term: "Executor"
tags:
  - type/glossary
---

# Executor

MPAPP's coroutine-aware thread pool, integrated with the platform's native event loop. See [[Async Executor and Event Loops]].

## See in code

- [`include/mpapp/executor.hpp`](../../include/mpapp/executor.hpp) — `task<T>`, `ui_task<T>`, dispatcher handles. The public surface per [[ADR-0019-async-executor-native-dispatcher]].
- [`src/executor/mock.cpp`](../../src/executor/mock.cpp) — deterministic backing for `main_dispatcher()`; advances on explicit `tick()` from tests.
- [`tests/executor_test.cpp`](../../tests/executor_test.cpp) — coroutine + task-suspension behavior tests.
