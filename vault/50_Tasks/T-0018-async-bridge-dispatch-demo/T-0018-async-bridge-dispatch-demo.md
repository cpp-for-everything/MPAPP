---
type: task
id: T-0018
title: async bridge dispatch demo (ADR-0018 Phase F) on Win / Linux / Android
status: done
milestone: M-04c
owner: alex
area: handlers
blockedBy: []
coveragePercent: 100
hasScreenshots: true
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/handlers
---

# T-0018 — async bridge dispatch demo (ADR-0018 Phase F) on Win / Linux / Android

Rule 11 closure for [[ADR-0018-hybrid-webview-typed-bridge]] Phase F — the typed-async method-dispatch surface (`hybrid_bridge::register_async_method<T>(...)` + `hybrid_bridge::dispatch_async(envelope, on_response)`). Three platform demos drive the same `demo_bridge` derived from `mpapp::hybrid_bridge` through four representative dispatch scenarios — sync method, inline-responding async method, deferred-responding async method (`respond` captured for later), and an unknown-method error envelope — and observe the JSON-RPC response envelopes the dispatcher emits.

## What landed

### Linux — `examples/gtk4_async_bridge_demo/`

A small GTK4 program (one `main.cpp`) with:

- A `demo_bridge` derived from `mpapp::hybrid_bridge` that registers three methods in its constructor:
    - `register_method("add_sync", &demo_bridge::add_sync)` — sync `(int,int) → int`.
    - `register_async_method<int>("add_async_inline", &demo_bridge::add_async_inline)` — async method that calls `respond(a+b)` inline before returning.
    - `register_async_method<int>("defer_add", &demo_bridge::defer_add)` — async method that captures the `respond` callback for later resolution via a `resolve_pending()` member.
- Three status `GtkLabel`s reflecting the most-recent request envelope, the most-recent response envelope (or `(pending — click 'resolve' to fire)` for a deferred call), and a `pending_count` integer.
- Four buttons, one per scenario, each running a one-line `bridge_.dispatch(envelope, out)` or `bridge_.dispatch_async(envelope, cb)` call:
    1. `dispatch  add_sync(2, 3)` — `bridge_.dispatch(R"({"id":1,"method":"add_sync","args":[2,3]})", out)` — UI updates immediately with `{"id":1,"result":5}`.
    2. `dispatch_async  add_async_inline(10, 20)` — `dispatch_async` invokes the user method synchronously, which fires `respond(30)` inline, which fires the `on_response` callback before `dispatch_async` returns.
    3. `dispatch_async  defer_add(7, 8)` — the user method captures `respond` and returns; the `on_response` callback has not fired yet; `pending_count` is set to 1; the response label shows `(pending …)`.
    4. `resolve pending  defer_add` — calls `bridge_.resolve_pending()` which fires the captured `respond`, which feeds back through the `dispatch_async` machinery and finally invokes `on_response` with `{"id":3,"result":15}`.

Each click is wired through a per-button member-functor struct (`btn_sync_cb_t`, `btn_async_inline_cb_t`, …) so the GTK4 `g_signal_connect` callback gets a stable address. The bridge is stored by value as a member of the application class.

### Windows — `examples/windows_async_bridge_demo/`

A WinUI 3 counterpart with the same `demo_bridge`, same envelopes, same four buttons, same status labels. Built with `mpapp-core` + `mpapp-handlers-windows`. Proves the typed-async dispatcher lowers identically on the MSVC toolchain.

### Android — JNI smoke hook in `examples/android_hello/`

Same pattern as T-0017's routing smoke — `MainActivity.onCreate()` calls `nativeRunBridgeSmokeTest()` after the existing `nativeRunRoutesSmokeTest()` hook (T-0017) and before `nativeLaunch()`. The hook is implemented in `examples/android_hello/app/src/main/cpp/native_main.cpp` inside `namespace t0018`:

```cpp
class demo_bridge : public mpapp::hybrid_bridge {
public:
    demo_bridge() {
        register_method("add_sync",        &demo_bridge::add_sync);
        register_async_method<int>("add_async_inline",
                                   &demo_bridge::add_async_inline);
        register_async_method<int>("defer_add",
                                   &demo_bridge::defer_add);
    }
    // ... add_sync / add_async_inline / defer_add / resolve_pending
};

void run_smoke();
```

`run_smoke()` exercises the same four scenarios as the Linux + Windows demos and emits a structured `T-0018:` log line per step.

### Build wiring

- `examples/CMakeLists.txt` adds `gtk4_async_bridge_demo` (under `if(UNIX AND NOT APPLE)`) and `windows_async_bridge_demo` (under `if(WIN32)`).
- Both new dirs reuse the existing per-platform handler library set + `mpapp-core` — `mpapp::hybrid_bridge` is fully header-only (it inlines `dispatch` and `dispatch_async`) so no new build-script changes are needed.
- The Android JNI smoke hook adds one `native` declaration to `MainActivity.java` (`nativeRunBridgeSmokeTest`); the existing `android_hello` Gradle module covers everything else.

## Screenshots

- `screenshots/windows-winui3-async-bridge-initial.png` — Win11 + WinUI 3, cropped to the upper-left 700×320 region of the demo window. Shows the title bar (`MPAPP T-0018 - Async Bridge Demo (WinUI 3)`), all three status labels (`last_request: (none)`, `last_response: (none)`, `pending_count: 0`), and all four typed buttons in their initial state.
- `screenshots/android-emulator-app-running.png` — emulator x86_64 (AVD `coroute_test`, API 28) post-smoke. The visible UI is the existing T-0011 `android_hello` demo; the relevant evidence is that the process is alive after `nativeRunBridgeSmokeTest` ran in `onCreate`. The actual bridge dispatch evidence is the logcat artifact below.

The Linux GTK4 demo screenshot is missing for the WSLg compositor-cache reason documented in `notes/linux-wslg-screenshot-quirk.md` — same quirk as T-0017's `gtk4_routes_demo`. The `msrdc` window shows up in the Windows process list with the correct title and a non-zero rectangle, but the X11 surface inside it does not write to the framebuffer area Windows-side screen capture reads from. `wsl --shutdown` + cold-launch did not clear the cache because `msrdc.exe` survives the WSL VM shutdown.

## Logs

- `logs/android-emulator-bridge-smoke.log` — 5 lines of logcat output captured immediately after launching the rebuilt APK on the running emulator:

```
T-0018: sync add_sync(2,3) -> {"id":1,"result":5}
T-0018: inline add_async_inline(10,20) fired_before_return=true -> {"id":2,"result":30}
T-0018: deferred defer_add(7,8) fired_before_resolve=false has_pending=true
T-0018: deferred defer_add(7,8) fired_after_resolve=true -> {"id":3,"result":15}
T-0018: unknown-method -> {"id":4,"error":"unknown method: missing"}
```

These five lines prove (in order):

1. `dispatch(R"({"id":1,"method":"add_sync","args":[2,3]})", out)` writes `{"id":1,"result":5}` — sync method, JSON-RPC envelope composed correctly.
2. `dispatch_async(R"({"id":2,"method":"add_async_inline","args":[10,20]})", cb)` — the user method's `respond(30)` fires inline, so the `on_response` callback (`cb`) is invoked before `dispatch_async` returns. `fired_before_return=true` confirms this.
3. `dispatch_async(R"({"id":3,"method":"defer_add","args":[7,8]})", cb)` — the user method captures `respond` and returns without firing it. `fired_before_resolve=false` + `has_pending=true` confirms the dispatcher correctly handles the not-yet-resolved case.
4. After `bridge.resolve_pending()` — the captured `respond` fires, which feeds back through `dispatch_async`'s continuation, which finally fires `cb` with `{"id":3,"result":15}`. `fired_after_resolve=true` confirms.
5. `dispatch_async(R"({"id":4,"method":"missing","args":[]})", cb)` — no matching method, dispatcher composes `{"id":4,"error":"unknown method: missing"}` and fires `cb` immediately. Proves the error envelope path.

## Tests

The Phase F dispatcher surface is covered by mock-level tests in `tests/mock_handlers/hybrid_bridge_test.cpp`:

- Sync-method-through-dispatch_async — fires inline, composes `{"id":N,"result":V}`.
- Inline-responding async method — same envelope shape, fires before `dispatch_async` returns.
- Deferred-responding async method — `respond` captured for later, `on_response` fires only when the user method's `respond` is invoked.
- Deferred string-result async method — same path, `T = std::string`.
- Double-respond — the `shared_ptr<bool>` fired-guard inside `async_invoker_builder` drops the second call.
- Unknown-method dispatch — error envelope.
- Malformed-envelope dispatch — error envelope with `id=-1`.
- Async method with bad args — error envelope.

End-to-end visual + log-output regression is covered by this task's screenshots + the Android logcat artifact. The logcat output is deterministic (smoke runs synchronously in `onCreate`) so future regressions in the Phase F surface will reorder lines or change the envelope shapes in a diff-visible way.

The task folder's `tests/` directory is empty by design — same pattern as T-0016 + T-0017, the tests live under `tests/mock_handlers/` because they cover the cross-platform `mpapp::hybrid_bridge` surface, not the per-task demo wiring.

## Build state

- **Linux** Ubuntu 24.04 / GCC 13 / GTK4 4.14.x / WSL2: `cmake --build build-linux --target gtk4_async_bridge_demo` → binary at `build-linux/examples/gtk4_async_bridge_demo/gtk4_async_bridge_demo`. Demo runs (process alive, GTK4 window registered with WSLg's `msrdc` host), screenshot capture defeated by the documented WSLg quirk.
- **Windows** Win11 / MSVC 14.51 / WinUI 3 1.6 (`Microsoft.WindowsAppSDK`): `cmake --build build-full --target windows_async_bridge_demo` → `build-full\examples\windows_async_bridge_demo\windows_async_bridge_demo.exe`. Window screenshot via PowerShell `Graphics.CopyFromScreen` + Win32 `GetWindowRect` cropping.
- **Android** AVD `coroute_test` x86_64 (API 28) / NDK 26.1 / Gradle 8.10 with `_build_android.bat`: APK at `app/build/outputs/apk/debug/app-debug.apk` (10,495,212 bytes), then `adb install -r` + `adb shell am start -n io.mpapp.example/.MainActivity` + `adb logcat -d -s MPAPP:* | grep T-0018`. Smoke runs in `onCreate` synchronously; logcat is ready by the time the launch returns.

## Notes

- `notes/linux-wslg-screenshot-quirk.md` — task-specific note that reaffirms T-0017's diagnosis and records what was attempted differently for T-0018 (the cold-launch / `wsl --shutdown` retry that didn't help because `msrdc.exe` is a Windows-side process that survives the WSL VM shutdown).

## What this catches up

Per Rule 11, this task closes the visible-output gap for the M-04c shipped feature:

- **ADR-0018 Phase F** — typed-async method dispatch (`register_async_method<T>` + `dispatch_async` with both inline-responding and deferred-responding async user methods).

T-0018 is the third Rule 11 catch-up after T-0016 (canvas + Cairo) and T-0017 (typed routing). Remaining in the catch-up batch: CollectionView `item_template` (factory-based typed cells).
