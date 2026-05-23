---
type: task
id: T-0015
title: Shell real handlers (Win/Linux/Android)
status: done
milestone: M-04c
owner: ""
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

# T-0015 — Shell real handlers (Win / Linux / Android)

Promotes [[Components/Shell|Shell]] from mock to **android-real** by landing the rendering layer on all three primary platforms. The compile-time route table per [[ADR-0016-shell-compile-time-routes]] is intentionally deferred — the string-based `go_to()` URI parser shipped with the Shell mock is sufficient to drive `current_route` + `current_tab_index` for now.

## What landed

### Public surface (cross-platform)

`include/mpapp/shell.hpp` gains one new Observable:

```cpp
Observable<page*> current_content{nullptr};   // main content area
```

Apps set this when they swap content (typically in response to `current_tab_index.changed` or `navigated`). The real handlers swap the native content host's child via the [[ADR-0013-data-driven-widget-dispatch|ADR-0013]] dispatch registry on every change.

The earlier Shell mock surface (`current_route`, `tabs`, `current_tab_index`, `is_flyout_open`, `flyout_content`, `register_route`, `go_to`, `navigated`, `flyout_toggled`) is unchanged. Real handlers wire `tabs.changed` to a rebuilt native tab strip, `is_flyout_open.changed` to the SplitView/Paned/LinearLayout flyout visibility, `flyout_content.changed` to the flyout pane's child, and `current_content.changed` to the main content host's child.

### Per-platform handlers

**Windows** (`src/handlers/windows/shell_handler.cpp`)
- Wraps `mux::Controls::SplitView` (Pane = flyout host, Content = main grid).
- Main grid is a 2-row Grid: row 0 (Auto) = horizontal `StackPanel` of tab `Button`s, row 1 (*) = `ContentControl` bound to `current_content`.
- Tab button `Click` handler sets `current_tab_index` on the bound shell.
- `IsPaneOpen` ↔ `is_flyout_open`. `OpenPaneLength` = 240.

**Linux** (`src/handlers/linux/shell_handler.cpp`)
- Wraps a horizontal `GtkPaned`. Start child = flyout host (`GtkBox`, visibility toggled). End child = vertical `GtkBox` containing a horizontal tab strip `GtkBox` + a content host `GtkBox`.
- Tab buttons are `GtkButton`s; their `clicked` signal sets `current_tab_index`. Per-button (shell*, index) pair is stored as gobject data, freed by gobject finalizer.

**Android** (`src/handlers/android/shell_handler.cpp`)
- Wraps a horizontal `LinearLayout`. Left child = `FrameLayout` flyout host (visibility toggled). Right child = vertical `LinearLayout` containing a horizontal `LinearLayout` tab strip + a `FrameLayout` content host.
- Tab buttons are `android.widget.Button`s. **OnClickListener wiring to set `current_tab_index` is deferred to M-05 polish** — same pattern as the Android navigation_page handler.
- All JNI helpers begin with `if (env->ExceptionCheck()) env->ExceptionClear();`. Global refs released in the destructor.

Each platform `.cpp` self-registers via the `widget_dispatch` registry so a `shell*` resolves to its platform-native UIElement / GtkWidget* / jobject when nested.

## Test results

### Unit tests (Catch2 mock-handlers)

One new test added on top of the existing 6 Shell mock tests:

| Test | Result |
|---|---|
| `shell.current_content tracks page swaps` | PASS |

Plus the prior Shell mock tests (`defaults`, `register_route`, `go_to`, `navigated signal`, `flyout helpers`, `mock handler records`) continue to pass — the surface for those is unchanged.

### Cross-platform build

| Platform | Result |
|---|---|
| Windows (build-full) | **233/233 tests pass** (1 new), `=== SUCCESS ===` |
| Linux (build-linux, WSL Ubuntu-24.04) | `gtk4_hello` + `mock_handlers_test` link green |
| Android (build-android, NDK 26.1) | `BUILD SUCCESSFUL`, APK produced |

### Computer-use E2E

Closed retroactively by [[_Archive/T-0017-typed-routing-demo|T-0017]]: the typed-routing demo's two visible demos (`examples/gtk4_routes_demo/` and `examples/windows_routes_demo/`) instantiate a real `mpapp::shell` driven by `mpapp::shell_handler<linux_>` and `mpapp::shell_handler<windows>` respectively, including `shell_.add_tab(...)` calls and `shell_.go_to<...>()` navigations that flow through the same `current_route` / `current_content` / `tabs` Observables this task lands. The T-0017 archived screenshots therefore serve as the visible E2E evidence for the Shell real handlers too:

- `_Archive/T-0017-typed-routing-demo/screenshots/windows-winui3-routes-initial.png` — Win real shell handler rendering tab strip + content host through `mpapp::shell_handler<windows>`.
- `_Archive/T-0017-typed-routing-demo/screenshots/linux-gtk4-routes-initial.png` — Linux real shell handler rendering through `mpapp::shell_handler<linux_>` and `GtkPaned` + tab `GtkButton`s.
- `_Archive/T-0017-typed-routing-demo/logs/android-emulator-routes-smoke.log` — Android exercise of the same shell + routes surface; tab buttons render via `LinearLayout` per `mpapp::shell_handler<android>`.

The `Microsoft.WindowsAppRuntime` framework activation issue that originally blocked the Windows screenshot on this task was incidentally resolved while implementing T-0017's demos (`mpapp_add_winappsdk_runtime` now copies the WinAppSDK runtime DLLs next to each demo's exe via `cmake --build … --target …`).

## Files touched

- `include/mpapp/shell.hpp` — add `current_content` Observable
- `include/mpapp/handlers/{windows,linux,android}/shell_handler.hpp` (new)
- `src/handlers/{windows,linux,android}/shell_handler.cpp` (new)
- `tests/mock_handlers/shell_test.cpp` — add 1 new test
- `vault/10_Architecture/Controls Inventory.md` — Shell row → android-real
- `vault/10_Architecture/Components/Shell.md` — frontmatter + status callout
- `vault/40_Roadmap/M-04c-handler-heavy-port.md` — tracker update

## Known limitations

- **Compile-time route table (ADR-0016) is deferred.** The current Shell still uses runtime string lookup against `registered_routes`. Apps that want type-safe `go_to<"...">(args...)` wait on the ADR-0016 implementation pass.
- **Android tab clicks don't yet update `current_tab_index`.** The OnClickListener bridge piece lives in the M-05 polish task across all Android widgets that need it (NavigationPage back button, Shell tab strip, etc.).
- **macOS / iOS real handlers** pending Apple host (per [[ADR-0005-ios-macos-separate-interop]]).
- **No bar-styling props honored yet** — `tabs` are rendered as basic Buttons; selected-state highlighting, theming, and bar colors land with the Shell styling pass.

## See also

- [[Components/Shell]] · [[Controls Inventory]] · [[M-04c-handler-heavy-port]]
- [[ADR-0014-page-navigation-stack]] — composes with the page-stack engine.
- [[ADR-0016-shell-compile-time-routes]] — deferred follow-up.
- [[ADR-0013-data-driven-widget-dispatch]] — dispatch registry the handlers use.
- [[T-0014-async-navigation-push-pop]] — sibling task; same WinAppSDK blocker for E2E.
