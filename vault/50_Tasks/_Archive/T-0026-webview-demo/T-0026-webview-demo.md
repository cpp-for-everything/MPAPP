---
type: task
id: T-0026
title: WebView demo (html_source toggle, navigation signals) on Win + Linux
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

# T-0026 — WebView demo (html_source toggle, navigation signals)

Rule 11 closure for `mpapp::web_view` — the native browser embed wrapped via WebKitGTK 6.x (Linux), WebView2 (Windows), and `android.webkit.WebView` (Android). The demo exercises the `html_source` Observable + `is_loading` + `navigated` signal end-to-end without depending on network access.

## What landed

### Linux — `examples/gtk4_webview_demo/`

A small GTK4 program with:

- A `mpapp::web_view` bound via `wv_handler_.map_url` + `map_html`.
- Two embedded HTML strings (`kPageOne` orange-themed welcome, `kPageTwo` teal-themed "page two") swapped via a "Toggle content" button. The button bumps `html_source.changed`, the handler reloads the embedded page, and `navigated` fires with the inline-load URL (typically `about:blank` for raw HTML).
- A status label showing `is_loading` + `last_nav_url` + `page: one|two`, refreshed via subscriptions to `wv_.is_loading.changed` and `wv_.navigated`.

The Linux handler binds to **WebKitGTK 6.x** dynamic — LGPL-2.1 via dynamic-link per [[RFC-0001-licensing-and-patent-strategy]] § Linux pattern.

### Windows — `examples/windows_webview_demo/`

WinUI 3 counterpart using `mpapp::web_view_handler<windows>` which wraps `Microsoft.UI.Xaml.Controls.WebView2`. Same demo shape, same two pages.

### Android coverage

The Android `web_view_handler` wraps `android.webkit.WebView` and was completed earlier in M-04c (task #17 in the project task list). No dedicated Android JNI smoke is added here because the WebView surface is value-type observables (url / html_source / is_loading / can_go_back / can_go_forward) covered exhaustively by `tests/mock_handlers/web_view_test.cpp`; the visible-output evidence is the Win + Linux screenshots.

### Build wiring

`examples/CMakeLists.txt` adds `gtk4_webview_demo` (Linux) + `windows_webview_demo` (Win). Both reuse the existing per-platform handler library.

## Screenshots

- `screenshots/linux-gtk4-webview-initial.png` — Ubuntu 24.04 / GTK4 4.14 / WebKitGTK 6.x / WSLg. Shows the title bar, status label `is_loading: false  last_nav_url: about:blank  page: one`, "Toggle content" button. The embedded WebView area below shows as solid black due to a PrintWindow limitation (the WebKitGTK content renders through a separate EGL composition layer that PrintWindow can't read) — see `notes/webview-printwindow-limitation.md`. The status label `last_nav_url: about:blank` confirms the `navigated` signal fired correctly (WebKitGTK reports `about:blank` for inline `html_source` loads).
- `screenshots/windows-winui3-webview-initial.png` — Win11 / WinUI 3 / WebView2 runtime, cropped to the host UI region (600×220). Same demo shape; the embedded WebView2 area suffers the same PrintWindow limitation. `last_nav_url: (none)` because WebView2 initializes asynchronously and the captured frame predates the navigated event.

## Tests

Mock-handler coverage lives in `tests/mock_handlers/web_view_test.cpp` (url + html_source state, navigation signal order, is_loading toggle).

## What this catches up

Per Rule 11, this closes the visible-output gap for **WebView real handlers** (task #17 in the project task list). T-0026 is the 13th Rule 11 catch-up. The HybridWebView demo (JS bridge end-to-end with a host HTML page) is a separate follow-up task — T-0026 covers only the plain WebView surface.
