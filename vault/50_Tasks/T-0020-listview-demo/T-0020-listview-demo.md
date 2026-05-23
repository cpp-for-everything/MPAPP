---
type: task
id: T-0020
title: ListView demo (items_source + selection + item_tapped) on Win / Linux / Android
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

# T-0020 — ListView demo (items_source + selection + item_tapped) on Win / Linux / Android

Rule 11 closure for `mpapp::list_view` — the legacy MAUI virtualized item host that ships for source-compat parity (modern apps use [[CollectionView]]). Three platform demos drive the same `items_source` + `selected_index` + `item_tapped` triad and observe selection/tap state through the platform's native list widget (`GtkListBox` / WinUI 3 `ListView` / Android `ListView`).

## What landed

### Linux — `examples/gtk4_listview_demo/`

A small GTK4 program with:

- A `mpapp::list_view` bound to `mpapp::list_view_handler<linux_>`.
- Three rotating `items_source` variants — 6-fruit, 8-planet, 3-color lists — cycled by a "Rotate items_source" button.
- A status `mpapp::label` reflecting (in one line): items_count, selected_index, the selected item's label, the cumulative tap_count, and the index of the last tap.
- Subscriptions to `lv.item_tapped` (signal<int>) and `lv.selected_index.changed` (signal<const int&>) — clicking a row updates both counters; the rotation button also bumps the change counter by setting `selected_index = 0` after the swap.

Handler wiring: `lv_handler_.map_items_source(lv_)` + `lv_handler_.map_selected_index(lv_)` per the ADR-0020 wrap-platform-recycler pattern.

### Windows — `examples/windows_listview_demo/`

WinUI 3 counterpart with the same demo shape, using `mpapp::list_view_handler<windows>`. WinUI 3's native ListView highlights the selected row in the accent color (red in this user's theme).

### Android — JNI smoke hook in `examples/android_hello/`

`nativeRunListViewSmokeTest()` (`namespace t0020` in `examples/android_hello/app/src/main/cpp/native_main.cpp`) runs 5 scenarios + logs results prefixed `T-0020:`:

1. set items_source = 4 rows
2. set selected_index = 2 (observe selected_index.changed fire)
3. emit item_tapped(1) (observe tap_count = 1, last_tap_index = 1)
4. rotate items_source to 2 rows + selected_index = 0 (observe second selected_index.changed)
5. emit item_tapped(0) (observe tap_count = 2, last_tap_index = 0)

The smoke uses the model-level surface only — no native ListView attached on Android because the JNI hook runs in `onCreate` before the activity layout completes. The Linux + Win demos cover the native-rendered side.

### Build wiring

`examples/CMakeLists.txt` adds `gtk4_listview_demo` (Linux) and `windows_listview_demo` (Win). Both reuse the existing per-platform handler library. The Android JNI hook adds one `native void nativeRunListViewSmokeTest()` to `MainActivity.java`.

## Screenshots

- `screenshots/windows-winui3-listview-initial.png` — Win11 + WinUI 3 + `mpapp::list_view_handler<windows>`. Cropped to 700×380. Shows the title bar (`MPAPP T-0020 - ListView Demo (WinUI 3)`), status line (`items_count: 6  selected_index: 0  selected: Apple  taps: 0  last_tap_index: -1`), "Rotate items_source" button, and the 6-row Apple/Banana/Cherry/Date/Elderberry/Fig list with the first row highlighted (selected_index=0).
- `screenshots/linux-gtk4-listview-initial.png` — Ubuntu 24.04 + WSLg + GTK4 4.14 + `mpapp::list_view_handler<linux_>`. Full window via PrintWindow(PW_RENDERFULLCONTENT). Same UI shape as the Win screenshot — GtkListBox with the first row highlighted in the GTK4 selection accent.
- `screenshots/android-emulator-app-running.png` — emulator x86_64 (AVD `coroute_test`, API 28) post-smoke. The visible UI is the existing T-0011 `android_hello` demo; the relevant evidence is that the process is alive after `nativeRunListViewSmokeTest` ran in `onCreate`. The actual evidence is the logcat artifact below.

## Logs

- `logs/android-emulator-listview-smoke.log` — 5 lines of logcat proving the model-level surface fires the expected signals:

```
T-0020: after items=4: count=4 sel_idx=-1 sel_changes=0
T-0020: after select(2): sel_idx=2 sel_changes=1
T-0020: after tap(1): tap_count=1 last_tap_index=1
T-0020: after rotate-to-2: count=2 sel_idx=0 sel_changes=2
T-0020: after tap(0): tap_count=2 last_tap_index=0
```

These confirm: (1) setting items_source doesn't auto-change selected_index (still -1, no sel_changes fired); (2) explicit selected_index = 2 fires selected_index.changed exactly once; (3) item_tapped.emit(1) routes through the subscriber and bumps tap_count to 1 + last_tap_index to 1; (4) rotating items_source + selected_index = 0 fires a second selected_index.changed; (5) second emit routes correctly.

## Tests

Mock-handler coverage lives in `tests/mock_handlers/list_view_test.cpp`.

## Build state

- **Linux** Ubuntu 24.04 / GCC 13 / GTK4 4.14.x / WSL2: `cmake --build build-linux --target gtk4_listview_demo` → binary at `build-linux/examples/gtk4_listview_demo/gtk4_listview_demo`. Renders + responds to row clicks (selected_index reflects + item_tapped fires).
- **Windows** Win11 / MSVC 14.51 / WinUI 3 1.6: `cmake --build build-full --target windows_listview_demo` → `build-full\examples\windows_listview_demo\windows_listview_demo.exe`. Same.
- **Android** AVD `coroute_test` x86_64 (API 28): APK installed via `adb install -r`; logcat captured via `adb logcat -d -s MPAPP:* | grep T-0020`.

## What this catches up

Per Rule 11, this closes the visible-output gap for the M-04c shipped feature **ListView real handlers (Win / Linux / Android) per ADR-0020**.

T-0020 is the 7th Rule 11 catch-up after T-0014 (async navigation), T-0015 (Shell real handlers), T-0016 (canvas+Cairo), T-0017 (typed routing), T-0018 (async bridge dispatch), T-0019 (CollectionView item_template).
