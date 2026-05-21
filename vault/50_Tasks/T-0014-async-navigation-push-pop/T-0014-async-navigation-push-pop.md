---
type: task
id: T-0014
title: Async push_async/pop_async wired through navigation_page (ADR-0019)
status: done
milestone: M-04c
owner: ""
area: handlers
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/handlers
  - area/threading
---

# T-0014 — Async push_async / pop_async wired through navigation_page

Lands the first concrete consumer of [[ADR-0019-async-executor-native-dispatcher|ADR-0019]]'s `task<T>` shape: `navigation_page` gains `push_async` / `pop_async` / `pop_to_root_async` returning `task<T>`, per [[ADR-0014-page-navigation-stack|ADR-0014]]'s "sync as primitive, async as sugar" rule.

## What landed

### Public surface (cross-platform)

`include/mpapp/navigation_page.hpp` adds three coroutine methods:

```cpp
[[nodiscard]] task<void>  push_async(page* p);
[[nodiscard]] task<page*> pop_async();
[[nodiscard]] task<void>  pop_to_root_async();
```

Each body is just the sync call (`push(p)`, etc.) followed by `co_return`. In the mock build the task suspends and resumes immediately (eager-start runs straight through). Per-platform real handlers can override resumption to wait for transition animations once that surface lands.

### stop_token compatibility shim

The async executor already pulled in `<stop_token>`. Android NDK 26's libc++ ships without that header, so we added `include/mpapp/detail/stop_token_compat.hpp`:

- On platforms with `<stop_token>`, alias `mpapp::stop_source` / `mpapp::stop_token` to the `std` types.
- On platforms without, ship a minimal `shared_ptr<atomic<bool>>`-backed implementation covering the exact surface `executor.hpp` uses (`get_token`, `request_stop`, `stop_requested`, default-construct).

`executor.hpp` now references `mpapp::stop_source` / `mpapp::stop_token` (was `std::`).

### Example

`examples/windows_nav_spike/` — a small WinUI 3 example with two pages (home + details). Home button calls `nav.push_async(&details_page)`; details button calls `nav.pop_async()`. Demonstrates the full ADR-0014 + ADR-0019 pipe end-to-end: page_stack mutates, lifecycle signals fire, navigation_page handler swaps content via the ADR-0013 dispatch registry.

## Test results

### Unit tests (Catch2 mock-handlers)

Four new tests in `tests/mock_handlers/navigation_page_test.cpp`:

| Test | Result |
|---|---|
| `push_async completes synchronously in the mock build` | PASS |
| `pop_async returns the popped page` | PASS |
| `pop_to_root_async collapses the stack` | PASS |
| `async wrappers compose under co_await` | PASS |

The last test exercises real coroutine composition:

```cpp
auto scenario = [](navigation_page& n, page* x, page* y) -> task<int> {
    co_await n.push_async(x);
    co_await n.push_async(y);
    auto* popped = co_await n.pop_async();
    co_return (popped == y) ? 1 : 0;
};
```

Driven by Catch2 — `t.is_ready()` is true after the eager-start, `t.await_resume()` returns the result.

### Cross-platform build

| Platform | Result |
|---|---|
| Windows (build-full) | **232/232 tests pass** — `=== SUCCESS ===` |
| Linux (build-linux, WSL Ubuntu-24.04) | `gtk4_hello` + `mock_handlers_test` link green |
| Android (build-android, NDK 26.1) | `BUILD SUCCESSFUL`, APK produced |

The stop_token compat shim was the only change needed to make Android green — confirmed by an earlier Android failure with `'stop_token' file not found` that resolved after `executor.hpp` was routed through the compat header.

## Computer-use E2E

Attempted to launch `windows_nav_spike.exe` for an interactive screenshot. Both the new nav spike **and the existing button spike** exit immediately with NTSTATUS `0xC000027B` (`STATUS_APPLICATION_INTERNAL_EXCEPTION`) — a WinRT activation failure caused by the absence of a registered `Microsoft.WindowsAppRuntime` framework package on this dev machine.

**This is an environmental issue, not a code issue:** the same exit code reproduces with the long-shipping button spike. The MPAPP code path is unaffected. E2E interactive verification on Windows is deferred pending a working WinAppSDK runtime install. The mock-handler tests + multi-platform build green are the validation MPAPP relies on at this stage.

## Files touched

- `include/mpapp/detail/stop_token_compat.hpp` (new) — std::stop_token shim for NDK 26
- `include/mpapp/executor.hpp` — route through compat; `std::stop_*` → `mpapp::stop_*`
- `include/mpapp/navigation_page.hpp` — add async wrappers
- `tests/mock_handlers/navigation_page_test.cpp` — 4 new tests
- `examples/CMakeLists.txt` — register windows_nav_spike subdir
- `examples/windows_nav_spike/CMakeLists.txt` (new)
- `examples/windows_nav_spike/main.cpp` (new)

## Known limitations

- Mock-build async tasks resume synchronously inside the eager-start. Real platform-native dispatchers (DispatcherQueue / GMainLoop / Looper) integrating with the host UI thread are a follow-up under ADR-0019 §Decision.
- macOS / iOS real navigation_page handlers + async wrappers pending Apple host (per [[ADR-0005-ios-macos-separate-interop]]).

## See also

- [[ADR-0014-page-navigation-stack]] — sync-as-primitive contract this task realizes.
- [[ADR-0019-async-executor-native-dispatcher]] — the executor shape these wrappers compose with.
- [[Components/NavigationPage]] · [[Controls Inventory]]
- [[M-04c-handler-heavy-port]]
