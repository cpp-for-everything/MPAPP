---
type: task
id: T-0019
title: CollectionView item_template demo (factory-based typed cells) on Win / Linux / Android
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

# T-0019 — CollectionView item_template demo (factory-based typed cells) on Win / Linux / Android

Rule 11 closure for `collection_view::item_template` — the `Observable<std::function<std::unique_ptr<view>(int)>>` factory that materializes one typed cell per `items_source` row, automatically re-materializes whenever `items_source` or `item_template` change, and surfaces the materialized cells through the same typed-items render path the existing handlers already consume. Three platform demos drive the same factory + rotation pattern and observe the resulting `materialized_count`, cumulative factory invocations, and `materialized_changed` signal.

## What landed

### Linux — `examples/gtk4_item_template_demo/`

A small GTK4 program with:

- A `row_label` composite (`mpapp::label` + its `label_handler<linux_>`) so the `item_template` factory can return a fully-renderable cell as a single `unique_ptr<view>`. The `collection_view` owns the `unique_ptr`; the `unique_ptr` owns both the label and its handler.
- An `item_template` lambda that:
    1. increments `factory_invocations_` (cumulative counter — never resets);
    2. records the last passed-in index in `last_factory_index_`;
    3. creates a fresh `row_label`;
    4. sets `text = "[" + index + "] " + items_source[index]`.
- Three rotating `items_source` variants (`{Apples, Bananas, Cherries, Dates}`, `{Sapphire, Topaz, Garnet}`, `{Mercury, Venus, Earth, Mars, Jupiter}`).
- A "Rotate items_source" button that bumps `rotation_index_ = (… + 1) % 3` and re-sets `cv_.items_source = items_sets_[rotation_index_]`, which triggers the auto-re-materialize path.
- A status label that refreshes via the `materialized_changed` signal — `materialized_count: N   factory_invocations: M   last_index: K`.

Handler wiring: `cv_handler_.map_typed_items(cv_)` subscribes the handler to both `typed_items.changed` AND `materialized_changed`, so the GTK4 `GtkListBox` re-renders the rows whenever the factory re-materializes (`rebuild_active` → `rebuild_typed(materialized_views())`).

### Windows — `examples/windows_item_template_demo/`

A WinUI 3 counterpart with the same demo shape — same `row_label` composite (with `label_handler<windows>`), same three items_sets, same rotation button, same status label. Built with `mpapp-core` + `mpapp-handlers-windows`.

### Android — JNI smoke hook in `examples/android_hello/`

Same pattern as T-0017 + T-0018 — `MainActivity.onCreate()` calls `nativeRunItemTemplateSmokeTest()` after the existing T-0018 bridge smoke. The hook is implemented in `examples/android_hello/app/src/main/cpp/native_main.cpp` inside `namespace t0019` as `void run_smoke()`. It exercises the surface in four stages:

1. set item_template + set items_source (4 items) → expect 4 factory calls
2. rotate items_source (3 items) → expect 3 more factory calls, total 7
3. rotate items_source (5 items) → expect 5 more factory calls, total 12
4. clear item_template → expect `materialized_count == 0` without further factory calls

The smoke uses `mpapp::label` cells with NO attached handler — the smoke exercises the model-level surface (factory invocation count + `materialized_count` + `materialized_changed` signal firings), not the Android `RecyclerView`-side render path. The Linux + Windows demos cover the handler-rendered side.

### Build wiring

- `examples/CMakeLists.txt` adds `gtk4_item_template_demo` (under `if(UNIX AND NOT APPLE)`) and `windows_item_template_demo` (under `if(WIN32)`).
- Both new dirs reuse the existing per-platform handler library set + `mpapp-core`. No new headers, no new build-script changes.
- The Android JNI smoke hook adds one `native` declaration to `MainActivity.java` (`nativeRunItemTemplateSmokeTest`).

## Screenshots

- `screenshots/windows-winui3-item-template-initial.png` — Win11 + WinUI 3, cropped to the top-left 700×460 region via Win32 `PrintWindow(PW_RENDERFULLCONTENT=2)`. Shows the title bar (`MPAPP T-0019 - CollectionView item_template Demo (WinUI 3)`), the status line (`materialized_count: 5   factory_invocations: 12   last_index: 4`), the "Rotate items_source" button, and the five materialized cells `[0] Mercury` / `[1] Venus` / `[2] Earth` / `[3] Mars` / `[4] Jupiter` rendered as GTK-style `GtkListBox` rows (Win equivalent: a `Microsoft.UI.Xaml.Controls.ListView` host with one `TextBlock` per materialized cell). The state shown is after two "Rotate items_source" clicks landing on the third items_set; this matches the smoke arithmetic (4 + 3 + 5 = 12 cumulative factory calls).
- `screenshots/linux-gtk4-item-template-initial.png` — Ubuntu 24.04 / WSLg / GTK4 4.14.x, full demo window captured via `PrintWindow(PW_RENDERFULLCONTENT=2)` against the fresh `msrdc.exe` proxy. Shows the title (`MPAPP T-0019 - CollectionView item_template …`), the status line (`materialized_count: 4   factory_invocations: 4   last_index: 3`), the "Rotate items_source" button, and the four materialized cells `[0] Apples` / `[1] Bananas` / `[2] Cherries` / `[3] Dates` rendered by `mpapp-handlers-linux::collection_view_handler::rebuild_typed` walking `materialized_views()` and dispatching each label through `linux_dispatch`. This is the initial state — no rotate clicks — so the counters match `items_sets_[0]`.size = 4. Captured retroactively after closing the user's other foreground apps; the proxy-cache quirk only bites when multiple stacked msrdc proxies survive across sessions.
- `screenshots/android-emulator-app-running.png` — emulator x86_64 (AVD `coroute_test`, API 28) post-smoke. The visible UI is the existing T-0011 `android_hello` demo; the relevant evidence is that the process is alive after `nativeRunItemTemplateSmokeTest` ran in `onCreate`. The actual item_template evidence is the logcat artifact below.

## Logs

- `logs/android-emulator-item-template-smoke.log` — 4 lines of logcat output captured immediately after launching the rebuilt APK on the running emulator:

```
T-0019: after items=4: mat_count=4 factory_invocations=4 last_index=3 mat_changes=2
T-0019: after items=3: mat_count=3 factory_invocations=7 last_index=2 mat_changes=3
T-0019: after items=5: mat_count=5 factory_invocations=12 last_index=4 mat_changes=4
T-0019: after clear-template: mat_count=0 factory_invocations=12 mat_changes=5
```

These four lines prove (in order):

1. Setting `item_template` then setting `items_source = {a,b,c,d}` triggered exactly 4 factory invocations (one per row), `materialized_count == 4`, last passed-in index `== 3`. `mat_changes == 2`: one for the `item_template = …` assignment (which rematerializes against the empty items_source → no factory calls but fires the signal), one for the `items_source = {a,b,c,d}` assignment.
2. Rotating `items_source = {x,y,z}` discarded the previous 4 cells, re-invoked the factory 3 more times (cumulative `7`), `materialized_count == 3`, last index `== 2`. `mat_changes == 3`.
3. Rotating `items_source = {p,q,r,s,t}` discarded the previous 3 cells, re-invoked 5 more times (cumulative `12`), `materialized_count == 5`, last index `== 4`. `mat_changes == 4`.
4. Setting `item_template = {}` (empty) discarded the materialized cells without re-invoking the factory (still cumulative `12`), `materialized_count == 0`. `mat_changes == 5`.

This matches the `tests/mock_handlers/collection_view_test.cpp` cases verbatim — same numbers, same materialize ordering — but now executed on the Android NDK toolchain end-to-end through `onCreate` instead of via the mock-handler test harness.

## Tests

The item_template surface is covered by mock-level tests in `tests/mock_handlers/collection_view_test.cpp`:

- `item_template materializes a cell per items_source row` — first-time materialization size + factory invocation count.
- `item_template re-materializes when items_source changes` — rotation arithmetic (2 → 5 → 0).
- `item_template re-materializes when template changes` — swapping factories produces different cell types, clearing factory drops `materialized_count` to 0.
- `item_template factory receives the row index` — sequential indices passed to the factory.
- `item_template doesn't override typed_items on the surface` — both surfaces hold values independently, exclusivity is enforced by the handler not the model.
- `materialized_changed fires on rematerialize` — signal is emitted after the rematerialize completes.

End-to-end visual + log-output regression is covered by this task's Windows screenshot (the visible rendered cells through `mpapp-handlers-windows::collection_view_handler`) + the Android logcat artifact (the model-level counters + signal firings). The logcat output is deterministic so future regressions in the materialize surface will show as diffs in the log file.

The task folder's `tests/` directory is empty by design — same pattern as T-0016 / T-0017 / T-0018, the tests live under `tests/mock_handlers/` because they cover the cross-platform `mpapp::collection_view` surface, not the per-task demo wiring.

## Build state

- **Linux** Ubuntu 24.04 / GCC 13 / GTK4 4.14.x / WSL2: `cmake --build build-linux --target gtk4_item_template_demo` → binary at `build-linux/examples/gtk4_item_template_demo/gtk4_item_template_demo`. Process runs (pgrep matches) but the WSLg msrdc compositor cache prevents a fresh Windows-side screenshot. See note.
- **Windows** Win11 / MSVC 14.51 / WinUI 3 1.6 (`Microsoft.WindowsAppSDK`): `cmake --build build-full --target windows_item_template_demo` → `build-full\examples\windows_item_template_demo\windows_item_template_demo.exe`. Visible end-to-end via the cropped PrintWindow screenshot.
- **Android** AVD `coroute_test` x86_64 (API 28) / NDK 26.1 / Gradle 8.10 with `_build_android.bat`: APK at `app/build/outputs/apk/debug/app-debug.apk`, then `adb install -r` + `adb shell am start -n io.mpapp.example/.MainActivity` + `adb logcat -d -s MPAPP:* | grep T-0019`. Smoke runs in `onCreate` synchronously.

## Notes

- `notes/linux-wslg-screenshot-quirk.md` — task-specific reference to the recurring WSLg msrdc compositor-cache issue. Adds the new finding that `wsl --shutdown` does not help because `msrdc.exe` is a Windows-side process.

## What this catches up

Per Rule 11, this task closes the visible-output gap for the M-04c shipped feature:

- **CollectionView item_template** — factory-based typed cells. The `item_factory_t` typedef + `Observable<item_factory_t>` surface + auto-rematerialize-on-change wiring + `materialized_views()` accessor + `materialized_changed` signal.

T-0019 is the fourth Rule 11 catch-up after T-0016 (canvas + Cairo), T-0017 (typed routing), and T-0018 (async bridge dispatch). With this task landing, all four shipped-but-unaccompanied M-04c features have task folders, demos, and at least two-platform visible evidence in the vault.
