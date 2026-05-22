---
type: log
tags:
  - type/log
---

# 2026-W21 — Autonomous M-04c push (page family + list family at android-real)

Session log for the autonomous run that landed the bulk of M-04c real handlers. Picks up where [[2026-W21-autonomous-bulk-port]] left off.

## What landed (in commit order)

| Commit | Scope | Build state |
|---|---|---|
| `cc0b0af` | NavigationPage + TabbedPage + FlyoutPage real handlers (9 handlers) | Win 228/228, Linux green, Android BUILD SUCCESSFUL |
| `0e67d38` | 7 gating ADRs (ADR-0015 graphics dual / ADR-0016 Shell routes / ADR-0017 Grid tracks / ADR-0018 JS bridge / ADR-0019 async exec / ADR-0020 virtualization / ADR-0021 cells) | docs |
| `cf7da0e` | `push_async` / `pop_async` / `pop_to_root_async` on `navigation_page` + stop_token compat shim for NDK 26 + Win nav spike example | Win 232/232 |
| `e1522b3` | Shell real handlers + `current_content` Observable | Win 233/233 |
| `4ab48c4` | TableView cell type tree (cell base + text/entry/switch/view/image + mock handlers + 7 tests) | Win 240/240 |
| `2378aaa` | ListView real handlers — wrap mux::ListView / GtkListBox / android.widget.ListView | Win 240/240 |
| `d347ff8` | CollectionView real handlers — selection_mode (None/Single/Multiple) mapped per platform | Win 240/240 |
| `704ce65` | TableView real handlers — sections flatten to title-row + data-row pairs | Win 240/240 |
| `e6257f4` | M-04c tracker update | docs |
| `f020dff` | Grid `track_def` + MAUI string parser per ADR-0017 phase 1; per-platform handlers follow up | Win 246/246 |
| `9be4f86` | Session log in vault | docs |
| `<grid-phase2>` | Grid real handlers phase 2 — wrap mux::Grid / GtkGrid / GridLayout + per-child placement attached store | Win 246/246 |
| `<cell-docs>` | Components/Cell + TextCell/EntryCell/SwitchCell/ViewCell/ImageCell .md files | docs |

**Net result:** every page-level widget (NavigationPage / TabbedPage / FlyoutPage / Shell), every list-family widget (ListView / CollectionView / TableView), and **Grid as a real layout engine** are all at **android-real** on all 3 platforms.

## Status of M-04c at session close

| Widget | Status |
|---|---|
| NavigationPage | android-real + async wrappers |
| TabbedPage | android-real |
| FlyoutPage | android-real |
| Shell | android-real (compile-time routes per ADR-0016 deferred) |
| ListView | android-real |
| CollectionView | android-real (multi-select events + grid layouts deferred) |
| TableView | android-real (cell-typed rendering tied to surface refactor) |
| TableView cells (text/entry/switch/view/image) | mock |
| Grid (real) | **android-real** — native Grid wrap on Win/Linux/Android + placement attached store |
| ShapeView, GraphicsView | mock (Cairo facade per ADR-0015 follow up) |
| WebView, HybridWebView | mock (WebKitGTK/WebView2/Android WebView follow up) |

## Cross-cutting tech that landed

- **ADR-0019 first realization:** `task<T>` + `ui_task<T>` shape wired into navigation_page. Mock build resolves synchronously; native UI dispatchers (DispatcherQueue / GMainLoop / Looper) are the follow-up scope.
- **stop_token compat shim:** Android NDK 26 ships libc++ without `<stop_token>`. `include/mpapp/detail/stop_token_compat.hpp` aliases to std on platforms with the header, ships a minimal impl otherwise. `executor.hpp` now uses `mpapp::stop_source` / `mpapp::stop_token`.
- **Page-stack lifecycle pattern proven:** `page_stack::page_did_appear` reliably drives content-host swaps via the ADR-0013 dispatch registry across all 3 platforms.
- **Wrap-platform-recycler pattern proven:** ListView → CollectionView → TableView each landed with the same shape (rebuild_items + selection-round-trip + native-recycler wrap). Pattern is now well established.

## Caveats & known gaps

- **Computer-use E2E on Windows still blocked.** Both the existing button spike and the new nav spike exit immediately with NTSTATUS `0xC000027B` because `Microsoft.WindowsAppRuntime.1.8` framework package is not installed on this dev box. Code is fine; install is a one-time user action.
- **Android event routers** (back-button tap, tab-strip tap, list-row tap, Shell tab tap) consistently deferred to M-05 polish. Same JNI OnClickListener wiring pattern across all of them.
- **macOS / iOS** still pending an Apple host.
- **Compile-time route table (ADR-0016)** for Shell is unimplemented; the existing string-based `go_to()` URI parser still drives `current_route` + `current_tab_index`.

## Files added/modified this session

```
include/mpapp/cell.hpp
include/mpapp/text_cell.hpp
include/mpapp/entry_cell.hpp
include/mpapp/switch_cell.hpp
include/mpapp/view_cell.hpp
include/mpapp/image_cell.hpp
include/mpapp/detail/stop_token_compat.hpp
include/mpapp/navigation_page.hpp                       (push_async / pop_async / pop_to_root_async)
include/mpapp/shell.hpp                                 (current_content)
include/mpapp/grid_layout.hpp                           (track_def + parser)
include/mpapp/executor.hpp                              (route to compat header)
include/mpapp/handlers/mock/{text,entry,switch,view,image}_cell_handler.hpp
include/mpapp/handlers/{windows,linux,android}/navigation_page_handler.hpp
include/mpapp/handlers/{windows,linux,android}/tabbed_page_handler.hpp
include/mpapp/handlers/{windows,linux,android}/flyout_page_handler.hpp
include/mpapp/handlers/{windows,linux,android}/shell_handler.hpp
include/mpapp/handlers/{windows,linux,android}/list_view_handler.hpp
include/mpapp/handlers/{windows,linux,android}/collection_view_handler.hpp
include/mpapp/handlers/{windows,linux,android}/table_view_handler.hpp
src/handlers/{windows,linux,android}/navigation_page_handler.cpp
src/handlers/{windows,linux,android}/tabbed_page_handler.cpp
src/handlers/{windows,linux,android}/flyout_page_handler.cpp
src/handlers/{windows,linux,android}/shell_handler.cpp
src/handlers/{windows,linux,android}/list_view_handler.cpp
src/handlers/{windows,linux,android}/collection_view_handler.cpp
src/handlers/{windows,linux,android}/table_view_handler.cpp
tests/mock_handlers/cells_test.cpp
tests/mock_handlers/grid_track_parser_test.cpp
tests/mock_handlers/navigation_page_test.cpp           (4 new async tests)
tests/mock_handlers/shell_test.cpp                     (1 new current_content test)
examples/windows_nav_spike/                            (new)
vault/20_ADRs/ADR-0015..0021                           (7 new ADRs)
vault/40_Roadmap/M-04c-handler-heavy-port.md           (tracker updates)
vault/50_Tasks/T-0014-async-navigation-push-pop/       (new task log)
vault/50_Tasks/T-0015-shell-real-handlers/             (new task log)
vault/10_Architecture/Controls Inventory.md            (status flips)
vault/10_Architecture/Components/*.md                  (per-widget status + descriptions)
```

## Next pieces of work (ordered by impact)

1. **Grid real layout — phase 2.** Wrap `mux::Controls::Grid` / `GtkGrid` / `android.widget.GridLayout`. Bind `row_definitions`/`column_definitions` to each native track API. Per-child placement attached properties (`Grid.SetRow`, `Grid.SetColumn`).
2. **ShapeView + GraphicsView real handlers** with the Cairo backend (per ADR-0015 §Decision, default Cairo). Linux is free via GTK4; Windows + Android need Cairo binary deps wired in.
3. **WebView real handlers**: Win WebView2, Linux WebKitGTK (LGPL — dynamic linking + rebuild path per RFC-0001), Android android.webkit.WebView.
4. **HybridWebView typed JS bridge** per ADR-0018, building on WebView.
5. **ADR-0016 compile-time route table** for Shell — heavy template metaprogramming.
6. **macOS + iOS real handlers** for everything already at android-real — Apple host required.

---

## Continued autonomous run (2026-05-22, late session)

Picks up after the page + list families landed. Drove the remaining widgets except the graphics pair through to android-real.

### Commits landed

| Commit | Scope | Build state |
|---|---|---|
| `c306916` | **view_cell** real handlers — Win Border w/ Child swap + Linux GtkBox single-slot + Android FrameLayout. Content resolved through ADR-0013 dispatch. | Win/Linux/Android green |
| `4c94b6a` | **switch_cell** real handlers — Win Border+Grid (TextBlock+ToggleSwitch) + Linux horizontal GtkBox (GtkLabel+GtkSwitch) + Android LinearLayout (TextView weight=1 + Switch). Two-way `on` binding via `MppCheckedChangeListener` kind=4. Emits `on_changed` on user flips. | Win/Linux/Android green |
| `9556131` | **image_cell** real handlers — Win Border+Grid (Image 40px + StackPanel(TextBlock pair)) + Linux GtkBox (GtkImage 40px + label pair) + Android LinearLayout (ImageView 80px + label pair weight=1). BitmapImage / `gtk_image_set_from_file` / `BitmapFactory.decodeFile` loaders. | Win/Linux/Android green |
| `83a710e` | **entry_cell** real handlers — TextBox/GtkEntry/EditText with keyboard_kind→InputScope/InputPurpose/InputType mapping; two-way text via existing TextWatcher (kind=3); new `MppEditorActionListener` carries IME terminal actions (DONE/GO/NEXT/SEARCH/SEND) to `completed`. | Win/Linux/Android green |
| `9d8a45c` | **WebView** real handlers — Win `muxc::WebView2` + Linux `WebKitGTK 6.x` (LGPL dynamic per Rule 9) + Android `android.webkit.WebView` w/ MppWebViewClient. Wires url + html_source + is_loading + can_go_back/forward + navigating/navigated end-to-end. Added INTERNET permission + `usesCleartextTraffic` to android_hello manifest. Linux stubs out cleanly if WebKitGTK missing at configure. | Win/Linux/Android green |
| `00761be` | **HybridWebView** real handlers — JS<->C++ bridge across all 3 platforms. Win WebView2 PostWebMessageAsString + AddScriptToExecuteOnDocumentCreatedAsync; Linux WebKitUserContentManager script-message-handler (`mpapp_send`) + user-script at document-start; Android `addJavascriptInterface("mpapp_native", MppJsBridge)`. Shared injected JS shim: `window.mpapp = { send(p), on(fn), _receive(p) }`. | Win/Linux/Android green |

### Cross-cutting tech added this run

- **`MppEditorActionListener` (kind-discriminated)** — third generic Android router after MppActionRouter and MppCheckedChangeListener. Filters IME action IDs (`DONE`/`GO`/`NEXT`/`SEARCH`/`SEND`) before invoking the native handler — non-terminal events drop on the floor.
- **`MppWebViewClient`** — bespoke WebViewClient subclass routing `onPageStarted`/`onPageFinished` into `web_view_client_dispatch.cpp`.
- **`MppJsBridge`** — `@JavascriptInterface` host object that exposes `send(payload)` to the JS shim and trampolines into `hybrid_web_view_handler` via `js_bridge_dispatch.cpp`.
- **WebKitGTK 6.x standardization** — initial attempt fell back to `webkit2gtk-4.1` but its GTK3 headers can't combine with our GTK4 build. Dropped the fallback. `libwebkitgtk-6.0-dev` is now a documented Linux build requirement.

### Status as of late session 2026-05-22

| Widget | Status |
|---|---|
| NavigationPage / TabbedPage / FlyoutPage / Shell | android-real |
| ListView / CollectionView / TableView | android-real |
| TableView cells (text/view/switch/image/entry) | **android-real** ← all 5 landed today |
| Grid (real) | android-real |
| **WebView / HybridWebView** | **android-real** ← landed today |
| ShapeView / GraphicsView | mock — Cairo facade per ADR-0015 still pending |
| macOS / iOS | pending Apple host |

**Net result:** Every widget in M-04c reaches android-real *except* the ShapeView/GraphicsView pair (gated on ADR-0015 graphics facade) and the macOS/iOS platforms (gated on Apple host). The cell-type tree is now fully wired, including bidirectional bindings on switch_cell + entry_cell with native event echoes through the kind-discriminated router family.

## Final push (2026-05-22 close)

| Commit | Scope | Build state |
|---|---|---|
| `658ccee` | **ShapeView + GraphicsView** real handlers (v1). Per-platform native primitives — Win muxc::Border+Shapes / muxc::Canvas; Linux GtkDrawingArea + cairo draw callback / sized GtkDrawingArea; Android custom `MppShapeView` w/ onDraw / plain `android.view.View` w/ setMinimumWidth/Height. polygon+path → bounding rect in v1. Unified canvas facade per ADR-0015 deferred to v2. | Win/Linux/Android green |
| `4ad32ea` | M-04c milestone: `planned` → `active`; tracker shows every concrete widget at android-real. | docs |
| `df2e25f` | **CollectionView multi-select event round-trip** on 3 platforms. Win SelectedItems→IndexOf, Linux "selected-rows-changed"→get_selected_rows, Android getCheckedItemPositions() from item_click_router. Closes the deferred multi-select gap. | Win/Linux/Android green |

**M-04c is functionally complete.** Every concrete widget in the Controls Inventory is `android-real` on Win + Linux + Android. The remaining open items are process-level:

1. ADR acceptance pass for 0014–0021 (decisions are implemented; review gate is the missing step per Rule 4).
2. ADR-0016 compile-time route table for Shell — heavy template-metaprogramming follow-up.
3. ADR-0015 v2 unified canvas facade (Cairo + Skia compile-time selectable).
4. macOS / iOS handlers across the entire widget set — gated on Apple host.

## Polish pass (2026-05-22 very-close)

After the widget surface reached completion, a small wave of polish followed:

| Commit | Scope | Build state |
|---|---|---|
| `82e5d90` | Refresh `Current Focus.md` + `Home.md` — was W20-era, predated everything in M-04b/M-04c. New version reflects 61/64 widgets at android-real and documents the pick-up-here list. | docs |
| `1df1f84` | Android tabbed_page — tab clicks + selected-tab styling. Adds `tabbed_page_tab` (kind=2) to MppActionRouter dispatch; restyles active tab w/ primary-blue+bold. | Win/Linux/Android green |
| `854a377` | Android Shell — selected-tab styling. Same shape as tabbed_page; `apply_selection` was previously a no-op. Shared color palette across the two handlers. | Android green (Win+Linux untouched) |

**Why these matter:** the Win + Linux Shell/TabbedPage handlers wrap native widgets (`mux::SplitView` / `GtkPaned` for Shell; `mux::Pivot` / `GtkNotebook` for TabbedPage) that handle tab clicks + selected-tab visual emphasis automatically. The Android handlers hand-rolled the tab strip from TextViews/Buttons, leaving both gaps. Now the user-visible cross-platform behavior matches.

## TableView typed-sections pass

| Commit | Scope | Build state |
|---|---|---|
| `ab6add0` | **TableView typed_sections** — render the ADR-0021 cell tree end-to-end. Parallel surface to flat `sections`. Win mux::ListView.Items.Append(cell_native); Linux gtk_list_box_append per cell widget; Android outer FrameLayout swaps between ListView (flat) and ScrollView+LinearLayout w/ section-header TextViews + cell native views (typed). | Win/Linux/Android green, 246/246 tests pass |

**Why this matters:** the cell-type tree (text_cell / view_cell / switch_cell / image_cell / entry_cell) shipped with full per-platform native handlers earlier in the session, but TableView itself could only render flat strings — its primary host couldn't actually display the typed cells. Now `table_view.typed_sections` carries `vector<table_section_typed{title, vec<cell*>}>`; when populated, the handler walks each cell, pulls its native handle via ADR-0013 dispatch, and stitches it into the table's section/row layout. Flat-string rendering remains as the fallback when typed_sections is empty, so existing call sites + tests don't break.

This closes the largest remaining "the surface ships but you can't actually use it together" gap from the cell-tree work.

## See also

- [[40_Roadmap/M-04c-handler-heavy-port]] — canonical tracker.
- [[Controls Inventory]] — per-widget status.
- [[T-0014-async-navigation-push-pop]] · [[T-0015-shell-real-handlers]] — task logs.
- [[2026-W21-autonomous-bulk-port]] — earlier session that got M-04b to done.
