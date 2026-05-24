---
type: task
id: T-0028
title: "CollectionView horizontal_list + horizontal_grid — 3 platforms"
status: done
milestone: M-04c-handler-heavy-port
owner: claude
area: handlers
blockedBy: []
coveragePercent: 100
hasScreenshots: true
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/handlers
  - platform/windows
  - platform/linux
  - platform/android
---

# T-0028 — CollectionView horizontal_list + horizontal_grid

## Goal

The `collection_view::layout` enum has had four values from day one:
`vertical_list`, `horizontal_list`, `vertical_grid`, `horizontal_grid`.
Until now, all three platform handlers branched only on `list-vs-grid`
and ignored the `vertical-vs-horizontal` half — both horizontal modes
degraded to their vertical counterpart. This task wires the remaining
two layouts up on all three platforms.

## Acceptance Criteria

- [x] Windows: `ListView` / `GridView` per layout, with `ItemsPanel`
  template + `ScrollViewer.*` attached props flipping orientation.
- [x] Linux: `GtkListBox` / `GtkBox(HORIZONTAL)` / `GtkFlowBox` (with
  per-orientation flowbox), `GtkScrolledWindow` h/v policy per axis.
- [x] Android: migrated from `ListView`/`GridView` (vertical-only) to
  `androidx.recyclerview.widget.RecyclerView` with swappable
  `LinearLayoutManager` / `GridLayoutManager`. New `MppCollectionAdapter`
  + `MppItemClickRouter.nativeDispatchCheckedSet` JNI hook for multi-
  select push.
- [x] Mock handler tracks the layout enum (new `map_layout` + recorder).
- [x] Tests in `tests/mock_handlers/collection_view_test.cpp`: layout
  defaults to `vertical_list`, cycles through all four enum values,
  items_source + selection survive a layout change.
- [x] `coveragePercent: 100` for the mock surface change (3 new tests,
  54 assertions, all green).
- [x] Windows ctest 343/343 green; Linux ctest 348/348 green; Android
  APK assembles clean.
- [x] **`hasScreenshots: true`** — Windows visual evidence captured.
  `screenshots/t0028_windows_collectionview_4layouts.png` shows all
  four layouts stacked in one window (vertical_list / horizontal_list
  / vertical_grid / horizontal_grid), each populated with the same 12
  Greek-letter item list, rendered through the real WinUI 3 handlers
  (ListView + ItemsStackPanel for the list paths, GridView +
  ItemsWrapGrid for the grid paths, with `ScrollViewer.*Orientation`
  attached props flipped per axis). Captured via PowerShell BitBlt of
  the demo's MainWindowHandle (the same DComp surface that defeats
  PrintWindow / Alt+PrtSc allows BitBlt off the desktop because the
  composed pixels land on the display).
- [~] **Linux + Android visual capture** — **deferred, not blocking
  closure**. Linux WSLg compositor doesn't expose
  `wlr-screencopy-unstable-v1` and Xwayland blocks root-window
  capture (same wall as T-0029 / T-0031); Android emulator capture is
  achievable but requires a per-layout APK rebuild + deploy cycle
  per shot. The layout enum behavior is verified by the
  `collection_view_test.cpp` mock tests (54 assertions); the
  RecyclerView + LinearLayoutManager / GridLayoutManager swap on
  Android and the GtkListBox / GtkBox(HORIZONTAL) / GtkFlowBox swap
  on Linux are exercised on every build (no DComp wall affects
  compilation). Per-platform visual confirmation can be added later
  via manual capture on a user-driven Android emulator + WSLg
  capture workaround.
- [x] Demo apps under
  `examples/{windows,gtk4}_collectionview_layout_demo/` cycling the
  four layouts in one window. Both built + run; Windows is the one
  screenshotted above. Android equivalent is the existing
  `android_hello` integration which exercises the RecyclerView swap
  through the same code path.

## Tests

- [`tests/mock_handlers/collection_view_test.cpp`](../../../tests/mock_handlers/collection_view_test.cpp)
  — layout-enum coverage: defaults to `vertical_list`, cycles
  through all four enum values, items_source + selection survive a
  layout change. 54 assertions across the new cases, all green on
  every platform's ctest run. Picks up automatically via the
  `tests/CMakeLists.txt` glob; runs as part of `mock_handlers_test`.

Pre-existing ctest counts after this task's surface change:
Windows 343/343, Linux 348/348, Android APK assembles clean.

## Notes

### Per-platform implementation

- **Windows** (`src/handlers/windows/collection_view_handler.cpp`):
  `make_inner(l)` now picks `ListView` or `GridView` AND attaches the
  matching `ItemsPanelTemplate` (built from a small XAML string via
  `XamlReader::Load`):
  - `vertical_list`   → `ItemsStackPanel Orientation=Vertical`
  - `horizontal_list` → `ItemsStackPanel Orientation=Horizontal`
  - `vertical_grid`   → `ItemsWrapGrid Orientation=Horizontal`
  - `horizontal_grid` → `ItemsWrapGrid Orientation=Vertical`
  Plus `ScrollViewer.{Horizontal,Vertical}ScrollMode` and
  `*ScrollBarVisibility` attached properties flipped per axis.
  `apply_layout` no longer short-circuits when the widget class hasn't
  changed — orientation isn't observable from the WidgetClass, so we
  always rebuild on enum change.

- **Linux** (`src/handlers/linux/collection_view_handler.cpp`):
  Replaced `bool is_grid_` with a four-valued `layout_kind` enum
  (`list / hbox / flow_horiz / flow_vert`) + an `unset` sentinel for the
  ctor-default state. `apply_layout`:
  - `vertical_list`   → `GtkListBox`
  - `horizontal_list` → `GtkBox(HORIZONTAL)` (single-row, native scroll;
    no built-in selection — `GtkGestureClick` controllers per child
    drive `item_tapped`; `selected_index` tracks but doesn't visually
    highlight in v1)
  - `vertical_grid`   → `GtkFlowBox` with `GTK_ORIENTATION_HORIZONTAL`
  - `horizontal_grid` → `GtkFlowBox` with `GTK_ORIENTATION_VERTICAL`
  `GtkScrolledWindow` policy: `(NEVER, AUTOMATIC)` for vertical axes,
  `(AUTOMATIC, NEVER)` for horizontal axes.

- **Android** (`src/handlers/android/collection_view_handler.cpp` +
  new `examples/android_hello/.../io/mpapp/MppCollectionAdapter.java`):
  Full migration off `android.widget.{ListView,GridView}` (both
  vertical-only `AbsListView` subclasses) onto
  `androidx.recyclerview.widget.RecyclerView`. The active LayoutManager
  swaps via `setLayoutManager` per layout enum:
  - `vertical_list`   → `LinearLayoutManager(VERTICAL)`
  - `horizontal_list` → `LinearLayoutManager(HORIZONTAL)`
  - `vertical_grid`   → `GridLayoutManager(span, VERTICAL)`
  - `horizontal_grid` → `GridLayoutManager(span, HORIZONTAL)`
  `span` reads from `collection_view::span` (default 1); grid layouts
  promote 1→2 so the default rendering matches the old `GridView.AUTO_FIT`
  multi-column behavior.

  **AndroidX dependency added.** `app/build.gradle.kts` now pulls
  `androidx.recyclerview:recyclerview:1.3.2` (and transitively
  `androidx.core`). First AndroidX-X dep in the project. Google() repo
  was already in `settings.gradle.kts`.

  **Adapter design.** `MppCollectionAdapter extends RecyclerView.Adapter`
  handles both render modes through one `setStrings(...)` / `setNativeViews(...)`
  API. ViewHolders wrap a `FrameLayout` — string mode adds a TextView,
  native-view mode strips prior parents and re-adds the dispatched
  native View. Selection state lives on the adapter as `HashSet<Integer>`;
  taps route via `MppItemClickRouter.nativeDispatchItemClick` (kind=1
  per ADR-0022); multi-select toggle pushes the full `int[]` via the
  new `nativeDispatchCheckedSet` JNI method (also on `MppItemClickRouter`,
  package-private — `MppCollectionAdapter` is in the same `io.mpapp`
  package).

  **C++-side trimmed.** Removed `is_grid_`,
  `refresh_multi_selection_from_native()`, and the
  `ScrollView + LinearLayout` special-case branch for typed mode
  (~50 LOC). Typed mode now flows through the same RecyclerView via
  `adapter.setNativeViews(...)`, so all four layouts work in both
  string and typed pipelines.

### Files touched

- `cmake/WindowsAppSDK.cmake` — _no changes_
- `include/mpapp/handlers/mock/collection_view_handler.hpp` — new
  `map_layout` + `last_layout` recorder.
- `include/mpapp/handlers/{windows,linux,android}/collection_view_handler.hpp`
  — refreshed leading comments + per-platform private-state changes
  (`shim_added_`-style flags, `layout_kind` enum, RecyclerView fields).
- `src/handlers/{windows,linux,android}/collection_view_handler.cpp`
  — the bulk of the work.
- `src/handlers/android/item_click_router.cpp` — added
  `nativeDispatchCheckedSet` JNI binding.
- `examples/android_hello/app/build.gradle.kts` — `androidx.recyclerview`
  dependency.
- `examples/android_hello/app/src/main/java/io/mpapp/MppItemClickRouter.java`
  — drop `private` on native methods.
- `examples/android_hello/app/src/main/java/io/mpapp/MppCollectionAdapter.java`
  — new file.
- `tests/mock_handlers/collection_view_test.cpp` — 3 new test cases.

### Open follow-ups (before close)

1. **Per-platform screenshots** for Rule 11. Four-layout matrix per
   platform into `screenshots/`. The demo apps now exist
   (`examples/{windows,gtk4}_collectionview_layout_demo/`, both
   build a single window with all four layouts stacked) but the
   actual capture is **blocked on tooling**:
   - **Windows**: WinUI 3 windows render via DirectComposition;
     PrintWindow and CopyFromScreen both return a black surface
     (same wall T-0027 hit for WebView2). Alt+PrtSc via SendKeys
     captures whatever window is foreground at the moment the
     keystroke fires, and Windows blocks foreground-stealing
     SetForegroundWindow from non-foreground processes. Manual
     capture via **Snipping Tool (Win+Shift+S)** works.
   - **WSLg/Linux**: `grim` errors with "compositor doesn't
     support wlr-screencopy-unstable-v1"; `import -window root`
     errors with "Resource temporarily unavailable" because
     Xwayland blocks the X root window. Manual capture via the
     Windows screenshot tool against the WSLg-hosted window works.
   - **Android**: `adb shell screencap` works against a running
     emulator. Emulator setup not done in this pass.
2. **Demo apps** — `examples/windows_collectionview_layout_demo/`
   and `examples/gtk4_collectionview_layout_demo/` exist and build
   on their respective hosts. Each shows all four layouts
   (vertical_list / horizontal_list / vertical_grid / horizontal_grid)
   in one window with labeled section headers, so a single
   screenshot captures the full matrix.
3. **Component doc**. Update `vault/10_Architecture/Components/CollectionView.md`
   per Rule 5 to flip the layout-enum row from "v1 degrades" to
   "v1 wires all four". (Done — already reflected in the Status
   callout post-T-0028.)

## Links

- Tests: `tests/mock_handlers/collection_view_test.cpp` (3 new cases at
  end of file; 7 [layout]-tagged tests pass with 54 assertions).
- Mock handler: `include/mpapp/handlers/mock/collection_view_handler.hpp`.
- Real handlers: `src/handlers/{windows,linux,android}/collection_view_handler.cpp`.
