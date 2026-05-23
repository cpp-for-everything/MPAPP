# Windows WebView2 hybrid_web_view init race — RESOLVED

Originally filed as an async-init race; investigation revealed two
stacked bugs that together prevented the WinUI 3 `hybrid_web_view`
from ever firing a JS→C++ bridge call. Both are now fixed.

Verification screenshot: `screenshots/windows-hybridwebview-fix-verified-crop.png`
shows `bridge_calls: 1   last_js_event: page loaded 2026-05-23T16:18:54.216Z`
— matching the GTK4 baseline.

## Bug A — missing runtime DLLs (the dominant symptom)

`mpapp_add_winappsdk_runtime` copied `Microsoft.WindowsAppRuntime{,.Bootstrap}.dll`
next to every WinUI 3 example but never the WebView2 runtime pair:
`WebView2Loader.dll` and `Microsoft.Web.WebView2.Core.dll`. The implicit
load happens at first `EnsureCoreWebView2Async` call. Without the DLLs,
the call throws `winrt::hresult_error` with
`hr=0x8007007E (ERROR_MOD_NOT_FOUND, "The specified module could not be found.")`.

The simpler `web_view` examples (`url = "..."`) coincidentally never hit
this code path because they were never launched on a clean build — they
were verified in worktrees that still had stale copies of the DLLs from
older runs. The hybrid demo, being newer, exposed the gap.

Fix: extended `mpapp_add_winappsdk_runtime` in `cmake/WindowsAppSDK.cmake`
to also copy both WebView2 DLLs from `${MPAPP_WEBVIEW2_DIR}/runtimes/win-x64/`.
The post-build comment now reads "copying WindowsAppRuntime + WebView2
DLLs". `web_view` demos benefit too.

## Bug B — orphaned IAsyncOperation completion

The original wire path subscribed `CoreWebView2Initialized`, then inside
that callback fired `AddScriptToExecuteOnDocumentCreatedAsync` and
immediately `NavigateToString`. Even with Bug A fixed, two problems
remained:

1. `CoreWebView2Initialized` does not fire on its own — the control sits
   idle until something explicitly drives init. `web_view` drove it
   implicitly by assigning `Source = uri`; the hybrid path never set
   Source and never called `EnsureCoreWebView2Async`.
2. Setting `.Completed(handler)` on an `IAsyncOperation` whose only
   strong ref is `auto op = ...` is fragile — the operation can be
   dropped before the handler fires, so the shim install may complete
   without ever notifying us.

Fix: replaced the chain with a `winrt::fire_and_forget` member coroutine
`async_init()` in `src/handlers/windows/hybrid_web_view_handler.cpp`.
The coroutine `co_await`s each step:

1. `EnsureCoreWebView2Async` — kicks init explicitly (no longer waits
   for an event that never fires).
2. `AddScriptToExecuteOnDocumentCreatedAsync(kBridgeShim)` — install
   shim BEFORE any navigation, so the page's auto-fired
   `window.mpapp.call('notify', ...)` finds the bridge in place.
3. Subscribe `WebMessageReceived`.
4. Flush any buffered `pending_html_` via `NavigateToString`.

The coroutine frame keeps each `IAsyncAction/IAsyncOperation` alive
across awaits, naturally serializing the steps. The map_messages path
now just registers a `Loaded` handler that kicks the coroutine once.

## Files touched

- `cmake/WindowsAppSDK.cmake` — added WebView2 DLL copy.
- `include/mpapp/handlers/windows/hybrid_web_view_handler.hpp` —
  added `async_init()` declaration, `shim_added_` / `init_kicked_`
  flags, dropped `core_ready_token_`.
- `src/handlers/windows/hybrid_web_view_handler.cpp` —
  replaced wire-on-Initialized pattern with `async_init` coroutine;
  kept `wire_bridge()` as a sync fallback for the rare reused-handler
  case.
