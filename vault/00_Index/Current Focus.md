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
| List family (ListView / CollectionView / TableView) | **android-real** + multi-select event round-trip on all 3 platforms |
| Cell tree (text/view/switch/image/entry) | **android-real** — every cell with bidirectional bindings where the surface has them |
| Grid (real layout engine) | **android-real** — native Grid wrap + per-child placement via attached property store |
| WebView / HybridWebView | **android-real** — native messaging end-to-end (WebMessageReceived / script-message-handler / addJavascriptInterface) |
| ShapeView / GraphicsView | **android-real (v1)** — per-platform native primitives. Full unified canvas facade is v2 per [[ADR-0015-graphics-backend-dual]]. |
| macOS / iOS | code-complete on app-shell layer; the M-04b/M-04c sweep stayed on Win/Linux/Android pending an Apple host |

## What's still open (in priority order)

1. **ADR acceptance pass** for proposed ADRs 0014–0021. Code is implemented and shipping; review-gate is the missing step per Rule 4. Once deciders fill in, each ADR's frontmatter flips `proposed → accepted`.
2. **macOS + iOS sweep** across the entire widget set. Requires an Apple host. Existing Objective-C++ handlers on app-shell are the template; the rest need to follow.
3. **ADR-0016 compile-time route table for Shell** — heavy template-metaprogramming follow-up. Design is documented; user-facing `shell.go_to<"home/details">(42)` syntax with compile-time route + type validation. Currently the string-based `go_to(uri)` parser drives `current_route`.
4. **ADR-0015 v2 unified canvas facade** — Cairo (default) / Skia (opt-in) compile-time selectable backend. v1 already ships per-platform native primitives so apps that need basic shapes don't block on this.
5. **Cross-cutting tests for real handlers.** Mock-handler tests cover the surface contract; real-handler behavior is verified only through end-to-end builds + spot-checks. Worth a `tests/integration/` pass once a CI matrix is set up.

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

Look for the highest tractable item that doesn't need a human-only step:

- The CollectionView `layout` enum (vertical_list / horizontal_list / vertical_grid / horizontal_grid) still only renders vertical_list. Wiring the remaining three across mux::GridView / GtkFlowBox / android.widget.GridView is well-scoped.
- TableView's surface refactor (`vec<table_section{title, vec<unique_ptr<cell>>}>`) so its rows render through the cell-type tree instead of being plain strings.
- TabbedPage + Shell bar styling polish (selected-tab color emphasis).
- HybridWebView typed JS bridge per [[ADR-0018-hybrid-webview-typed-bridge]] — current v1 ships an untyped string bridge; the ADR proposes JSON-RPC + `[[mpapp::js_method]]` attribute-driven typing.

For things gated on a human:
- ADR acceptance pass (0014–0021).
- macOS + iOS host availability.
