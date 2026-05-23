---
type: adr
id: ADR-0019
title: "Async executor — native UI dispatcher + task<T> adapter"
status: accepted
decisionDate: 2026-05-21
deciders:
  - alex
supersedes: ""
supersededBy: ""
area: threading
tags:
  - type/adr
  - status/accepted
  - area/threading
---

# ADR-0019 — Async executor: native UI dispatcher + task<T>

> [!info] Status
> **proposed** — unblocks NavigationPage's `push_async`/`pop_async`, HybridWebView's typed bridge (ADR-0018), and every other async user surface.

## Context

[[ADR-0014-page-navigation-stack]] specified sync as the primitive with async sugar on top. The remaining question is *which* executor backs `task<T>` and how it integrates with the UI thread.

Three credible options:

- **Bundled executor** — MPAPP ships its own `task<T>` + `ui_task<T>` (scaffolded in `tests/executor_test`). Portable, predictable, no platform dep. **Doesn't** integrate with native idle / timer events — separate plumbing for those.
- **External library** — cppcoro or libcoro. Mature C++20 coroutine schedulers. Adds a dep. Still need UI-thread bridging.
- **Native dispatchers** — use `Microsoft.UI.Dispatching.DispatcherQueue` (Windows), `GMainLoop` / `g_main_context_invoke` (Linux), `android.os.Looper` + `Handler` (Android), `dispatch_async_to_main_queue` / `NSRunLoop` (macOS/iOS). MPAPP's `task<T>` adapts to whichever the platform provides.

The MPAPP profile (cross-platform parity, mock-first, finite binary budget) wants minimal added surface. The UI thread on every platform already runs a message loop; we should compose with it, not parallel to it.

## Decision

We will use **native UI dispatchers** as the backing executor, with a thin MPAPP-side `task<T>` and `await dispatch_to_ui` adapter that bridges to the platform primitive.

Layout:

```cpp
// include/mpapp/async/task.hpp
namespace mpapp::async {

template <class T>
struct task;          // coroutine handle wrapping co_return T

template <class T>
struct ui_task;       // task<T> whose continuation resumes on the UI thread

awaiter dispatch_to_ui();   // co_await this to hop to the UI thread

}
```

```cpp
// Per-platform implementation in src/async/{windows,linux,android,macos,ios}_executor.cpp
//   - Windows : winrt::Microsoft::UI::Dispatching::DispatcherQueue
//   - Linux   : g_main_context_invoke on the default GMainContext
//   - Android : android.os.Handler bound to Looper.getMainLooper
//   - macOS/iOS: dispatch_async to the main queue (libdispatch)
```

The cross-platform `task<T>` shape is uniform; the per-platform executor file binds resumption to the host's UI message loop.

`ui_task<T>` is `task<T>` with a stable contract that the final-suspend resumption happens on the UI thread — handlers that need to touch native widgets `co_await` it.

For testing without a real UI loop, the mock platform (`platform::mock`) ships a `test_dispatcher` that drains the queue synchronously on demand. The existing `tests/executor_test` already scaffolds this — we expand it.

NavigationPage's `push_async(p)` returns `ui_task<void>` whose body calls `np.push(p)` synchronously (per ADR-0014) and the executor handles the "complete on UI thread" semantics. The async sugar is genuinely a wrapper, not a separate code path.

## Consequences

### Positive

- Integrates with native idle / timer / animation loops — no duplicate message pumps.
- No external dependency.
- The `task<T>` shape is consistent across the entire framework (navigation, bridge, future timers, future I/O).
- Tests don't need an emulator or a fake loop — `test_dispatcher` drains in-process.

### Negative

- Per-platform plumbing (5 small files when macOS/iOS land). Each one is ~50 LOC of glue.
- Coroutine-handle bookkeeping is subtle: care needed around exception propagation and cancellation. Documented in `task.hpp` with property-based tests.
- Apps depending on a third-party C++ coroutine lib (cppcoro etc.) may have an interop layer to write. Documented as a known limitation; co_await-interop between coroutine libraries is generally OK because `co_await` is the language contract.

### Neutral

- Cancellation tokens are a follow-up — out of scope for this ADR. The `task<T>` shape leaves room (e.g. a stop_token-like parameter) without breaking API.

## Alternatives Considered

- **Bundled executor only** — rejected; would require us to parallel the UI message loop or run on a separate thread (incompatible with UI-touch-on-UI-thread invariants on all platforms).
- **cppcoro / libcoro** — rejected; adds a dependency for value we'd still have to wrap to bridge to the UI loop. Future ADRs can revisit if a feature emerges that they uniquely offer.
- **Defer async** — rejected; HybridWebView's bridge (ADR-0018) and Shell's go_to (ADR-0016) both want async semantics now.

## References

- [[ADR-0014-page-navigation-stack]] — push_async wraps push() per this executor.
- [[ADR-0018-hybrid-webview-typed-bridge]] — bridge resolves into task<T>.
- [[ADR-0006-interop-parity]] — async semantics must be uniform across platforms.
- `tests/executor_test.cpp` — existing scaffold.
