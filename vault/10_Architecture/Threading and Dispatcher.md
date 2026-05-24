---
type: moc
area: threading
tags:
  - area/threading
---

# Threading and Dispatcher

The threading model is described in [[Async Executor and Event Loops]] — this note is a thin cross-reference for navigation.

## Summary

- **One UI thread**, bound to the platform's main run loop.
- **Background pool** with work-stealing queues, sized to hardware concurrency.
- **I/O completions** dispatched by platform-native primitive (IOCP / io_uring / kqueue / epoll+ALooper).
- **Coroutines (C++20)** are the user-facing API: `task<T>`, `ui_task<T>`, `io_task<T>`.
- **Cancellation** via `std::stop_token`, not exceptions.

## See in code

- [`include/mpapp/executor.hpp`](../../include/mpapp/executor.hpp) — public surface: `task<T>`, `ui_task<T>`, `main_dispatcher()`, the cancellation `std::stop_token` plumbing per [[ADR-0019-async-executor-native-dispatcher]].
- [`src/executor/mock.cpp`](../../src/executor/mock.cpp) — deterministic `test_dispatcher` (mock backing). Real per-platform sources (Win IOCP, Linux io_uring, macOS kqueue, Android epoll+ALooper) land per the M-04+ roadmap.
- [`tests/executor_test.cpp`](../../tests/executor_test.cpp) — coroutine ordering + `task<T>` await semantics.

## See

[[Async Executor and Event Loops]] for the full design.
