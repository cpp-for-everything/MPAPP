---
type: milestone
id: M-04c
title: Handler heavy port — ADR-gated widgets
phase: p3
status: planned
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
| NavigationPage | ADR-0014 proposed | **mock landed** — page_stack engine + lifecycle signals + attached-prop store (2026-05-21) |
| TabbedPage | ADR-0014 proposed | **mock landed** — children + selected_index + tab-switch lifecycle (2026-05-21) |
| FlyoutPage | ADR-0014 proposed | **mock landed** — flyout/detail/is_presented + present/dismiss/toggle + presented_changed (2026-05-21) |
| Shell | ADR-0014 + URI routing ADR TBD | **mock landed** — register_route + go_to URI parser + tabs/flyout/navigated/flyout_toggled (2026-05-21); full route templates + guards + Aware interfaces deferred |
| ListView | virtualized-item-host ADR TBD | **mock landed** — items_source + selected_index + item_tapped (2026-05-21) |
| TableView | virtualized-item-host ADR TBD | **mock landed** — sections vec + intent + row_height + add_section/add_row helpers (2026-05-21) |
| CollectionView | virtualized-item-host ADR TBD | **mock landed** — items_source + selection_mode none/single/multiple + select/deselect/clear (2026-05-21) |
| WebView | RFC-0001 § Linux licensing | **mock landed** — url + html + history + load/back/forward/reload + navigating/navigated (2026-05-21) |
| HybridWebView | depends WebView | **mock landed** — extends WebView with hybrid_namespace + send_to_js/simulate_inbound + message_received/message_sent (2026-05-21) |
| ShapeView | 2D graphics backend ADR TBD | **mock landed** — shape_kind enum + data path string + fill/stroke/stroke_thickness/opacity (2026-05-21) |
| GraphicsView | 2D graphics backend ADR TBD | **mock landed** — width/height + draw_count + invalidate() + draw_requested signal (2026-05-21) |
| Grid (real) | track-based layout ADR TBD | mock exists |

**Status as of 2026-05-21 close:** Every widget in [[Controls Inventory]] is at **mock or better**. Zero `not-started` rows remain. The next pieces of work for M-04c are the gating ADRs (URI routing, virtualized item host, 2D graphics backend, track-based layout) plus per-platform real handlers for the 11 mocks above.

## See also

- [[ADR-0013-data-driven-widget-dispatch]] — the dispatch foundation M-04b shipped on; everything in M-04c composes with it.
- [[ADR-0014-page-navigation-stack]] — first M-04c ADR; gates the 4 page-level widgets.
- [[M-04b-handler-bulk-port]] — the milestone that finished first; M-04c picks up the deferred remainder.
- [[Controls Inventory]] — single source of truth for current porting status.
