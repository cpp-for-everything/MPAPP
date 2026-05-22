---
type: moc
tags:
  - type/moc
---

# Current Focus

> [!important] Status — 2026-W21 (close)
> **M-04b done. M-04c functionally complete — `active` pending ADR-acceptance pass.**
>
> **61 of 64 components are at `android-real` on Win + Linux + Android.** The remaining `mock` rows are 4 abstract bases (View / Layout / Element / Cell) that have no native primitives by design; their concrete subclasses own the real handlers.
>
> The autonomous push that landed [[40_Roadmap/M-04c-handler-heavy-port|M-04c]] covered: the page-level family (NavigationPage / TabbedPage / FlyoutPage / Shell with async push/pop + page_stack engine), the list family (ListView / CollectionView / TableView with wrap-platform-recycler per [[ADR-0020-virtualized-item-host-wrap-platform]]), the full cell tree (text/view/switch/image/entry per [[ADR-0021-tableview-cell-types]]), Grid as a real layout engine, WebView + HybridWebView (WebView2 / WebKitGTK 6.x / android.webkit.WebView + JS-bridge shim on each), and ShapeView + GraphicsView v1 (per-platform native primitives).

## Where we are now

| Layer | Status |
|---|---|
| Page family (NavigationPage / TabbedPage / FlyoutPage / Shell) | **android-real** + `task<T>` async push/pop wrappers |
| List family (ListView / CollectionView / TableView) | **android-real** + multi-select event round-trip on all 3 platforms; CollectionView vertical_grid layout swap on all 3 platforms |
| Cell tree (text/view/switch/image/entry) | **android-real** — every cell with bidirectional bindings where the surface has them; row→cell.tapped routing through `table_view::cell_at` |
| Grid (real layout engine) | **android-real** — native Grid wrap + per-child placement via attached property store |
| WebView / HybridWebView | **android-real** — native messaging end-to-end (WebMessageReceived / script-message-handler / addJavascriptInterface); typed JSON-RPC bridge layered on top per ADR-0018: `set_bridge<T>()` inbound dispatch + `invoke_js("method", args...)` outbound, both shipping, sync only (task<T> async is v2) |
| ShapeView / GraphicsView | **android-real (v1)** — per-platform native primitives. Full unified canvas facade is v2 per [[ADR-0015-graphics-backend-dual]]. |
| macOS / iOS | code-complete on app-shell layer; the M-04b/M-04c sweep stayed on Win/Linux/Android pending an Apple host |

## What's still open (in priority order)

1. **ADR acceptance pass** for proposed ADRs 0014–0022 (9 ADRs). Code is implemented and shipping; review-gate is the missing step per Rule 4. Once deciders fill in, each ADR's frontmatter flips `proposed → accepted`.
2. **macOS + iOS sweep** across the entire widget set. Requires an Apple host. Existing Objective-C++ handlers on app-shell are the template; the rest need to follow.
3. **ADR-0018 v2 async return values** — typed bridge ships sync-only in v1; v2 wraps bridge methods + `invoke_js` returns in `task<T>` so `co_await wv->invoke_js<"add">(1,2)` resolves once JS responds. Reuses the ADR-0019 executor; needs a C++-side id→continuation map for response routing.
4. **ADR-0016 compile-time route table for Shell** — heavy template-metaprogramming follow-up. Design is documented; user-facing `shell.go_to<"home/details">(42)` syntax with compile-time route + type validation. Currently the string-based `go_to(uri)` parser drives `current_route`.
5. **ADR-0015 v2 unified canvas facade** — Cairo (default) / Skia (opt-in) compile-time selectable backend. v1 already ships per-platform native primitives so apps that need basic shapes don't block on this.
6. **Cross-cutting tests for real handlers.** Mock-handler tests cover the surface contract; real-handler behavior is verified only through end-to-end builds + spot-checks. Worth a `tests/integration/` pass once a CI matrix is set up.

## Active milestone

[[40_Roadmap/M-04c-handler-heavy-port|M-04c]] (`active`) — exits to `shipped` once the proposed ADRs are accepted and the user signs off.

## Recently accepted ADRs

(Newer ADRs 0014–0021 are still `proposed`; the code that they describe is implemented.)

- [[ADR-0013-data-driven-widget-dispatch]] (M-04b foundation — per-platform dispatch registry)
- [[ADR-0012-application-window-handler-abstraction]] — app-shell handler template; proved by T-0011
- [[ADR-0011-cross-compilation-toolchain]] — Zig (`zig cc`)
- [[ADR-0010-licensing-and-patent-strategy]] — Apache 2.0 + commercial dual
- [[ADR-0009-public-api-template-wrappers-only]] — template wrappers only

(See [[Decision Log]] for the full chain.)

## Pinned reading for new contributors

- [[CLAUDE]] — vault rules.
- [[Controls Inventory]] — single source of truth for widget porting status.
- [[40_Roadmap/M-04c-handler-heavy-port]] — the most recent milestone tracker.
- [[Type System]] — template-wrapper-type design.
- [[Build System]] — cross-compilation matrix.
- [[60_Research/dotnet-maui-deep-dive]] — the spec MPAPP mirrors.
- [[90_Logs/2026-W21-autonomous-m04c-push|2026-W21 session log]] — narrative of how M-04c got to functionally complete.

## Where to pick up next

The Current-Focus "pickup list" caught up to itself across the W21 close push. The polish items it called out are now landed:

- ✅ CollectionView `layout` enum — vertical_grid wired on all 3 platforms via the outer-container + inner-widget-swap pattern (mux::GridView / GtkFlowBox / android.widget.GridView). horizontal_list / horizontal_grid still degrade to vertical for v1 (would require ItemsPanelTemplate work on Win + GtkOrientable on Linux + RecyclerView on Android).
- ✅ TableView typed_sections — `vector<table_section_typed{title, vec<cell*>}>` parallel surface renders cell-tree rows through ADR-0013 dispatch. row_tapped wired on Win + Linux (was Android-only); cell.tapped routes through `table_view::cell_at` on all 3 platforms.
- ✅ TabbedPage + Shell selected-tab styling — Android handlers now restyle the active tab with primary-blue + bold; Win/Linux already had this via the native widget.
- ✅ **HybridWebView typed JS bridge v1** per [[ADR-0018-hybrid-webview-typed-bridge]] — `set_bridge<MyBridge>()` for inbound JSON-RPC dispatch + `invoke_js("method", args...)` for outbound. Built on a header-only JSON layer (`include/mpapp/detail/json.hpp`, ~520 LOC) + a cross-platform `process_inbound` choke point so the bridge-or-raw decision lives in one file. Sync only — `task<T>` async is v2.

What's left at code level:

- **ADR-0018 v2 — async typed bridge.** Wrap bridge methods + `invoke_js` returns in `task<T>` so `co_await wv->invoke_js<"add">(1,2)` resolves once JS responds. The C++-side response router is a small id→continuation map; the JS shim already broadcasts responses via `message_received` so the matching logic is straightforward.
- **ADR-0016 compile-time Shell route table** — heavy template metaprogramming around NTTP string literals. The string-based runtime `go_to(uri)` parser already works; this adds the compile-time-checked `shell.go_to<"home/details">(42)` syntax.
- **ADR-0015 v2 unified canvas facade** (Cairo + Skia compile-time selectable) — ShapeView + GraphicsView v1 already ship per-platform native primitives, so apps that need basic shapes don't block on this.
- **CollectionView item_template** tied to the cell-type tree — would let users supply a cell factory and have CollectionView use it instead of plain string items. Composes with the new vertical_grid path.
- **JS shim ergonomics** — current shim broadcasts inbound to listeners; user JS code does its own envelope dispatch + response posting. A `window.mpapp.register(name, fn)` + auto-response shim would mirror the C++-side `register_method` ergonomics. Not blocking — pure ergonomics.

What's left at process / host level:

- **ADR acceptance pass** for 0014–0022 (9 proposed ADRs). Code is implemented and shipping; review-gate per Rule 4.
- **macOS + iOS handler sweep** across the widget set. Existing Objective-C++ handlers on app-shell are the template.
