---
type: milestone
id: M-04c
title: Handler heavy port — ADR-gated widgets
phase: p3
status: shipped
deliverables:
  - Every M-04c-listed widget reaches `android-real` status (Win + Linux + Android verified)
  - One design ADR per heavy widget or widget family, accepted before code lands
  - Page-stack engine (per ADR-0014) reused by all 4 page-level widgets
exitCriteria:
  - 0 components in `Controls Inventory.md` at `not-started` status
  - Shell + NavigationPage + TabbedPage + FlyoutPage + ListView + TableView + CollectionView + WebView + HybridWebView + ShapeView + GraphicsView + Grid all `android-real`
  - Each gated ADR is `accepted`
tags:
  - type/milestone
  - status/shipped
  - phase/p3
  - phase/p4
  - phase/p5
---

# M-04c — Handler heavy port (ADR-gated)

The final widget-porting milestone. Carves out the 12-ish widgets that need a **design ADR** before code can land — typically because they introduce a new architectural pattern (stack semantics, virtualization, drawing surface) or a licensing posture (WebView).

## Why this is its own milestone

[[40_Roadmap/M-04b-handler-bulk-port|M-04b]] absorbed every widget with a clear native primitive mapping. The widgets that remain don't have that clean mapping — each is a small design problem in its own right. Lumping them into M-04b would have stalled the bulk port behind a series of ADR discussions.

## Gated widgets and their gating ADRs

| Widget | Gating ADR | Status | Reason |
|---|---|---|---|
| `NavigationPage` | [[ADR-0014-page-navigation-stack]] | ADR proposed; **handlers landed** | Page stack semantics |
| `TabbedPage` | [[ADR-0014-page-navigation-stack]] | shares ADR; **handlers landed** | Tab-host + stack composition |
| `FlyoutPage` | [[ADR-0014-page-navigation-stack]] | shares ADR; **handlers landed** | Drawer + detail stack |
| `Shell` | [[ADR-0014-page-navigation-stack]] + [[ADR-0016-shell-compile-time-routes]] | ADRs proposed | Combines all the above + URI routing |
| `ListView` | [[ADR-0020-virtualized-item-host-wrap-platform]] | ADR proposed | Item recycling + items_source binding |
| `TableView` | [[ADR-0020-virtualized-item-host-wrap-platform]] + [[ADR-0021-tableview-cell-types]] | ADRs proposed | Static section/row layout + cell-type tree |
| `CollectionView` | [[ADR-0020-virtualized-item-host-wrap-platform]] | ADR proposed | Modern MAUI list (replaces ListView) |
| `WebView` | [[RFC-0001-licensing-and-patent-strategy]] § Linux WebKitGTK (resolved: WebKitGTK dynamic link) | decision recorded | LGPL on Linux requires dynamic linking + rebuild path |
| `HybridWebView` | [[ADR-0018-hybrid-webview-typed-bridge]] | ADR proposed | C++ ↔ JS interop bridge |
| `ShapeView` | [[ADR-0015-graphics-backend-dual]] | ADR proposed | Cairo (default) / Skia (opt-in) dual backend |
| `GraphicsView` | shares ADR-0015 | ADR proposed | Canvas surface |
| `Grid` (real layout engine) | [[ADR-0017-grid-track-definitions]] | ADR proposed | Real `*` / `Auto` track definitions + child placement |

**Cross-cutting:** [[ADR-0019-async-executor-native-dispatcher]] gates `push_async` / `pop_async` (NavigationPage), the JS bridge's task<T>, and any future async surface across the framework.

## Process

For each gated widget:

1. Write the design ADR. Status `proposed`. Must answer:
   - What's the native primitive on each platform?
   - What's the public C++ surface?
   - How does it compose with already-landed widgets via the [[ADR-0013-data-driven-widget-dispatch|dispatch registry]]?
   - What are the known platform divergences?
2. Mark ADR `accepted` after a review pass.
3. Implement using the [[_Templates/Worker-Prompt|worker-prompt template]] adapted for the heavier surface — likely several commits per widget rather than one.
4. Land the inventory + frontmatter updates.
5. Move on to the next gating ADR.

## Tracker

| Widget | ADR | Implementation status |
|---|---|---|
| NavigationPage | ADR-0014 proposed | **android-real (2026-05-21)** — page_stack engine + 3-platform handlers + push_async/pop_async (2026-05-22) |
| TabbedPage | ADR-0014 proposed | **android-real (2026-05-21)** — 3-platform handlers |
| FlyoutPage | ADR-0014 proposed | **android-real (2026-05-21)** — 3-platform handlers |
| Shell | ADR-0014 + [[ADR-0016-shell-compile-time-routes]] | **android-real (2026-05-22)** — Win SplitView + Linux GtkPaned + Android LinearLayout w/ tab strip + content swap; compile-time route table per ADR-0016 deferred |
| ListView | [[ADR-0020-virtualized-item-host-wrap-platform]] | **android-real (2026-05-22)** — Win mux::ListView + Linux GtkListBox + Android android.widget.ListView w/ ArrayAdapter |
| TableView | [[ADR-0020-virtualized-item-host-wrap-platform]] + [[ADR-0021-tableview-cell-types]] | **android-real (2026-05-22)** — section-flatten rendering on 3 platforms; cell-typed rendering pending table_view surface refactor |
| CollectionView | [[ADR-0020-virtualized-item-host-wrap-platform]] | **android-real (2026-05-22)** — Win mux::ListView + Linux GtkListBox + Android ListView; selection_mode mapping wired; multi-select events + grid layouts deferred |
| TableView cells | [[ADR-0021-tableview-cell-types]] | **android-real (2026-05-22)** — text_cell + view_cell + switch_cell + image_cell + entry_cell all 3 platforms; new shared `MppEditorActionListener` carries IME terminal actions to entry_cell.completed |
| WebView | RFC-0001 § Linux licensing | **android-real (2026-05-22)** — Win muxc::WebView2 + Linux WebKitGTK 6.x (LGPL dynamic) + Android android.webkit.WebView; navigating/navigated/is_loading/can_go_back/forward wired via NavigationStarting/Completed + "load-changed" + MppWebViewClient |
| HybridWebView | depends WebView + [[ADR-0018-hybrid-webview-typed-bridge]] | **android-real (2026-05-22)** — `window.mpapp` JS shim injected on each page; Win WebView2 WebMessageReceived/PostWebMessageAsString + Linux WebKitUserContentManager script-message-handler + Android addJavascriptInterface MppJsBridge |
| ShapeView | [[ADR-0015-graphics-backend-dual]] | **android-real (2026-05-22, v1)** — Win muxc::Border + muxc::Shapes::Rectangle/Ellipse/Line; Linux GtkDrawingArea + cairo draw callback; Android MppShapeView w/ onDraw. polygon+path render as bounding rect; full SVG path parsing + unified canvas facade per ADR-0015 is v2 scope. |
| GraphicsView | [[ADR-0015-graphics-backend-dual]] | **android-real (2026-05-22, v1)** — Win muxc::Canvas + Linux GtkDrawingArea + Android android.view.View; width/height propagated to native widget. User-facing draw API still gated on ADR-0015 v2 canvas facade. |
| Grid (real) | [[ADR-0017-grid-track-definitions]] | **android-real (2026-05-22)** — Win mux::Grid w/ GridLength + Linux GtkGrid w/ hexpand bridging + Android GridLayout w/ Spec-based LayoutParams; per-child placement via grid.set_row/set_column attached store. Star sizing is approximate on Linux/Android — exact only on Win. |

**Status as of 2026-05-22 close (final):** **Every concrete widget in M-04c is at android-real on Win/Linux/Android.** ShapeView + GraphicsView shipped v1 with per-platform native primitives (deferring the unified canvas facade per ADR-0015 to v2). Remaining work is process-level rather than code-level:

- Each proposed ADR (0014–0021) needs a formal acceptance pass before M-04c flips from `active` to `shipped`. The decisions are already implemented; this is the review-gate part of Rule 4.
- Compile-time route table per ADR-0016 for Shell — design is documented, implementation is template-metaprogramming heavy and ships as a Shell follow-up.
- ShapeView/GraphicsView v2 (Cairo+Skia compile-time selectable graphics facade) per ADR-0015.
- macOS + iOS handlers across the entire widget set — gated on Apple host.

## See in code

The 12 heavy widgets this milestone shipped, all `android-real`:

- **Page family** (per [[ADR-0014-page-navigation-stack]]) — engine [`include/mpapp/page.hpp`](../../include/mpapp/page.hpp) + [`detail/page_stack.hpp`](../../include/mpapp/detail/page_stack.hpp); widgets [`navigation_page.hpp`](../../include/mpapp/navigation_page.hpp) · [`tabbed_page.hpp`](../../include/mpapp/tabbed_page.hpp) · [`flyout_page.hpp`](../../include/mpapp/flyout_page.hpp) · [`shell.hpp`](../../include/mpapp/shell.hpp).
- **List family** (per [[ADR-0020-virtualized-item-host-wrap-platform]]) — [`list_view.hpp`](../../include/mpapp/list_view.hpp) · [`collection_view.hpp`](../../include/mpapp/collection_view.hpp) · [`table_view.hpp`](../../include/mpapp/table_view.hpp).
- **Cell tree** (per [[ADR-0021-tableview-cell-types]]) — [`cell.hpp`](../../include/mpapp/cell.hpp) + [`text_cell.hpp`](../../include/mpapp/text_cell.hpp) · [`entry_cell.hpp`](../../include/mpapp/entry_cell.hpp) · [`switch_cell.hpp`](../../include/mpapp/switch_cell.hpp) · [`view_cell.hpp`](../../include/mpapp/view_cell.hpp) · [`image_cell.hpp`](../../include/mpapp/image_cell.hpp).
- **Web** — [`web_view.hpp`](../../include/mpapp/web_view.hpp) · [`hybrid_web_view.hpp`](../../include/mpapp/hybrid_web_view.hpp) + bridge [`hybrid_bridge.hpp`](../../include/mpapp/hybrid_bridge.hpp) + JSON layer [`detail/json.hpp`](../../include/mpapp/detail/json.hpp) (per [[ADR-0018-hybrid-webview-typed-bridge]]).
- **Graphics** (per [[ADR-0015-graphics-backend-dual]]) — facade [`include/mpapp/detail/graphics/canvas.hpp`](../../include/mpapp/detail/graphics/canvas.hpp) + backends [`src/detail/graphics/cairo_backend.cpp`](../../src/detail/graphics/cairo_backend.cpp) + [`skia_backend.cpp`](../../src/detail/graphics/skia_backend.cpp); widgets [`shape_view.hpp`](../../include/mpapp/shape_view.hpp) · [`graphics_view.hpp`](../../include/mpapp/graphics_view.hpp).
- **Grid as a real layout engine** (per [[ADR-0017-grid-track-definitions]]) — [`grid_layout.hpp`](../../include/mpapp/grid_layout.hpp).

Per-platform handlers for each widget live at `src/handlers/{windows,linux,android}/<snake>_handler.cpp`. Closure tasks (archived per Rule 11) under [`vault/50_Tasks/_Archive/T-002{8,9}-*/`](../50_Tasks/_Archive/) + [`T-003{0,1}-*/`](../50_Tasks/_Archive/).

## See also

- [[ADR-0013-data-driven-widget-dispatch]] — the dispatch foundation M-04b shipped on; everything in M-04c composes with it.
- [[ADR-0014-page-navigation-stack]] — first M-04c ADR; gates the 4 page-level widgets.
- [[M-04b-handler-bulk-port]] — the milestone that finished first; M-04c picks up the deferred remainder.
- [[Controls Inventory]] — single source of truth for current porting status.
