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
| WebView / HybridWebView | **android-real** — native messaging end-to-end (WebMessageReceived / script-message-handler / addJavascriptInterface). Typed JSON-RPC bridge layered on top per ADR-0018, **v2 complete inbound + outbound**: C++ side has `set_bridge<T>()` inbound dispatch (sync via `register_method` + **async via `register_async_method<T>`** with deferred `respond(value)` callback) + `invoke_js`/`invoke_js_cb`/`invoke_js_async` outbound (callback + coroutine APIs); JS side has `window.mpapp.register(name, fn)` + `window.mpapp.call(name, args...)`. Symmetric typed round-trips end-to-end. Async bridge methods route through `dispatch_async(payload, on_response)` — sync methods still fire inline (preserving v1 behavior); async methods defer the response until their `respond()` callback resolves. |
| ShapeView / GraphicsView | **android-real (v1)** — per-platform native primitives. Full unified canvas facade is v2 per [[ADR-0015-graphics-backend-dual]]. |
| macOS / iOS | code-complete on app-shell layer; the M-04b/M-04c sweep stayed on Win/Linux/Android pending an Apple host |

## What's still open (in priority order)

1. **ADR-0015 follow-ups: Skia backend + ShapeView/GraphicsView migration.** Cairo backend is now real on Linux, Windows (via vcpkg), and Android (via vcpkg + NDK; minSdk bumped to 28 for bionic libiconv). **GraphicsView migration is in flight** — [[T-0029]] landed both design moves (pixel readback on the abstract canvas API + a `drawable` callback Observable on `graphics_view`) plus the Linux real handler that pumps the callback through the facade and blits BGRA32 pixels into the GtkDrawingArea. Windows + Android `map_drawable` stubs are in place for interface uniformity; their real blit paths are T-0029 phase 2. **Skia backend scaffolding + impl** landed in [[T-0030]] — `find_package(unofficial-skia)` detection in CMake, `skia_canvas` wraps `SkBitmap` + `SkCanvas` with all facade ops routed to Skia primitives. Opt-in via `-DMPAPP_GRAPHICS_BACKEND=skia` once user runs `vcpkg install skia:<triplet>` (~30 min from source, cached after). Falls back to stub cleanly when Skia isn't installed. ShapeView migration is still outstanding (replaces existing real handlers' per-platform shape primitives with facade calls).
2. **macOS + iOS sweep** across the entire widget set. Requires an Apple host. Existing Objective-C++ handlers on app-shell are the template; the rest need to follow.
3. **Cross-cutting tests for real handlers.** Mock-handler tests cover the surface contract; real-handler behavior is verified only through end-to-end builds + spot-checks. Worth a `tests/integration/` pass once a CI matrix is set up.
4. **[[_Archive/T-0028-collectionview-orientation|T-0028]] closure.** Code-complete on all three platforms (Win 343/343, Linux 348/348, Android APK clean). Pending Rule 11 closure: per-platform screenshots (4 layouts × 3 platforms) + optional layout-toggle demo apps. `status: in-progress` until then.

## Active milestone

[[40_Roadmap/M-04c-handler-heavy-port|M-04c]] (`active`) — exits to `shipped` once the proposed ADRs are accepted and the user signs off.

## Recently accepted ADRs

The M-04c proposal wave (ADR-0014–0022) is now mostly **accepted** — 8 of 9 flipped per Rule 4 after the implementations shipped + tested across all 3 platforms. ADR-0015 stays `proposed` even though the Cairo backend just landed because the Skia backend + handler-migration work remain. ADR-0023 is now **accepted** (2026-05-23) — covering shell-route-guards-and-lifecycle, Rule-11-closed by [[_Archive/T-0017-typed-routing-demo|T-0017]].

- [[ADR-0022-android-kind-discriminated-routers]] — Android listener kind dispatch family (just accepted)
- [[ADR-0021-tableview-cell-types]] — full MAUI cell parity (just accepted)
- [[ADR-0020-virtualized-item-host-wrap-platform]] — wrap-the-recycler pattern (just accepted)
- [[ADR-0019-async-executor-native-dispatcher]] — `task<T>` + UI dispatcher (just accepted)
- [[ADR-0018-hybrid-webview-typed-bridge]] — full v2 JS bridge stack (just accepted)
- [[ADR-0017-grid-track-definitions]] — Grid track-def value type + parser (just accepted)
- [[ADR-0016-shell-compile-time-routes]] — NTTP route_table (just accepted)
- [[ADR-0014-page-navigation-stack]] — page_stack engine (just accepted)
- [[ADR-0013-data-driven-widget-dispatch]] (M-04b foundation — per-platform dispatch registry)
- [[ADR-0012-application-window-handler-abstraction]] — app-shell handler template; proved by T-0011

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
- ✅ **HybridWebView typed JS bridge v1** per [[ADR-0018-hybrid-webview-typed-bridge]] — `set_bridge<MyBridge>()` for inbound JSON-RPC dispatch + `invoke_js("method", args...)` for outbound. Built on a header-only JSON layer (`include/mpapp/detail/json.hpp`, ~520 LOC) + a cross-platform `process_inbound` choke point so the bridge-or-raw decision lives in one file.
- ✅ **HybridWebView typed JS bridge v2** — outbound callback (`invoke_js_cb<T>`) + coroutine (`invoke_js_async<T>`) APIs, plus inbound async dispatch via `register_async_method<T>` and `dispatch_async(payload, on_response)`. Async bridge methods accept a trailing `std::function<void(T)> respond` callback they invoke when ready (synchronously or deferred). Routes through partial-specialization `async_invoker_builder<T, Method>` to sidestep the "Args... in non-trailing position is non-deducible" C++ rule. Sync methods still fire inline through dispatch_async for v1 compatibility.
- ✅ **ADR-0016 compile-time Shell route table** — `mpapp::route_table{ route<"home", home_page>{}, route<"home/details", details_page, param<"id", int>>{}, ... }` declared as an `inline constexpr` value; `shell.go_to<"home/details", &routes>(42)` is compile-time-checked against the table for both route name and argument types. Implementation rests on a `fixed_string<N>` NTTP wrapper (C++20 class-type NTTP), an index-sequence-based route finder that emits a clear `static_assert` on lookup failure, and an ADL-customizable `to_route_string` for URI argument stringification. The string-based `go_to(std::string_view)` parser was extended to also cut at `?` so the typed `go_to<>` path's `//path?p1=v1&p2=v2` URIs still drive `current_tab_index` by tab label.
- ✅ **CollectionView item_template** — `Observable<function<unique_ptr<view>(int)>> item_template`. When set, the collection_view auto-materializes one cell per items_source row, owns the lifetime, and emits `materialized_changed` so handlers know to rebuild. Composes with the existing typed render pipeline on all 3 platforms — `rebuild_active()` precedence is now typed_items → materialized_views → flat items_source, and each handler's map_typed_items subscribes to `materialized_changed` to pick up template re-fires.
- ✅ **Shell route guard — can_activate** — `Observable<function<bool(string_view target)>> can_activate` consulted before each `go_to(uri)` and the typed `go_to<Path, &Table>()`. False aborts navigation and emits `navigation_blocked`. Closes ADR-0016's "route guards deferred to follow-up" item for the simpler activate case; can_deactivate (current-route-aware) + route-lifecycle hooks (`OnNavigatedTo`/`OnNavigatedFrom`) remain future work tied to the ADR-0019 executor.
- ✅ **Shell can_deactivate + page navigated_to/from lifecycle** — second route guard (`Observable<function<bool(current,target)>> can_deactivate`) + per-page lifecycle signals (`navigated_to(uri)` / `navigated_from(previous_uri)`) fired by shell on each successful go_to. Guard chain is deactivate-then-activate; either's false aborts and emits `navigation_blocked`. New ADR-0023 (proposed) captures the full design.
- ✅ **ADR-0015 v2 canvas facade (stub backend)** — `mpapp::detail::graphics::canvas` abstract interface + `color`/`path`/`rect` value types + SVG path subset parser + `make_canvas(w, h)` factory. `MPAPP_GRAPHICS_BACKEND` CMake option selects backend at compile time. Cairo backend now real on Linux (default via libcairo + pkg-config); Windows + Android fall back to stub until each bundles its own Cairo (vcpkg + NDK prebuilt). Skia backend + the ShapeView/GraphicsView migration are the remaining v2 work.
- ✅ **ADR-0015 v2 Cairo backend** — `src/detail/graphics/cairo_backend.cpp` implements the full `canvas` interface against `cairo_image_surface_t`. Quad Bezier ops upconvert to Cairo's cubic form; ellipse rendering uses translate-scale-arc; save/restore maps to Cairo's state stack. LGPL-2.1 via dynamic-link against system libcairo (RFC-0001 §Linux pattern). 7 ctest cases gated on `MPAPP_GRAPHICS_HAS_CAIRO`; Linux ctest grew to 347 total when Cairo is selected.
- ✅ **ADR-0015 v2 Cairo on Windows + Android** — vcpkg's `cairo:x64-windows` provides the lib + .pc metadata; vcpkg's bundled mingw64 pkgconf handles Windows drive-letter paths. Android wiring (`examples/android_hello/app/build.gradle.kts`) resolves the vcpkg arm64/x64-android prefix and bridges it into the externalNativeBuild CMake invocation via `MPAPP_CAIRO_PREFIX` → `ENV{PKG_CONFIG_PATH}`. minSdk bumped 24 → 28 because fontconfig (Cairo's default-feature dep) needs Android bionic libiconv (API 28+). Win ctest 347/347 green with real Cairo; Android APK builds with libcairo + pixman + fontconfig + freetype statically linked (`libandroid_hello.so` 9.1 MB → 24.7 MB).

What's left at code level:

- **ADR-0016 compile-time Shell route table** — heavy template metaprogramming around NTTP string literals. The string-based runtime `go_to(uri)` parser already works; this adds the compile-time-checked `shell.go_to<"home/details">(42)` syntax.
- **ADR-0015 v2 unified canvas facade** (Cairo + Skia compile-time selectable) — ShapeView + GraphicsView v1 already ship per-platform native primitives, so apps that need basic shapes don't block on this.
- **CollectionView item_template** tied to the cell-type tree — would let users supply a cell factory and have CollectionView use it instead of plain string items. Composes with the new vertical_grid path.
- **JS shim ergonomics** — current shim broadcasts inbound to listeners; user JS code does its own envelope dispatch + response posting. A `window.mpapp.register(name, fn)` + auto-response shim would mirror the C++-side `register_method` ergonomics. Not blocking — pure ergonomics.

What's left at process / host level:

- **ADR acceptance pass** — done as of 2026-05-23. 9 of 10 M-04c-era ADRs are now `accepted` (0014, 0016–0022, 0023); only ADR-0015 stays `proposed` because the Skia backend + ShapeView/GraphicsView migration are still future work.
- **macOS + iOS handler sweep** across the widget set. Existing Objective-C++ handlers on app-shell are the template.
