---
type: moc
area: tooling
tags:
  - area/tooling
  - area/process
---

# Test Harness

Per [[ADR-0008-mock-first-implementation]] (CLAUDE rule 6), MPAPP develops the full API surface as **mocks** before any platform code. The test harness is built to make mock-based testing the default and to enable **human-free UI automation on Apple platforms** from the day a MacBook self-hosted runner comes online.

## Layers

```
┌─────────────────────────────────────────────────────┐
│ UI smoke tests (per-platform native UI automation)  │  P3+
├─────────────────────────────────────────────────────┤
│ Conformance tests (cross-platform, public API)      │  P2+
├─────────────────────────────────────────────────────┤
│ Mock unit tests (Catch2 or GoogleTest)              │  P1+
└─────────────────────────────────────────────────────┘
```

## Layer 1 — Mock unit tests

Where most testing happens. Runs on any host, no native dependencies.

```cpp
TEST_CASE("Observable<int> fires on change") {
    mpapp::Observable<int> count{0};
    int hits = 0;
    mpapp::signal_slot<const int&> slot;
    count.changed.connect(slot, [&](int) { ++hits; });

    count.set(1);
    REQUIRE(hits == 1);
    count.set(1);   // same value, no fire
    REQUIRE(hits == 1);
    count.set(2);
    REQUIRE(hits == 2);
}
```

Mock handlers (`button_handler<platform::mock>`) record calls and let tests assert against the recorded sequence:

```cpp
TEST_CASE("Button click invokes command") {
    auto vm = todo_view_model{};
    auto btn = mpapp::button{ .command = bind(&todo_view_model::increment) };
    auto& handler = btn.handler<platform::mock>();

    handler.simulate_click();
    REQUIRE(vm.count.get() == 1);
}
```

## Layer 2 — Conformance tests

Same test, run on every supported platform. Same observable behavior expected. This is what enforces [[Interop Parity]].

```cpp
CONFORMANCE_TEST("Button text round-trip") {
    auto btn = mpapp::button{ .text = "hello" };
    REQUIRE(btn.text.get() == "hello");
    btn.text = "world";
    REQUIRE(btn.text.get() == "world");
}
```

On a real platform, this test also asserts the native widget's text matches via platform-specific UI introspection.

## Layer 3 — UI smoke tests

Per-platform native UI automation. The platforms differ in tools:

| Platform | UI automation |
|---|---|
| Windows | [WinAppDriver](https://github.com/microsoft/WinAppDriver) / UIA |
| Android | [UIAutomator](https://developer.android.com/training/testing/ui-automator) |
| Linux | [Dogtail](https://dogtail.gitlab.io/) / AT-SPI |
| macOS | AppleScript / Accessibility API |
| iOS | `XCUITest` driven via `xcrun simctl` |

These tests validate that the gallery app behaves correctly end-to-end on each platform. Run on every PR (subject to CI budget — see [[CI Strategy]]).

## Apple platform: human-free from day one

A key user directive: when we eventually iterate on macOS/iOS, the AI agent must be able to iterate **without human intervention**. That means:

1. Tests run via `xcrun simctl` (iOS Simulator) and `osascript` / Accessibility API (macOS) — no manual interaction.
2. The build pipeline produces self-running app bundles that exit on test completion.
3. Failure modes are recoverable: a stuck Simulator gets reset programmatically.
4. Screenshots are captured automatically and attached to the PR.

Designing this harness is the job of **T-0008** ([[T-0008-mac-ios-test-harness-design]]) in P1, well before the MacBook comes online.

## Coverage

Per CLAUDE rule 11, tasks cannot close without 100% line + branch coverage of new code. Tools:

- **Windows / Linux / macOS:** `llvm-cov` (Clang's source-based coverage).
- **Android:** `llvm-cov` on the ARM64 build, results merged.
- **iOS:** `llvm-cov` via Xcode's profiling.

Coverage reports are uploaded to a free tier of Codecov (or self-hosted alternative — TBD via separate RFC if needed).

## Mock dispatcher for deterministic async tests

```cpp
TEST_CASE("Async chain") {
    mpapp::test_dispatcher dispatcher;

    auto t = [&]() -> mpapp::ui_task<int> {
        co_await mpapp::async_sleep(std::chrono::seconds(1));
        co_return 42;
    }();

    REQUIRE_FALSE(t.done());
    dispatcher.advance(std::chrono::seconds(1));
    REQUIRE(t.done());
    REQUIRE(t.value() == 42);
}
```

The mock dispatcher drives time deterministically — no real sleeps, no flaky tests.

## See in code

- [`tests/CMakeLists.txt`](../../tests/CMakeLists.txt) — Catch2 fetch + `mock_handlers_test` executable that glob-includes every `tests/mock_handlers/*_test.cpp` via `CONFIGURE_DEPENDS`. Adding a test = drop a file in the dir, no CMake edit.
- [`tests/mock_handlers/`](../../tests/mock_handlers/) — Layer 1 mock unit tests. 66 per-component test files exercising the surface contract. Examples:
  - [`button_test.cpp`](../../tests/mock_handlers/button_test.cpp) — mock-handler recording: mapper-on-attach, no-emit-on-same-value, click forwarding.
  - [`graphics_skia_test.cpp`](../../tests/mock_handlers/graphics_skia_test.cpp) — backend-conditional via `#if MPAPP_GRAPHICS_HAS_SKIA`; reads pixels back through `canvas::pixel_data()`.
- [`tests/smoke_test.cpp`](../../tests/smoke_test.cpp) — minimum-viable smoke for the mpapp-core link.
- [`tests/executor_test.cpp`](../../tests/executor_test.cpp) — async executor + `test_dispatcher` deterministic-time exercises (the "no real sleeps, no flaky tests" pattern).
- [`tests/template_type_spike/`](../../tests/template_type_spike/) — Layer 0 type-system invariant tests (Observable / Computed / Command compile-time checks).
- Layer 2 (conformance) + Layer 3 (per-platform UI automation) — described in this doc but not yet implemented; lands with M-09 tooling-DX milestone.

## See also

- [[ADR-0008-mock-first-implementation]]
- [[CI Strategy]]
- [[Async Executor and Event Loops]]
- [[Controls Inventory]]
- [[CLAUDE]] rule 11
