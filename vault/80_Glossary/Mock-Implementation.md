---
type: glossary
term: "Mock Implementation"
tags:
  - type/glossary
---

# Mock Implementation

A platform-agnostic handler stub that logs / no-ops, used during the mock-first phase (ADR-0008). Enables full-surface unit testing before any real platform code. See [[Test Harness]].

## See in code

- [`include/mpapp/handlers/mock/button_handler.hpp`](../../include/mpapp/handlers/mock/button_handler.hpp) — canonical mock handler: records `text=<value>` + `clicked` events into a `calls()` vector for assertion.
- [`include/mpapp/handlers/mock/`](../../include/mpapp/handlers/mock/) — one mock handler per component, all following the same recording pattern.
- [`tests/mock_handlers/button_test.cpp`](../../tests/mock_handlers/button_test.cpp) — exercises the mock-handler recording surface; `tests/mock_handlers/*_test.cpp` is glob-included so adding a new component's tests doesn't require touching `tests/CMakeLists.txt`.
