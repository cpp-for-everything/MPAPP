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

## TableView demo pass

| Commit | Scope | Build state |
|---|---|---|
| `fadc164` | Mock tests for typed_sections — 246→248 ctest. Builds `vector<table_section_typed>` from real text_cell + switch_cell instances; asserts identity, structure, and the mock recorder fires "typed_sections.count=N". | Win+Linux green |
| `563c37e` | **`examples/gtk4_tableview_demo/`** — Linux end-to-end demo. Two sections, mixed cells (text/entry/switch), live two-way bindings + completed signal. Proves the typed surface composes via GtkListBox + cell native widgets through ADR-0013 dispatch. | Linux green |
| `b97217f` | **`examples/windows_tableview_demo/`** — WinUI 3 counterpart. Same view-model code; only handler template arguments swap. Proves the typed surface composes via mux::ListView.Items.Append on a meaningfully different stack. | Win green |
| `24812f2` | **`examples/android_hello`** extended w/ 2-section TableView at the bottom of the layout (text_cell + switch_cell). Smallest-delta path — reuses the existing Gradle scaffolding instead of duplicating it in a sibling. | Android green |

**Net:** typed_sections is now demonstrated end-to-end on **all three target platforms** with identical view-model code. The cell tree composes through ADR-0013 dispatch on GtkListBox, mux::ListView, and Android's FrameLayout-wrapped ScrollView+LinearLayout. Cross-platform parity for the typed surface is now provable, not just contractual.

## TableView row→cell tap routing

| Commit | Scope | Build state |
|---|---|---|
| `afed31f` | **TableView row_tapped → cell.tapped** on 3 platforms. Win + Linux row-tap wiring was Android-only before; now all 3 platforms emit `table_view::row_tapped(section, row)` and additionally fire the targeted cell's `tapped` signal through a shared `cell_at(section, row)` helper. | Win/Linux/Android green |
| `4cabac8` | Three test cases locking in the cell_at contract (typed/flat lookups, out-of-range, simulated handler dispatch). | Win 251/251 |

The em-dash bug from the W20 era recurred in one of the new test names and broke Catch2's ctest filter — caught + fixed in the same commit.

## CollectionView vertical_grid + multi-select polish

| Commit | Scope | Build state |
|---|---|---|
| `df2e25f` | **CollectionView multi-select event round-trip** on 3 platforms. Native widget toggles a row's checked state in `selection_mode=multiple` → handler echoes the full set back into `selected_indices`. Win iterates SelectedItems via Items.IndexOf; Linux walks gtk_list_box_get_selected_rows on "selected-rows-changed"; Android calls getCheckedItemPositions() after each multi-mode tap. | Win/Linux/Android green |
| `78c4e54` | **CollectionView vertical_grid layout** on 3 platforms. Outer stable container + inner widget swap pattern: mux::Border + ListView/GridView (Win); GtkScrolledWindow + GtkListBox/GtkFlowBox (Linux); FrameLayout + ListView/GridView (Android). map_layout swaps the inner widget at runtime; SelectionMode + items + tap router transfer to the fresh inner. horizontal_* still degrades to vertical for v1. | Win/Linux/Android green |

## ADR-0022 — Android router pattern codification

| Commit | Scope | Build state |
|---|---|---|
| `6421408` | ADR-0022 codifying the kind-discriminated Android listener family pattern. Catalogs current kind allocations across MppActionRouter / MppItemClickRouter / MppCheckedChangeListener / MppTextWatcher / MppEditorActionListener / MppWebViewClient / MppJsBridge / MppNumberPickerListener / MppSeekBarChangeListener. Documents the procedure for adding a new kind on an existing listener vs allocating a brand-new listener class. | docs |

## ADR-0018 typed JS bridge — full v2 stack (5 phases)

The biggest single architectural feature shipped this session. Built bottom-up over five focused commits.

| Phase | Commit | Surface | Coverage |
|---|---|---|---|
| **A** — JSON layer | `52d579a` | `include/mpapp/detail/json.hpp` (~520 LOC) — header-only encode/decode for primitives + vector + optional + ADL extension. Writer appends to `std::string&`; reader is pull-style over `std::string_view`. | +11 ctest |
| **B** — bridge base + inbound | `85a485b` | `include/mpapp/hybrid_bridge.hpp` — `register_method` + JSON-RPC dispatcher. Sync methods; `task<T>` async deferred. Args parsed into typed `std::tuple<Args...>`; void returns write `null`. | +12 ctest |
| **C** — wire bridge into hybrid_web_view | `f1eff1d` | `hybrid_web_view::set_bridge<T>()` + `process_inbound` choke point. 3-platform handler refactor — Win/Linux/Android handlers now call `bound_->process_inbound(payload)` instead of duplicating the bridge-vs-raw decision. | +5 ctest |
| **D** — outbound invoke_js | `1488373` | `invoke_js(method, args...)` fire-and-forget. `json::writer::field_array` helper. Auto-incrementing outbound id stream separate from inbound ids. | +2 ctest |
| **E** — invoke_js_cb + response router | `3f8a62e` | `invoke_js_cb<T>(method, on_result, args...)` registers a callback by envelope id; process_inbound's tri-state classifier routes responses with matching pending id to the callback. | +4 ctest |
| **F** — invoke_js_async (task<T>) | `76035e7` | `invoke_js_async<T>(method, args...)` returns an awaiter so `co_await wv.invoke_js_async<int>("add",1,2)` works inside `mpapp::task<T>` / `ui_task<T>` coroutines. | +2 ctest |
| **G** — JS shim ergonomics | `c160269` | Updated `window.mpapp` shim on all 3 platforms: `register(name, fn)` for typed JS-side methods + `call(name, args...)` for outbound. `_receive` now dispatches by method name + auto-posts result/error envelopes. | (JS code only) |

**Net:** ADR-0018 v2 is feature-complete for synchronous bridge methods. Symmetric typed round-trips end-to-end on both sides: C++→JS via `invoke_js{_cb,_async}` + JS-side `register`; JS→C++ via `hybrid_bridge` + JS-side `call`. The C++ surface supports both callback and coroutine APIs. ctest grew from 248 → 287 across the seven commits.

The only remaining gap is Phase F (the *other* Phase F — async bridge methods returning `task<T>` themselves, which requires refactoring `hybrid_bridge::dispatch` to a callback-style `dispatch_async`). Documented as a "Phase F" pickup item; not blocking since sync bridge methods cover the common case.

## ADR-0018 Phase F — async bridge method dispatch

| Commit | Scope | Build state |
|---|---|---|
| _(pending)_ | **register_async_method + dispatch_async** — `hybrid_bridge` gains an `async_invoke` field on `method_entry` alongside the existing sync `invoke`; `register_async_method<T>(name, &Self::method)` registers a method with trailing `std::function<void(T)> respond` callback that the user invokes when ready. `dispatch_async(payload, on_response)` handles both kinds uniformly — sync methods fire `on_response` inline (preserving v1 behavior); async methods defer until the user method's `respond()` resolves. `hybrid_web_view::process_inbound` now routes through `dispatch_async` so both shapes go through the same code path. `async_invoker_builder<T, Method>` is a partial specialization on the full member-function-pointer type that internally splits `A...` into `Args... + std::function<void(T)>` via `index_sequence<N-1>` — sidesteps the "pack in non-trailing position is non-deducible" rule that bit both MSVC and GCC on the naïve formulation. shared_ptr<bool> "fired" guard on `respond` makes double-fire a silent no-op. Tests cover sync-through-async-path, async with sync respond, async with deferred respond, double-respond, args-mismatch, unknown-method, malformed-envelope, and end-to-end through `hybrid_web_view::process_inbound`. | Win 302/302 + Linux green + Android APK builds clean |

**Net:** ADR-0018 is now fully complete in both directions. Bridge methods can be sync (return-by-value) or async (capture-and-defer-respond). The dispatch_async path is the canonical one going forward; the original sync `dispatch()` remains for callers who only need the synchronous return-string shape. ctest 290 → 302 across the +12 new async test cases; the suite stays green on every platform.

## ADR-0016 — compile-time Shell route table

| Commit | Scope | Build state |
|---|---|---|
| _(pending)_ | **route_table NTTP** — `include/mpapp/detail/fixed_string.hpp` (C++20 class-type NTTP wrapper) + `include/mpapp/route.hpp` (`param<"name", T>`, `route<"path", Page, Params...>`, `route_table<Routes...>`). `shell::go_to<Path, &Table>(args...)` templated entry point that static_asserts route-not-found, arg count, and per-arg type convertibility; falls through to the existing string-based `go_to(uri)` after building the `//path?p1=v1&p2=v2` URI so `current_route` / `current_tab_index` / `navigated` semantics are preserved. The string-based parser was extended to cut at `?` (in addition to `/`) so the typed path's query strings don't break tab matching. Route finder is index-sequence based rather than recursive partial specialization → clean static_assert diagnostic. ADL-customizable `to_route_string` for argument stringification (built-ins for int/long/long long/unsigned/double/bool/string). 6 ctest cases + ~15 compile-time `static_assert`s. | Win 308/308 + Linux 308/308 + Android APK builds clean |

**Net:** the compile-time path is open for new C++ code without breaking the runtime path that the XAML compiler still uses. Apps can mix string-based and typed `go_to` freely. Two remaining ADR-0016 follow-ups (route guards, route-aware lifecycle) need the executor (already shipped per ADR-0019) and are tracked as future work.

## CollectionView item_template — factory-based typed cells

| Commit | Scope | Build state |
|---|---|---|
| _(pending)_ | **CollectionView::item_template** — `Observable<function<unique_ptr<view>(int)>>` parallel surface alongside items_source + typed_items. When the factory is set, the collection_view auto-materializes one cell per items_source row (owned in `std::vector<unique_ptr<view>> materialized_`), then emits a new `materialized_changed` signal. The three platform handlers extend `rebuild_active()` with a new precedence: typed_items → materialized_views() → flat items_source. Each map_typed_items now also subscribes to `materialized_changed` so template re-fires drive a fresh typed-render through the existing pipeline. Functor-struct callback pattern (rather than ad-hoc lambdas) keeps the callback addresses stable for the slot lifetime — the same trick the existing handlers use. Tests cover materialize-on-items-change, materialize-on-template-change, factory receives index, coexists with typed_items, and the materialized_changed signal-fire contract (including empty-state fires). | Win 266/266 mock + Linux 266/266 mock + Android APK builds clean (ctest 314 total) |

**Net:** apps with small-to-medium typed-cell lists can declare `cv.item_template = [](int i) { ... };` instead of manually managing a parallel `vector<unique_ptr<cell>>`. The framework owns the cells, re-materializes on items_source/item_template change, and surfaces them through the existing typed-render path on each platform. True virtualization (RecyclerView / ItemsRepeater) remains future work — item_template materializes the entire items_source eagerly.

## Shell route guard — can_activate

| Commit | Scope | Build state |
|---|---|---|
| _(pending)_ | **shell::can_activate** — `Observable<function<bool(string_view target)>>` consulted by `go_to(uri)` before navigation. False aborts: `current_route` stays where it was, `navigated` does NOT fire, but a new `navigation_blocked` signal emits the rejected target URI. The typed `go_to<Path, &Table>(args...)` delegates to the string-based path so the guard fires for both. 3 ctest cases: block on false, proceed on true, applies to typed `go_to` too. ctest 314 → 317. | Win + Linux + Android green |

**Net:** closes ADR-0016's "route guards deferred to follow-up ADR" item for the simpler activate case. `can_deactivate` (current-route-aware) and route-lifecycle hooks (`OnNavigatedTo`/`OnNavigatedFrom`) remain future work tied to ADR-0019's executor.

## Shell can_deactivate + page lifecycle hooks (ADR-0023)

| Commit | Scope | Build state |
|---|---|---|
| `db70b07` | **shell::can_deactivate** + **page::navigated_to / navigated_from** — two-phase guard (deactivate first, activate second), then page-level lifecycle signals fired on each successful go_to. `navigated_from` fires on the outgoing page with the PREVIOUS URI before current_route updates; `navigated_to` fires on whoever is current_content after the update with the new URI. Both guards short-circuit on false → emit `navigation_blocked` → no state change, no lifecycle. 7 new ctest cases. ctest 317 → 324. | Win + Linux + Android green |
| `c0fa777` | **ADR-0023** (proposed) — captures the full guards + lifecycle design that ADR-0016 deferred. Documents: two-phase chain, ordering invariants, sync-only rationale, no first-class param dict (deferred). | docs |

**Net:** the full Shell-navigation contract from ADR-0014 + ADR-0016 + ADR-0023 is now live and tested. The remaining route_table follow-ups (positional path params, `parse_args<Path>(uri)`) are independent additive work.

## ADR acceptance pass — flip 8 of the 9 proposed (ec0997e)

After the W21 push closed out the implementation work for the proposal wave, the user (deciders: alex) signed off on the eight ADRs whose code is fully shipped + tested across all three platforms:

| ADR | Title | Implementation evidence |
|---|---|---|
| **ADR-0014** | Page navigation stack semantics | page_stack engine + async push/pop wrappers shipped earlier in W21 |
| **ADR-0016** | Shell URI routing — compile-time route table | fixed_string NTTP + route_table just landed this session |
| **ADR-0017** | Grid track definitions — value type + parser | track_def + 3-platform handlers shipped earlier |
| **ADR-0018** | HybridWebView JS bridge — typed async method calls | Phases A-F + JS shim, full v2 round-trips end-to-end |
| **ADR-0019** | Async executor — native UI dispatcher + task<T> | task<T> / ui_task<T> + test_dispatcher shipped earlier |
| **ADR-0020** | Virtualized item host — wrap platform recyclers | ListView + CollectionView + TableView shipped + typed_items + item_template |
| **ADR-0021** | TableView cell type tree — full MAUI parity | All 5 cells (text/view/switch/image/entry) real on 3 platforms |
| **ADR-0022** | Android event routing — kind-discriminated listener family | Pattern codification of work already shipped across the cell + view handlers |

Per Rule 4, these are now immutable — future changes require a new ADR with `supersedes:`.

**Still `proposed`:** ADR-0015 (graphics backend dual — v1 ships but v2 facade hasn't been built yet) and the just-opened ADR-0023 (route guards + lifecycle — implementation just landed but waiting on a separate decider beat to flip).

## ADR-0015 v2 — canvas facade (stub backend)

| Commit | Scope | Build state |
|---|---|---|
| `91d1c43` | **canvas facade + stub backend** — abstract `mpapp::detail::graphics::canvas` interface with `color` / `point` / `size` / `rect` / `path` / `line_cap` / `line_join` value types; SVG-subset path parser (M/L/Q/C/Z); hex color parser (#RRGGBB + #RRGGBBAA, fail-quiet); paint state setters + draw ops (fill/stroke rect/ellipse/path + clip + clear) + state-stack + transforms. Stub backend records every call as a string for testing — also doubles as the default backend so the framework compiles everywhere with zero native dependency. `make_canvas(w, h)` factory. CMake option `MPAPP_GRAPHICS_BACKEND` (`stub` default; `cairo` / `skia` accepted but warn-and-fall-back-to-stub for now). 16 ctest cases + 68 assertions. ctest 324 → 340. | Win + Linux + Android green |

**Net:** the v2 facade surface is locked. Apps and handlers can now paint through `canvas*` without coupling to any specific backend. The remaining v2 work — real Cairo + Skia backends + the ShapeView/GraphicsView migration — is decoupled from the facade design and ships incrementally. ADR-0015 stays `proposed` until the first real backend ships, but the implementation-notes section now documents the facade layout.

## ADR-0015 v2 — Cairo backend lands (Linux default)

| Commit | Scope | Build state |
|---|---|---|
| _(pending)_ | **Real Cairo backend** — `src/detail/graphics/cairo_backend.cpp` implements the full `canvas` interface against `cairo_image_surface_t`. Every method maps to its Cairo equivalent: state stack → `cairo_save`/`cairo_restore`, transforms → `cairo_translate`/`scale`/`rotate`, fill/stroke colors capture the latest `set_*` value and apply via `cairo_set_source_rgba` immediately before each draw op (Cairo's source-pattern model differs from the facade's separate-fill-and-stroke split), `quad_to` upconverts to Cairo's cubic form via the standard 2/3 control-point lift, ellipse uses translate-scale-arc. CMake: `pkg_check_modules(MPAPP_CAIRO QUIET cairo)`; backend default flips to `cairo` on Linux when libcairo found; otherwise falls back to stub with a CMake warning so the build stays green on platforms that don't yet bundle Cairo. Include + link wired PUBLIC on mpapp-core so test code can include `<cairo/cairo.h>` for pixel-readback verification. LGPL-2.1 via dynamic link per RFC-0001 §Linux pattern. 7 new ctest cases gated on `MPAPP_GRAPHICS_HAS_CAIRO`; covers make_canvas → real-render, clear/fill_rect pixel readback against an independent image-surface mirror, save/restore state-stack balance, all path-op kinds without crashing, ellipse + clip composition. Linux ctest with Cairo backend: 23 graphics cases / 92 assertions (vs 16 / 68 with stub). | Linux 347/347 with Cairo + Win 340/340 with stub fallback + Android APK builds clean with stub fallback |

**Net:** ADR-0015's primary backend is now real on Linux. Apps that build the Linux target with the default CMake config get hardware-quality 2D rendering through libcairo, while Windows + Android stay on the stub backend until each platform's Cairo bundling story lands. The same source compiles on every platform — the only platform-specific bit is the CMake-detection logic that gates compilation of `cairo_backend.cpp`. Skia backend + the ShapeView/GraphicsView migration are the remaining v2 work; both decoupled from the backend-selection design that just shipped.

## ADR-0015 v2 — Cairo on Windows + Android via vcpkg

| Commit | Scope | Build state |
|---|---|---|
| `b6f7cac` | **Cairo on Windows + Android** — vcpkg cairo port supplies the lib + `.pc` metadata on every triplet. Windows uses `cairo:x64-windows` + vcpkg's bundled mingw64 pkgconf (the msys2/usr/bin pkgconf splits paths at `C:` drive-letter colons; mingw64's handles Windows paths correctly). Android uses `cairo:x64-android` or `cairo:arm64-android` — Gradle's externalNativeBuild block in `examples/android_hello/app/build.gradle.kts` resolves the vcpkg prefix from `VCPKG_ROOT` (defaulting to `$USERPROFILE/vcpkg`) and passes it as `MPAPP_CAIRO_PREFIX` into the per-app CMakeLists, which bridges it into `ENV{PKG_CONFIG_PATH}` for the same `pkg_check_modules` detection that works on Linux. Android `minSdk` bumped 24 → 28 because fontconfig (Cairo's default-feature dep) needs bionic libiconv, which Android exposes starting at API 28. CMakeLists fallback message documents the per-platform setup. | Win ctest 347/347 with real Cairo + Linux ctest 347 with real Cairo + Android APK builds w/ libcairo + pixman + fontconfig + freetype statically linked (libandroid_hello.so 9.1 MB → 24.7 MB; stripped APK 5.8 MB) |

**Net:** ADR-0015 is now real-Cairo on every supported platform. The same `cairo_backend.cpp` code compiles unchanged via the platform-uniform pkg-config detection path. CMake auto-falls-back to stub when vcpkg isn't installed so contributor onboarding stays low-friction. Remaining v2 work: Skia backend (opt-in, ~30 MB, BSD-3) + the ShapeView/GraphicsView migration to route through the canvas facade instead of per-platform native primitives.

## CollectionView typed_items

| Commit | Scope | Build state |
|---|---|---|
| `9b245d7` | **CollectionView typed_items** parallel surface on 3 platforms. Mirrors TableView's typed_sections pattern: `vector<view*>` of cell/view pointers; handler renders each via ADR-0013 dispatch. Win/Linux: append to the existing ListView/GridView/GtkListBox/GtkFlowBox inner widget. Android: swap inner to ScrollView+LinearLayout(VERTICAL) for typed mode since the AdapterView+ArrayAdapter recycler doesn't fit non-virtualizing typed children. v1 limitations: non-virtualizing in typed mode, selection doesn't apply, Android layout enum ignored in typed mode. | Win 287/287 + Linux + Android green |
| `a8669a8` | Mock-level tests for typed_items — defaults, non-owning view* round-trip, coexist with items_source. | Win 290/290 |

**Why this matters:** the cell tree + ADR-0013 dispatch composition is now usable inside ListView/CollectionView too, not just TableView. Apps with small lists of rich items can use CollectionView with `typed_items = {…cell pointers…}` instead of having to fall back to TableView for typed rows.

## ADR-0018 / typed-bridge / Current Focus orientation

| Commit | Scope |
|---|---|
| `1f72752` | Refresh Current Focus + Home.md after the ADR-0018 v1 wave |
| `eb738a1` | Refresh Current Focus after the v2 wave (callbacks + coroutines + JS shim) |

## See also

- [[40_Roadmap/M-04c-handler-heavy-port]] — canonical tracker.
- [[Controls Inventory]] — per-widget status.
- [[T-0014-async-navigation-push-pop]] · [[T-0015-shell-real-handlers]] — task logs.
- [[2026-W21-autonomous-bulk-port]] — earlier session that got M-04b to done.
