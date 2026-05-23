---
type: task
id: T-0027
title: HybridWebView end-to-end demo + html_source surface fill-in (Win + Linux)
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

# T-0027 — HybridWebView end-to-end demo + html_source surface fill-in

Rule 11 closure for `mpapp::hybrid_web_view` — the typed JS-bridge variant of WebView (ADR-0018). Both demos host an embedded HTML page that auto-calls `window.mpapp.call('notify', ...)` on body.onload; the C++ bridge picks up the call and updates a host-side status label. A host button fires `hwv.invoke_js('show', ...)` in the reverse direction.

## Surface gap closed alongside the demo

`mpapp::hybrid_web_view`'s model surface was bridge-only — no way to load HTML through the cross-platform Observable shape that `mpapp::web_view` provides. Apps using HybridWebView had to either start with a blank page or work around it via `invoke_js`/document.body.innerHTML. Closed by:

- **Model**: added `Observable<std::string> html_source{""}` to `include/mpapp/hybrid_web_view.hpp`, matching `mpapp::web_view`'s surface.
- **Linux handler** (`hybrid_web_view_handler<linux_>::map_html_source` + `apply_html`): wires to `webkit_web_view_load_html(view, html, nullptr)`.
- **Windows handler** (`hybrid_web_view_handler<windows>::map_html_source` + `apply_html`): wires to `CoreWebView2.NavigateToString(...)`. Buffers the HTML in `pending_html_` if `CoreWebView2` isn't initialized yet (then flushes once `CoreWebView2Initialized` fires), matching the pattern in `map_messages`.
- Android handler: not changed in this task; the existing `android.webkit.WebView` wiring covers the bridge surface. Adding html_source there is a follow-up if needed.

## What landed

### Linux — `examples/gtk4_hybridwebview_demo/`

Host UI:

- `mpapp::hybrid_web_view` with the bridge attached.
- `notify_bridge` derives from `mpapp::hybrid_bridge`, registers a `notify(string)` method that forwards the message to a host-side callback updating a status field.
- "Send 'hello from C++' to JS" button — calls `hwv_.invoke_js("show", ...)` to push a payload back to the embedded page.
- Status label showing `bridge_calls` + `last_js_event` + `cpp_to_js_sends`.

Embedded HTML (`kIndexHtml`):

- Dark teal page with orange text reading "T-0027 HybridWebView demo".
- An `init()` script that waits for `window.mpapp` to exist, registers a JS-side `show(msg)` method that updates the page's `#reply` paragraph, then fires `window.mpapp.call('notify', 'page loaded ' + ISO timestamp)`.

When the page loads, the auto-fired `notify` call routes through the JSON-RPC envelope → `hybrid_bridge::dispatch` → `notify_bridge::notify(msg)` → host callback → status label update. Within milliseconds of launch, the status label reads `bridge_calls: 1   last_js_event: page loaded 2026-...`.

### Windows — `examples/windows_hybridwebview_demo/`

WinUI 3 counterpart using `mpapp::hybrid_web_view_handler<windows>` wrapping `muxc::WebView2`. Same demo shape; HTML loaded via the new `CoreWebView2.NavigateToString(...)` path.

### Android coverage

The Android HybridWebView wiring already shipped (task #18 + tests/mock_handlers/hybrid_web_view_test.cpp). T-0027 adds `html_source` only to the Win + Linux handlers; the Android handler change is a tiny mechanical follow-up if/when needed (`WebView.loadDataWithBaseURL`).

### Build wiring

`examples/CMakeLists.txt` adds `gtk4_hybridwebview_demo` + `windows_hybridwebview_demo`. Both reuse the existing per-platform handler library — the new `map_html_source` lives in `mpapp-handlers-linux` / `mpapp-handlers-windows` libs.

## Screenshots

- `screenshots/linux-gtk4-hybridwebview-initial.png` — Ubuntu 24.04 / GTK4 4.14 / WebKitGTK 6.x / WSLg. Shows the host UI: title bar, status label `bridge_calls: 1  last_js_event: page loaded <ISO timestamp>  cpp_to_js_sends: 0`, "Send 'hello from C++' to JS" button. The embedded HTML region below is solid black due to the documented PrintWindow / EGL composition-layer limitation (same as T-0026's WebView demo); the bridge_calls=1 in the status proves the embedded page loaded, executed its JS, and successfully called `notify` through the typed bridge.
- `screenshots/windows-winui3-hybridwebview-initial.png` — Win11 / WinUI 3 / WebView2. Same demo shape; WebView2 area in the screenshot is similarly opaque to PrintWindow (DComp composition layer) but the status label reflects the bridge state.

## Notes

- `notes/printwindow-limitation.md` cross-references T-0026's documentation of the WebView capture limitation.

## Tests

Mock-handler coverage lives in `tests/mock_handlers/hybrid_web_view_test.cpp` + `tests/mock_handlers/hybrid_bridge_test.cpp` (ADR-0018 Phase A through F).

## What this catches up

Per Rule 11, this closes the visible-output gap for the **HybridWebView end-to-end JS-bridge round-trip** AND lands the `html_source` surface fill-in that was missing from the v1 hybrid_web_view model. T-0027 is the 14th Rule 11 catch-up of this session and closes the HybridWebView catch-up backlog entry.
