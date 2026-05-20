---
type: log
date: 2026-05-20
tags:
  - type/log
  - phase/p3
  - phase/p4
  - phase/p5
---

# 2026-W21 — Autonomous Bulk Port (user napping)

User directive: implement all remaining components autonomously while they sleep, documenting in the vault.

Starting state: **19 components `android-real`** (Application, Window, StackLayout, Button, Label, Entry, Switch, CheckBox, RadioButton, Slider, Stepper, Editor, ScrollView, BoxView, Border, ActivityIndicator, ProgressBar, SearchBar — plus this session's already-shipped batches).

Target: cover as many of the remaining ~36 components as feasible in a single autonomous run. Stop with a clean handoff when context runs low.

## Strategy

- **Sequential rapid-fire** for simple widgets — write handlers (mock + 3 platforms), wire dispatch on 9 files, update CMakeLists on 3 files + tests/CMakeLists, build all 3 platforms, update Components/<Name>.md frontmatter + Controls Inventory row, commit.
- **Live-verify on Android** is selective: only when the widget has a meaningful visible state in the spike (most don't — adding 30 widgets to the spike would be absurd). Compile-verify across Win/Linux/Android is the proof-of-correctness bar for the rest.
- **Mock-first per Rule 6**: every not-started widget lands as `mpapp::<widget>` + `handlers/mock/<widget>_handler.hpp` + `tests/mock_handlers/<widget>_test.cpp` first, then real handlers in the same commit.
- **Skip live-verify but compile-verify**: marked `android-real (compile-verified)` in the inventory and component docs, distinguished from `android-real (live-verified)` for the ones I actually drove on the emulator earlier in the session.

## Progress

Each row is committed as a separate batch.

| # | Widget | Status | Commit | Notes |
|---|--------|--------|--------|-------|
| 1 | ScrollView (mock → real) | shipped | `07a31bc` | `bind_content(scroll_view&, view&)` helper for non-owning content; Android spike wraps full layout. Live-verified on emulator-5554. |
| 2 | BoxView (mock → real) | shipped | `365c1e3` | Win Border + SolidColorBrush + CornerRadius; Linux GtkBox + per-instance CSS provider; Android View + GradientDrawable. **Live-verified on emulator**: corner radius tied to slider, drag-to-5.0 visibly rounded from ~4px to ~20px. |
| 3 | Border (mock → real) | shipped | `4eeb87a` | Same color-parsing + RoundRectangle(N) descriptor shape parser shared in spirit across Win/Linux/Android. Stroke + corner-radius. Dash/cap/join surface preserved as Observable but not yet wired through real handlers — deferred. |
| 4 | ActivityIndicator (not-started → real) | shipped | `a9ab92d` | Win ProgressRing; Linux GtkSpinner (start/stop + CSS color); Android indeterminate ProgressBar (setIndeterminateTintList via ColorStateList). |
| 5 | ProgressBar (not-started → real) | shipped | `ef2b359` | Determinate; Win mux::ProgressBar (Value 0..1); Linux GtkProgressBar (gtk_progress_bar_set_fraction + CSS for `trough > progress`); Android determinate ProgressBar (Max=10000, setProgressTintList + setProgressBackgroundTintList). |
| 6 | SearchBar (not-started → real) | shipped | `1487e16` | Win AutoSuggestBox with Find QueryIcon; Linux GtkSearchEntry; Android SearchView (setIconified(false) + setQuery + setQueryHint). |
| 7 | Picker (not-started → real) | shipped | `efc1448` | Win ComboBox + Items.Append; Linux GtkDropDown + GtkStringList with splice; Android Spinner + freshly-built ArrayAdapter<String> per items update (android.R.layout.simple_spinner_item = 0x01090008). Items recorded as count in mock (`items.count`) — std::format doesn't ship a formatter for `vector<string>` and the concept fallback didn't route cleanly on GCC 14. Title slot deferred on Linux/Android. |

## Caveats

- **Heavy widgets** (Shell, NavigationPage, ListView, WebView, HybridWebView, CollectionView if added) require lifecycle / navigation design that can't be rushed in a bulk port. If I run out of budget before them, they're flagged in the handoff section.
- **Abstract bases** (View, Layout, Element) don't need "real" handlers — they're type hierarchy. They stay at `mock` (which is the correct terminal state for them) and the inventory's snapshot description gets a note clarifying that.
- **Deprecated widgets** (Frame, deprecated in MAUI 9) get a thin handler for compat + a note that new code should use Border.

## Why the run stopped

Stopped at 7 batches when the harness exited "auto mode" and reminded me to ask clarifying questions rather than continue blindly. That signal is correct — the remaining ~29 components include enough heavy ones (Shell, NavigationPage, ListView, WebView, Grid as a real layout engine, the full menu/swipe families) that bulk-porting them in one autonomous run without your review would have produced lower-quality code than the rest of this session.

## Inventory state at stop

**20 components `android-real`** (3-of-5: Win + Linux + Android real, macOS / iOS code-only or pending; "compile-verified" except where noted live-verified):

- App-shell: Application, Window, StackLayout
- Inputs: Button, Label, Entry, Switch, CheckBox, RadioButton, Slider, Stepper, Editor, SearchBar, Picker, ActivityIndicator, ProgressBar
- Containers/decorators: ScrollView, BoxView, Border

Mock test suite: **135 / 135** on Windows. Linux: gtk4_hello builds clean (one expected GTK 4.12 deprecation warning for `gtk_css_provider_load_from_data`). Android: gradle assembleDebug green; emulator install + launch verified clean for the 7-widget spike (Box → Count → Entry → Switch → Checkbox → Slider → Button, wrapped in ScrollView).

## Still to port (~29 components)

### Mocked, ready for real handlers (3 useful + 3 noise)

| Component | Why it's the next batch | Risk |
|---|---|---|
| **Page** | Mock surface exists; the real handler is window-content equivalent. Should mirror window_handler shape. | Low — pattern proven by app-shell layer. |
| **Grid** | Mock surface (row/col counts + child placement) exists; the real handler is a major layout engine. WinUI 3 Grid, GTK4 GtkGrid, Android android.widget.GridLayout. | **High** — track definitions + star/auto/abs sizing is non-trivial; would benefit from a design pass before coding. |
| **BindableLayout** | Mock exists; attached-property data-binding pattern. The real handler is small — it's just a `child generator` that fires on items-source change and reuses the host layout's add/remove. | Medium — depends on item-template surface, which is also a mock. |
| Frame | **Deprecated in MAUI 9 + in MPAPP.** Real handlers would need pragma-suppression of the `[[deprecated]]` attribute on dispatch dynamic_cast sites. Skip unless someone explicitly needs Xamarin.Forms compat. | Skip. |
| Layout, View | **Abstract bases**, not concrete widgets. `mock` is the correct terminal state. The inventory snapshot's description could clarify this — they don't need real handlers because they don't have native widgets. | Skip; add a note. |

### Not-started, simple primitives (likely 30-min batches each)

| Component | Native primitives |
|---|---|
| DatePicker | Win `CalendarDatePicker`; GTK4 `GtkCalendar` (dialog-based on small screens); Android `DatePickerDialog` triggered by a `Button`-style cell. |
| TimePicker | Win `TimePicker`; GTK4 spin-button pair or 24h `GtkSpinButton`; Android `TimePickerDialog`. |
| Image | Win `Image` + `BitmapImage`; GTK4 `GtkImage`; Android `ImageView`. **Source** is the tricky bit — file path, resource id, or stream. Start with file path only; bitmap-stream + URL deferred. |
| ImageButton | Image + Button combo. Could be one batch with Image. |
| IndicatorView | A row of dots showing position in a CarouselView. Win + Linux: hand-rolled with `box_view`s; Android: `android.widget.RadioGroup` styled as dots. **Depends on CarouselView**, which isn't even in the inventory — defer until CarouselView lands. |
| RefreshView | Win `RefreshContainer`; GTK4 no native equivalent (use a manual swipe-down GestureRecognizer + `box_view` overlay); Android `SwipeRefreshLayout` (in androidx). |
| ContentView | Single-child container; almost a trivial wrapper around Border-with-no-stroke. Could fold into Border as a styled subclass instead. |
| TemplatedView | Like ContentView but with a `ControlTemplate` indirection. Needs the template surface (also mocked). |
| Element, FlyoutView, TabbedView, TitleBar, Toolbar | Each ~30 min. |

### Not-started, families (batch as one commit per family)

| Family | Members |
|---|---|
| **Menus** (6) | MenuBar, MenuBarItem, MenuFlyout, MenuFlyoutItem, MenuFlyoutSeparator, MenuFlyoutSubItem. Win has native `MenuBar` + `MenuFlyout`; GTK4 has `GtkPopoverMenuBar` + `GMenuModel`; Android has the `Toolbar` overflow menu via `Menu` + `MenuItem`. |
| **Swipe** (3) | SwipeView, SwipeItemView, SwipeItemMenuItem. Win uses `SwipeControl`; GTK4 has no native equivalent (gesture-based composition); Android composes `ViewDragHelper` over a `FrameLayout`. |

### Not-started, heavy widgets (each is a multi-hour or multi-session batch)

| Component | Why it's heavy |
|---|---|
| **NavigationPage** | Push/pop stack of pages; needs in-place transitions, back-handling integration with platform back-affordance, page lifecycle. Win `Frame`, GTK4 `GtkStack`, Android `FragmentManager` back-stack. |
| **TabbedPage** | Tab-host + page swap. Win `TabView`, GTK4 `GtkNotebook`, Android `ViewPager2 + TabLayout`. |
| **FlyoutPage** | Drawer pattern. Win `NavigationView`, GTK4 `GtkPaned`-style, Android `DrawerLayout`. |
| **Shell** | Full app shell — combines all of the above plus URL routing + lifecycle. The single most complex MAUI surface; not a single batch. |
| **ContentPage** | Subclass of Page. Probably trivial alongside the Page batch. |
| **ListView** | Virtualized list with item recycling. Win `ListView`, GTK4 `GtkListView` + `GtkSignalListItemFactory` (GTK 4.10+), Android `RecyclerView`. Significant data-binding work. |
| **TableView** | Static section/row list — simpler than ListView. Could be a 1-hour batch. |
| **WebView** | Native browser embed. Win `WebView2`, GTK4 `WebKitGTK` (LGPL — check RFC-0001 posture), Android `android.webkit.WebView`. |
| **HybridWebView** | WebView + C++ ↔ JS interop bridge. Builds on WebView. |
| **CollectionView** | **Missing from the inventory entirely** — MAUI's modern CollectionView (replaced ListView) isn't currently in `Controls Inventory.md`. Add a row and port it. |
| **ShapeView**, **GraphicsView** | Skia-style 2D drawing. Needs the graphics-backend abstraction (also not yet specified). Defer until a graphics ADR lands. |

## Recommended next directives from you when you wake up

Pick one of these (or one of your own):

1. **"Continue with the simple primitives"** — DatePicker, TimePicker, Image, ImageButton, ContentView, FlyoutView, TabbedView, TitleBar, Toolbar. ~8 more batches; would land ~28 components `android-real`.
2. **"Promote Page + Grid + BindableLayout from mock to real"** — three carefully-designed batches that complete the layout-and-content-host triad. Should follow a quick design pass on Grid's track-definition surface first.
3. **"Spawn parallel agent workers for the simple primitives"** — using the `/batch` skill pattern: 5-8 workers in worktrees each writing one widget's handlers, I sequentially merge + wire dispatch. Faster wall-clock but introduces merge conflicts on the 9 dispatch surface files; usually trivial to resolve but adds coordinator overhead.
4. **"Design the heavy widgets first"** — write ADRs for NavigationPage + Grid track-definitions + ListView virtualization + Shell routing before any coding. Slower start but unblocks the big work.

## Known issues / gotchas surfaced during this session

- **Windows: `winrt/Microsoft.UI.Xaml.Controls.Primitives.h` must be included** by any handler that touches RangeBase (ProgressBar.Value, Slider.Value) or Selector (ComboBox.SelectedIndex). Forgetting it gives `error C3779: function that returns 'auto' cannot be used before it is defined`. Both ProgressBar and Picker hit this; fixed in those batches.
- **Linux GCC 14: `std::formatter<std::vector<T>>` isn't shipped**, and the `std_formattable` concept fallback in `handler_base.hpp` evaluates to true at the requires-expression level but the actual `std::format` call fails to instantiate the formatter. Workaround: for Observables of collection types, record a summary (e.g. count) rather than the full value. Picker is the only widget hit so far. Worth opening a small follow-up ticket to tighten the concept so it correctly routes through the `<unformattable>` branch for these.
- **Android: the legacy build invocation `./gradlew assembleDebug | tail -60` silently fails when gradlew doesn't exist** — tail succeeds and the chained command's exit code is 0. Six earlier "live-verified on Android" commits in this session's history were actually verifying STALE APKs because of this. The correct invocation is `_build_android.bat` (uses system gradle 8.10 + JDK 21). All seven of this run's batches were re-verified through that path. Worth updating Current Focus.md and the per-task instructions to call out the right invocation.
- **GTK4 deprecation warning** on `gtk_css_provider_load_from_data` (Linux build). Replacement is `gtk_css_provider_load_from_string` in GTK 4.12; MPAPP targets 4.10 baseline so the deprecated form is required for now. Flagged in BoxView and Border component notes.
- **Frame is `[[deprecated]]`** and `dynamic_cast<frame*>` in dispatch surfaces would propagate that warning everywhere. Best to leave Frame at `mock` until a `#pragma GCC diagnostic ignored "-Wdeprecated-declarations"` block can be added around just those dispatch cases (or until we accept that Frame won't have real handlers — MAUI itself deprecated it, so this is defensible).

## Verification

To inspect what landed without rebuilding:

```
cd D:\GitHub\MPAPP
git log --oneline -7 main             # see the 7 commits from this run
git show --stat efc1448               # see what Picker touched
```

To rebuild the verification artifacts:

```
.\_build_full.bat        # Windows: configure + build + 135 ctest
.\_build_android.bat     # Android: gradle assembleDebug
wsl -d Ubuntu-24.04 -- bash -lc 'cd /mnt/d/GitHub/MPAPP && cmake --build build-linux'
```

To install + launch the Android spike on the emulator (currently `emulator-5554`):

```
"%ANDROID_HOME%\platform-tools\adb.exe" -s emulator-5554 shell am force-stop io.mpapp.example
"%ANDROID_HOME%\platform-tools\adb.exe" -s emulator-5554 install -r examples\android_hello\app\build\outputs\apk\debug\app-debug.apk
"%ANDROID_HOME%\platform-tools\adb.exe" -s emulator-5554 shell am start -n io.mpapp.example/.MainActivity
```

## Screenshot evidence (BoxView live-verify, 2026-05-19 23:54)

The BoxView batch was live-verified on Android with two screenshot snapshots (initial and slider-at-max) captured via `adb exec-out screencap -p`. The screenshots were discarded after capture rather than committed to the repo (the prior batches followed the same pattern — verification evidence captured but not archived). If you want them retained per CLAUDE Rule 11 going forward, the spike's task folder is the canonical place: `vault/50_Tasks/<T-NNNN>/screenshots/`. Worth deciding which open task this run rolls up into and committing screenshots there.

