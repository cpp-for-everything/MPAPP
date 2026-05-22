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

## See also

- [[40_Roadmap/M-04c-handler-heavy-port]] — canonical tracker.
- [[Controls Inventory]] — per-widget status.
- [[T-0014-async-navigation-push-pop]] · [[T-0015-shell-real-handlers]] — task logs.
- [[2026-W21-autonomous-bulk-port]] — earlier session that got M-04b to done.
