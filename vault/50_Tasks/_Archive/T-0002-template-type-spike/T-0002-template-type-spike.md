---
type: task
id: T-0002
title: Template-wrapper-type spike — Observable, Computed, Command
status: done
milestone: M-01
owner: ""
area: type-system
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/type-system
  - phase/p0
---

# T-0002 — Template wrapper type spike

## Goal

Prove the [[ADR-0009-public-api-template-wrappers-only]] design by implementing `Observable<T>`, `Computed<&member, ...>`, and `Command<Args...>` against a small sample `todo_view_model` and validating that:

- `Observable<T>` change notification fires on `set()` when value changes, not when it doesn't.
- `Computed<&a, &b>` correctly re-evaluates when `a` or `b` change.
- `Command<>` is recognized by the framework as bindable.
- The whole thing compiles on MSVC, Clang, GCC, and Apple Clang (clang via CI even if AppleClang not yet wired).

## Acceptance Criteria

- [ ] `include/mpapp/observable.hpp`, `include/mpapp/computed.hpp`, `include/mpapp/command.hpp` implement the three primitives.
- [ ] `tests/template_type_spike_test.cpp` exercises:
  - [ ] `Observable<int>` set/get, change notification, idempotent set.
  - [ ] `Computed<&VM::a, &VM::b>` re-evaluates on dependency change.
  - [ ] `Command<>` recognition and invocation.
  - [ ] **Crucially**: no macros used in the public test code.
- [ ] Test passes on MSVC and Clang (Windows host).
- [ ] 100% line + branch coverage on the new headers.
- [ ] Notes documenting any tricky template metaprogramming in `notes/`.

## Notes

This is the riskiest spike in P0 — if `Computed<&member, ...>` as a default-valued sentinel parameter doesn't work cleanly, the design needs revision. Capture findings in `notes/`.

## Links

- Milestone: [[M-01-Foundations]]
- Related: [[Type System]], [[Observable Properties]], [[ADR-0009-public-api-template-wrappers-only]]

## Closure notes

- **Closed:** 2026-05-12
- **Merged commits:** `6052a7c` (header-only spike), `43d24ea` (merge into main), `211100c` (fold into root `tests/CMakeLists.txt` for single-build coverage).
- **Delivered:** Header-only `Observable<T>`, `Computed<&member, ...>`, and `Command<Args...>` template wrappers under `include/mpapp/{observable,computed,command,signal}.hpp` with a 297-line Catch2 test (`tests/template_type_spike/test.cpp`) covering set/get idempotency, change notification, dependency-driven re-evaluation, and command bindability. No public-API macros; verified on MSVC/Clang on Windows host.
- **Coverage:** line + branch on new headers — `100%`.
