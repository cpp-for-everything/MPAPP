---
type: glossary
term: "Mock Implementation"
tags:
  - type/glossary
---

# Mock Implementation

A platform-agnostic handler stub that logs / no-ops, used during the mock-first phase ([[ADR-0008-mock-first-implementation]]). Enables full-surface unit testing before any real platform code. See [[Test Harness]].

Mock-handler tests construct the [[Basic-Surface|surface]] (`mpapp::internal::basic_<name>`) directly + attach a `mpapp::<name>_handler<mpapp::platform::mock>` externally. This is the contract that keeps `mock_handlers_test` link-isolated from the per-platform handler library: the surface holds the handler by pointer, so the test's translation unit has no ODR-use of any platform handler's symbols.

```cpp
TEST_CASE("button mock handler logs initial text on map", "[mock][button]") {
    mpapp::internal::basic_button b;
    mpapp::button_handler<mpapp::platform::mock> h;
    h.map_text(b);
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text="});
}
```

App code uses the [[Wrapper-Component]] (`mpapp::button`) which embeds the handler; tests deliberately drop down to the surface.

The mock-handler specialisation itself inherits from `mpapp::mock_handler_base` and takes the surface type (`basic_<name>&`) in its `map_<property>` methods, mirroring the per-platform handler signatures.

## See in code

- [`include/mpapp/handlers/mock/button_handler.hpp`](../../include/mpapp/handlers/mock/button_handler.hpp) — canonical mock handler: records `text=<value>` + `clicked` events into a `calls()` vector for assertion. `void map_text(basic_button& b)` / `void map_clicked(basic_button& b)`.
- [`include/mpapp/handlers/mock/`](../../include/mpapp/handlers/mock/) — one mock handler per component, all following the same recording pattern.
- [`tests/mock_handlers/button_test.cpp`](../../tests/mock_handlers/button_test.cpp) — exercises the mock-handler recording surface via `mpapp::internal::basic_button`; `tests/mock_handlers/*_test.cpp` is glob-included so adding a new component's tests doesn't require touching `tests/CMakeLists.txt`.
- [`include/mpapp/handlers/mock/handler_base.hpp`](../../include/mpapp/handlers/mock/handler_base.hpp) — `mock_handler_base` records `calls()` for assertion; both `record(...)` / `record_event(...)` and `mock_property_recorder<Owner, T>` patterns coexist.

## See also

- [[Basic-Surface]] — the type mock-test code constructs directly.
- [[Wrapper-Component]] — what app code uses instead; mock tests bypass it.
- [[Handler]] — the broader handler concept.
- [[ADR-0008-mock-first-implementation]] — the mock-first contract.
- [[ADR-0024-wrapper-component-pattern]] — the wrapper/surface split that preserves mock-test link isolation.
- [[Test Harness]] — how the mock tests are wired into ctest.
