---
type: moc
area: threading
tags:
  - area/threading
---

# Async Executor and Event Loops

MPAPP owns its async executor. It is a **coroutine-based thread pool integrated with each platform's native event loop**, exposed as a **zero-cost abstraction** via CMake link-time implementation selection.

This is more ambitious than MAUI's `MainThread.InvokeOnMainThreadAsync` (which delegates to the platform's main thread queue). MPAPP integrates async I/O directly with the platform's most efficient primitive.

> [!info] Status — real main-thread dispatchers landed (`d5db702`)
> The **UI-thread dispatcher** half is real on Win + Linux + Android (macOS/iOS blind). `mpapp::dispatcher` gained a virtual `post_after`, and `main_dispatcher()` now returns whatever is installed via `install_main_dispatcher(dispatcher*)`, defaulting to the deterministic `test_dispatcher` (so tests keep virtual time). Each platform's application handler installs its real dispatcher at startup (on the UI thread):
> - **Linux** — `glib_dispatcher` (`g_idle_add_full` / `g_timeout_add_full` on the GLib main loop). `src/handlers/linux/glib_dispatcher.cpp`. Runtime-verified headless.
> - **Windows** — `win_dispatcher` (`DispatcherQueue.TryEnqueue` / one-shot `DispatcherQueueTimer`). `src/handlers/windows/dispatcher_queue.cpp`.
> - **Android** — `looper_dispatcher` (`Handler` on the main `Looper`: `post` / `postDelayed`) + the `io.mpapp.MppDispatchRunnable` shim + `nativeRun` JNI trampoline. `src/handlers/android/looper_dispatcher.cpp`.
> - **macOS / iOS** — GCD main queue (`dispatch_async` / `dispatch_after`), **blind** (compiled+run on a Mac: PENDING).
>
> The real dispatchers live in the per-platform **handler libs** (which link the native toolkit), so `mpapp-core` stays platform-neutral (T-0032). The background **thread pool** + native **async I/O** primitives below (IOCP / io_uring / kqueue / ALooper-epoll) remain the aspirational next layer — `async_sleep` / `ui_task` continuations / animation frame ticks already run on the real main loop through these dispatchers.

## Per-platform event-loop primitive

| Platform | Native primitive | Implementation file | Notes |
|---|---|---|---|
| Windows | IOCP | `src/executor/windows_iocp.cpp` | Best-of-breed for Windows async I/O |
| Linux | io_uring (preferred) + epoll fallback | `src/executor/linux_iouring.cpp` + `src/executor/linux_epoll.cpp` | io_uring on kernel ≥5.6; falls back to epoll otherwise |
| macOS / iOS | kqueue + CFRunLoop bridge | `src/executor/apple_kqueue.cpp` | UI thread runs CFRunLoop; background uses kqueue |
| Android | epoll + ALooper bridge | `src/executor/android_alooper.cpp` | UI thread runs ALooper; background uses epoll |

CMake selects the right `.cpp` per target. Only one implementation links into the final binary.

## The public API

Same names, semantics, types on every platform:

```cpp
namespace mpapp {

class executor;                              // Background thread pool
class dispatcher;                            // Per-thread message dispatcher
inline dispatcher& main_dispatcher();        // UI thread dispatcher

template <class T> class task;               // Background coroutine
template <class T> class ui_task;            // Resumes on UI thread
template <class T> class io_task;            // Resumes on I/O completion

// Awaitable primitives — backed by platform-native I/O
auto async_read(file_handle, span<byte>) -> io_task<size_t>;
auto async_connect(socket&, endpoint) -> io_task<void>;
auto async_sleep(duration) -> task<void>;

} // namespace mpapp
```

## Threading model

- **One UI thread** per process, bound to the platform's main run loop (CFRunLoop on Apple, ALooper on Android, message pump on Windows, GMainLoop on Linux/GTK).
- **Background pool** sized to `std::thread::hardware_concurrency()` by default, work-stealing queues.
- **I/O completion** handled by the platform-native primitive; completions resume the awaiting coroutine on whichever thread is most appropriate (UI for `ui_task`, pool for `task`, completion thread for `io_task`).

## Cancellation

`std::stop_token` propagated through coroutine frames. **No** cancellation-via-exception:

```cpp
auto fetch(std::stop_token stop) -> task<data> {
    auto bytes = co_await async_read(file, buf, stop);  // throws if cancelled? NO
    if (stop.stop_requested()) co_return std::unexpected{cancelled{}};
    co_return parse(bytes);
}
```

This matches `std::jthread` semantics — explicit checking, no surprise unwinds.

## Coroutine ABI portability

C++20 coroutines on MSVC / libstdc++ / libc++ are mature in 2026 but allocator customization is uneven. MPAPP pins coroutine-frame allocators per-platform in the executor implementations, so user-facing `task<T>` behaves identically.

## Lifecycle and dispatch

```cpp
// Background work:
mpapp::task<int> compute() {
    co_await mpapp::executor::current();         // Suspend, resume on pool
    return expensive();
}

// UI update from background:
mpapp::ui_task<void> refresh_ui(data d) {
    co_await mpapp::main_dispatcher();           // Suspend, resume on UI thread
    label.text = d.title;
    co_return;
}

// I/O on the native primitive:
mpapp::io_task<size_t> read_log() {
    auto n = co_await mpapp::async_read(file, buf);  // IOCP / io_uring / kqueue
    co_return n;
}
```

## Why own the executor

1. **Zero overhead.** No virtual dispatch on the hot path — the implementation is monomorphized by the linker.
2. **Native I/O integration.** Users get io_uring on modern Linux automatically; on Windows, IOCP — for free.
3. **Cancellation discipline.** `stop_token` is explicit; no surprise exception unwinds in long-running pipelines.
4. **Test harness friendly.** Mock-first ([[ADR-0008-mock-first-implementation]]) tests use a `test_dispatcher` that drives time deterministically.

## Skeleton implementation

The public-API skeleton lives in
[`include/mpapp/executor.hpp`](../../include/mpapp/executor.hpp) and
[`include/mpapp/test_dispatcher.hpp`](../../include/mpapp/test_dispatcher.hpp).
It implements the surface listed under "The public API" against a
deterministic `test_dispatcher` so mock-first tests
([[ADR-0008-mock-first-implementation]]) can target the async API today.

Implemented in this skeleton:

- `mpapp::dispatcher` — abstract base. `co_await dispatcher` resumes on the
  dispatcher's thread via `post(...)`.
- `mpapp::test_dispatcher` — deterministic dispatcher with a virtual clock.
  - `post(fn)` — enqueue immediately.
  - `post_after(d, fn)` — enqueue with a virtual-time deadline.
  - `advance(d)` — advance the virtual clock by `d`, firing every timer whose
    deadline has passed and draining ready work between ticks.
  - `run_until_idle()` — drain the ready queue without advancing the clock.
- `mpapp::main_dispatcher()` — returns a process-wide `test_dispatcher`
  singleton. Real per-platform dispatchers replace this in P3+ (the platform
  source files in the table above stay stubbed with `// TODO P4+` markers).
- `mpapp::executor` — background-pool stub. `schedule(fn)` and
  `co_await executor::current()` both route to `main_dispatcher()` for now;
  the real work-stealing pool lands in P3+.
- `mpapp::task<T>` — eager-start coroutine. Awaitable; provides
  `request_stop()`, `is_cancelled()`, `get_stop_token()`, `is_ready()`,
  `await_resume()`.
- `mpapp::ui_task<T>` — same shape as `task<T>`, but its `final_suspend`
  reschedules the continuation on `main_dispatcher()`.
- `mpapp::async_sleep(duration, stop_token = {})` — awaitable that yields
  control for `d` of virtual time and short-circuits if the supplied
  `std::stop_token` is already requested.

Cancellation matches the architecture's contract: `std::stop_token` is
propagated as a coroutine parameter, never as an exception. The mock build
links `src/executor/mock.cpp`; the platform-real sources stay stubbed and are
filled in alongside their per-platform task.

## See also

- [[Threading and Dispatcher]] (alias / cross-ref)
- [[Build System]]
- [[Test Harness]]
- [[70_References/IOCP]]
- [[70_References/io_uring]]
- [[70_References/kqueue]]
