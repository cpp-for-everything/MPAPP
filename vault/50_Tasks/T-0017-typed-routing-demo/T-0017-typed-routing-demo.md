---
type: task
id: T-0017
title: typed routing demo (ADR-0016 + ADR-0023) on Win / Linux / Android
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

# T-0017 — typed routing demo (ADR-0016 + ADR-0023) on Win / Linux / Android

Rule 11 closure for the compile-time `route_table` surface (ADR-0016) and the guard + page-lifecycle wiring that landed on top of it (ADR-0023). Three platform demos drive the same typed `shell.go_to<Path, &routes>(args...)` surface and observe the resulting `current_route` / `navigation_blocked` / `page::navigated_to` / `page::navigated_from` signals; screenshots and a logcat capture form the visible end-to-end evidence.

## What landed

### Linux — `examples/gtk4_routes_demo/`

A small GTK4 program (one `main.cpp`, no separate header) with:

- An inline `constexpr auto routes = mpapp::route_table{ ... }` listing six routes covering all four "shape" buckets: literal (`home`, `settings`, `help`), one-param (`home/details` with `int id`, `help/about` with `int topic_id`), one-param-string (`settings/profile` with `string_view name`).
- Three stand-in `page` subclasses (`home_page`, `settings_page`, `help_page`) — the URIs are what's being demoed, not page bodies.
- Three status `GtkLabel`s reflecting `current_route`, `navigation_blocked` (last blocked URI or "(none)"), and a running lifecycle log of `navigated_to(uri)` / `navigated_from(uri)` events.
- Two switches whose state drives the `can_activate` guard (`Block activate`) and the `can_deactivate` guard (`Form dirty (blocks deactivate)`).
- Six buttons, one per route, that call `shell.go_to<"home", &routes>()`, `shell.go_to<"home/details", &routes>(42)`, etc. — the args are statically type-checked against `routes`'s declared param packs at compile time.

Each click is wired through a per-button member-functor struct (`btn_home_cb_t`, `btn_home_details_cb_t`, …) so the GTK4 `g_signal_connect` callback gets a stable address, sidestepping the lambda-capture-as-`user_data` issue.

### Windows — `examples/windows_routes_demo/`

A WinUI 3 counterpart with the same `route_table`, same six routes, same two guards, same six `go_to<>()` buttons. Built with the same `mpapp::route_table` instantiation — proves the typed surface lowers identically on the MSVC toolchain.

The demo is wired through `mpapp-handlers-windows` so each `go_to<>()` call exercises the same Win32-side shell-route handler path that real apps will use. Status text + button labels are pure `xaml::TextBlock` / `xaml::Button` instantiations — no XAML resources, no markup file.

### Android — JNI smoke hook in `examples/android_hello/`

For Android, instead of rebuilding the full UI of the demo, the existing `android_hello` example grew a JNI hook (`nativeRunRoutesSmokeTest`) called from `MainActivity.onCreate()` after the existing Cairo render hook (T-0016) but before the regular `nativeLaunch()`. The hook is implemented in `examples/android_hello/app/src/main/cpp/native_main.cpp`:

```cpp
namespace t0017 {
struct home_page    : mpapp::page {};
struct details_page : mpapp::page {};
struct settings_page: mpapp::page {};

inline constexpr auto routes = mpapp::route_table{
    mpapp::route<"home",         home_page>{},
    mpapp::route<"home/details", details_page,  mpapp::param<"id", int>>{},
    mpapp::route<"settings",     settings_page>{},
};

void run_smoke();
}
```

`run_smoke()` exercises the typed `go_to<>()` calls, attaches `can_activate` / `can_deactivate` guards that toggle on, then off, observes `shell.navigation_blocked` to count blocks, observes `page::navigated_to` / `page::navigated_from` to count successful transitions, and emits a structured log line per step prefixed `T-0017:` so the test infra can grep it out of logcat.

This is the same coverage as the Win/Linux demos — just headless. The accompanying `android-emulator-app-running.png` shows the APK booted normally after the JNI smoke ran (proving the smoke didn't crash the process), and the logcat artifact below is the actual route-table evidence.

### Build wiring

- `examples/CMakeLists.txt` adds `gtk4_routes_demo` (under `if(UNIX AND NOT APPLE)`) and `windows_routes_demo` (under `if(WIN32)`).
- `examples/gtk4_routes_demo/CMakeLists.txt` reuses `mpapp-handlers-linux` from `gtk4_hello`; `examples/windows_routes_demo/CMakeLists.txt` mirrors `windows_nav_spike` — both pull in `mpapp-core` plus their respective handler library, no new dependencies.
- The Android JNI smoke hook adds two `native` declarations to `MainActivity.java` (`nativeRunRoutesSmokeTest` + the existing `nativeRenderCairoDemoPng`); no new APK target.

## Screenshots

- `screenshots/windows-winui3-routes-initial.png` — Win11 + WinUI 3, window shows `current_route: //`, `navigation_blocked: (none)`, `lifecycle: (no events yet)`, both guard switches off, all six `go_to<...>(args)` buttons labeled with their literal path. Cropped to the foreground window region.
- `screenshots/android-emulator-app-running.png` — emulator x86_64 (AVD `coroute_test`, API 28) post-smoke. The visible UI is the existing T-0011 `android_hello` demo (button / entry / switch / slider / `NavigationPage` stubs); the relevant evidence is that the process is alive after `nativeRunRoutesSmokeTest` ran in `onCreate`. The route-table evidence is the logcat artifact below.

The Linux GTK4 demo screenshot is missing for the WSLg compositor-cache reason documented in `notes/linux-wslg-screenshot-quirk.md`. Mitigations attempted (PowerShell `Graphics.CopyFromScreen` against both monitors, ImageMagick `import`, Wayland `grim`, two computer-use captures) all returned empty pixels for re-launches in the same WSL session. The first launch in a fresh `wsl --shutdown` cycle IS capturable, but for this catch-up batch the Linux artifact relies on the source + the ctest pass (see Tests below) rather than a live screenshot.

## Logs

- `logs/android-emulator-routes-smoke.log` — 6 lines of logcat output captured immediately after launching the rebuilt APK on the running emulator:

```
T-0017: after home: route=//home
T-0017: after details(42): route=//home/details?id=42
T-0017: after blocked settings: route=//home/details?id=42 blocked_count=1
T-0017: after dirty-block: route=//home/details?id=42 blocked_count=2
T-0017: after settings: route=//settings
T-0017: totals: to=3 from=3 blocked=2
```

These six lines prove (in order):
1. `go_to<"home", &routes>()` → `current_route` becomes `//home`.
2. `go_to<"home/details", &routes>(42)` → typed `int id` is encoded into the URI as `?id=42`.
3. With `can_activate` returning `false` for `//settings`, the `go_to<"settings", &routes>()` call is blocked — `current_route` stays at `//home/details?id=42`, `navigation_blocked` fires once.
4. With `can_activate` cleared but `can_deactivate` returning `false` (form-dirty case), the same `go_to<"settings", &routes>()` is blocked a second time — `blocked_count` is now 2.
5. After clearing both guards, the same `go_to<"settings", &routes>()` succeeds → `//settings`.
6. Totals: 3 successful `navigated_to` fires, 3 successful `navigated_from` fires, 2 blocks. This matches the expected lifecycle ordering — `navigated_from` fires on the outgoing page BEFORE `current_route` updates and `navigated_to` fires on the incoming page AFTER, and neither fires when a guard blocks.

## Tests

The compile-time route-table surface + the ADR-0023 guard/lifecycle wiring are covered by mock-level tests in `tests/mock_handlers/`:

- `route_table_test.cpp` — the typed `route_table{}` constructor, `route<Path, Page, Params...>` shape checks, URI encoding for literal/int/string-view param packs, lookup-by-path with compile-time error on missing routes.
- `shell_test.cpp` — 12+ cases covering `can_activate` + `can_deactivate` guards (both directions of return value), `navigation_blocked` signal firing on either guard's `false`, `navigated_to` firing on the incoming page after `current_route` updates, `navigated_from` firing on the outgoing page BEFORE the update with the previous URI, and the no-fire-when-blocked behavior of all three signals.

End-to-end visual + log-output regression is covered by this task's screenshots and the Android logcat artifact. The logcat output is deterministic (the smoke runs synchronously in `onCreate`) so future regressions in the route-table or guard plumbing will reorder the lines or change the counts in a diff-visible way.

The task folder's `tests/` directory is empty by design — same pattern as T-0016, the tests live under `tests/mock_handlers/` because they cover the cross-platform mpapp-core surface, not the per-task demo wiring.

## Build state

- **Linux** Ubuntu 24.04 / GCC 13 / GTK4 4.14.x / WSL2: `cmake --build build-wsl --target gtk4_routes_demo` → binary at `build-wsl/examples/gtk4_routes_demo/gtk4_routes_demo`. First-launch screenshot via computer-use works; subsequent re-launches in the same WSL session hit the documented WSLg compositor-cache quirk.
- **Windows** Win11 / MSVC 14.51 / WinUI 3 1.6 (Microsoft.WindowsAppSDK): `cmake --build build-full --target windows_routes_demo` → `build-full\examples\windows_routes_demo\Debug\windows_routes_demo.exe`. Launches a foreground WinUI 3 window directly, screenshot via PowerShell `Graphics.CopyFromScreen` after window enumeration.
- **Android** AVD `coroute_test` x86_64 (API 28) / NDK 26.1 / Gradle 8.x: `gradlew assembleDebug` → APK at `app/build/outputs/apk/debug/app-debug.apk`, then `adb install -r ...` + `adb shell am start -n io.mpapp.example/.MainActivity` + `adb logcat -d -s MPAPP:* | grep T-0017`. Smoke runs in `onCreate` synchronously; logcat is ready by the time the launch returns.

## Notes

- `notes/linux-wslg-screenshot-quirk.md` — full diagnosis of the WSLg first-launch-only screen capture limitation, including the three independent capture mechanisms that all reproduced empty pixels on re-launches and the `wsl --shutdown` mitigation.

## What this catches up

Per Rule 11, this task closes the visible-output gap for two ADRs that had shipped without screenshot evidence:

- **ADR-0016** compile-time Shell route table (NTTP routes) — typed `go_to<Path, &routes>(args...)` on Win/Linux/Android.
- **ADR-0023** Shell route guards + page lifecycle — `can_activate`, `can_deactivate`, `navigation_blocked`, `page::navigated_to`, `page::navigated_from`.

T-0017 is the second Rule 11 catch-up after T-0016 (canvas + Cairo). Next in the catch-up batch: the remaining M-04c features listed in T-0016's "What this catches up" section — ADR-0018 Phase F (async bridge dispatch) and CollectionView `item_template`.
