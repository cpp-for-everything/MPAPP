---
type: moc
tags:
  - type/moc
---

# Current Focus

> [!important] Status — 2026-W23 (operational: MSVC toolchain + multi-platform build verification)
> **Windows MSVC toolchain is fixed and green** (VS Build Tools 2026 / cl 14.51): core + tests + the full **WinUI 3 handler library** + a windows example all compile + link; the MSVC test exe passes 5200 assertions / 1449 cases. Build-verified on **every platform with a toolchain on this host**: Windows (MSVC), Linux (WSL GTK4 — example runs under WSLg), Android (NDK 27.2 — both ABIs; emulator boots + `adb screencap` works). **macOS/iOS are the only targets with no verification path here (no Mac).** Full matrix + details in [[2026-W23-Weekly]] (session 3).
>
> [!note]- Earlier W23 sessions (Tier-2/3 parity + MVVM/animation/backends)
> A 34-agent autonomous run closed the bulk of the **Tier-2 + Tier-3** MAUI-parity gaps, all **mock-first** and verified by compiling **and running** the Catch2 suite (combined integration binary: **683 test cases / 2016 assertions green**). See [[2026-W23-Weekly]].
>
> **Tier 2:** [[Components/AbsoluteLayout|AbsoluteLayout]] + [[Components/FlexLayout|FlexLayout]] (full grid-mirrored components; Linux + Android real handlers now compile-verified — see session 2 below); gradient brushes (`view::background_brush` + `shadow` + `clip`); page dialog services (DisplayAlert / DisplayActionSheet / DisplayPrompt); modal navigation; AppThemeBinding + `Application::requested_theme`; templating (DataTemplateSelector / ControlTemplate / ContentPresenter).
>
> **Tier 3 — Essentials 4 → ~37:** sensors (×6), battery, device_display, app_info, version_tracking, main_thread, clipboard, flashlight, vibration, haptic_feedback, share, launcher, browser, email, sms, phone_dialer, file_picker, media_picker, file_system, text_to_speech, permissions, geolocation, geocoding, contacts, screenshot, web_authenticator, app_actions — each interface + in-memory mock + tests.
>
> **Then (session 2 — MVVM/animation + per-platform backends):** added the framework-richness layer (CTK MVVM — observable_object / messenger / observable_validator / async_relay_command; binding fallbacks + property_path + CTK converters; full easing set + composite/repeat/color animations + fluent extensions; event-to-command / validation behaviors) and the **per-platform backends**, now verified against real toolchains discovered this session: **WSL Ubuntu g++ 14.2 + GTK4 + WebKitGTK + GIO** (the project's real CI), **MinGW Win32**, and the **Android NDK 27.2 clang (arm64+x86_64)**. The WSL gold-standard ctest caught + fixed **3 real UB crashes** MinGW had masked. Real backends landed: cross-platform (file_system/version_tracking/app_info/main_thread), **Windows Win32** (clipboard/battery/launcher/display), **Linux GIO/GTK4** (launcher/connectivity/clipboard), **Android JNI** (clipboard/vibration); and **real Linux + Android handler implementations for AbsoluteLayout/FlexLayout** (the earlier headers were declaration-only). Full mock suite: **1476 tests / 100%** under g++ 14.2.
>
> **Open follow-ups:** Windows real layout handlers (MSVC+WinUI) + macOS/iOS (Apple host); **on-device/runtime** verification (current per-platform gate is compile/link only); faithful flexbox solver; CMake wiring of the new `src/essentials/{windows,linux,android}` backends; XAML lowering into `mpapp-xc`. macOS/iOS remain the only targets with no verification path on this host.

> [!important] Status — 2026-W21 (close)
> **M-04b done. M-04c SHIPPED — all gated ADRs accepted + all M-04c-era tasks archived per Rule 11.**
>
> **62 of 64 components are at `android-real` on Win + Linux + Android** (CarouselView promoted mock→real this push). The remaining `mock` rows are abstract bases (View / Layout / Element / Cell) that have no native primitives by design; their concrete subclasses own the real handlers.
>
> The autonomous push that landed [[40_Roadmap/M-04c-handler-heavy-port|M-04c]] covered: the page-level family (NavigationPage / TabbedPage / FlyoutPage / Shell with async push/pop + page_stack engine), the list family (ListView / CollectionView / TableView with wrap-platform-recycler per [[ADR-0020-virtualized-item-host-wrap-platform]]), the full cell tree (text/view/switch/image/entry per [[ADR-0021-tableview-cell-types]]), Grid as a real layout engine, WebView + HybridWebView (WebView2 / WebKitGTK 6.x / android.webkit.WebView + JS-bridge shim on each), and ShapeView + GraphicsView v1 (per-platform native primitives).

> [!info] Foundational subsystems — recent additions (post-M-04c)
> Four RFCs landed as mock surfaces, expanding the platform-independent foundation that the widget tree composes on:
>
> - [[RFC-0003-gesture-recognizers]] — `view::gesture_recognizers` + tap / pan / pinch / swipe / pointer recognizers. Linux real wire-up via GtkGesture controllers; Win / Android / macOS / iOS captured as T-0038 / T-0039 / T-0040 / T-0041.
> - [[RFC-0004-image-source-family]] — polymorphic `image_source_ref` over file / uri / stream / font / resource sources; `image::source_object` carries the rich source alongside the legacy string `source`. Per-platform real loaders deferred to T-0045 / T-0046 / T-0047 / T-0048 / T-0049 follow-ups.
> - [[RFC-0005-resource-dictionaries-and-styling]] — `resource_dictionary` with merged-dictionary composition, `find_in<T>` hierarchical walker over `view::resources` + the new `view::parent_` pointer, `style` with `based_on` setter chains. Closed by [[T-0044-resource-dictionary-styling-mock]]. XAML lowering of `{StaticResource}` / `<Style TargetType=…>` deferred to mpapp-xc (M-09).
> - [[RFC-0006-visual-state-manager]] — `visual_state_manager` with grouped pseudo-state setters (Normal / Pressed / PointerOver / Disabled / Focused / Selected), `go_to_state(view, name)` walking every group + applying matching state's setters. Closed by [[T-0050-rfc-0006-vsm-mock]].
> - [[RFC-0007-data-binding]] — **the keystone.** `binding<S,T>` (one_way / two_way / one_time / one_way_to_source + converters), `multi_binding`, type-erased `binding_context` inherited down the view tree, `find_ancestor` / `RelativeSource`. Closed by [[T-0051-data-binding-engine]].
> - [[RFC-0008-triggers]] — property/data `trigger<T>`, `multi_trigger`, `event_trigger`, `state_trigger` (→ VSM). Closed by [[T-0052-triggers]].
> - [[RFC-0009-behaviors-and-effects]] — attached `behavior` + `effect` collections on `view`. Closed by [[T-0053-behaviors-and-effects]].
> - [[RFC-0010-animations]] — easing + `animation` (advance-by-dt) + `animation_manager` + `fade_to`/`scale_to`/`rotate_to`/`translate_to`. Closed by [[T-0054-animations]]. Native vsync ticker is Phase-6 (ADR-0019).
> - [[RFC-0011-dependency-injection]] — `service_collection` / `service_provider` (singleton/transient/factory/interface→impl) + `app_builder`. Closed by [[T-0055-dependency-injection]].
> - [[RFC-0012-fonts]] — `font` descriptor + alias `font_registry` (`configure_fonts`). Closed by [[T-0056-fonts]].
> - [[RFC-0013-essentials]] — device-API core: `preferences` / `secure_storage` / `connectivity` / `device_info` (interface + in-memory mock, DI-injectable backends). Closed by [[T-0057-essentials-core]].
> - **Value converters** ([[T-0058-value-converters]]) — `invert_bool` / `bool_to_visibility` / `format_with` for the binding converter slot (completes RFC-0007).
> - [[RFC-0014-commanding]] — `relay_command` / `relay_command_of<T>` (ICommand) + `basic_button::command`. Closed by [[T-0059-commanding]]. Completes the MVVM trio (binding + converters + commanding).
> - **CarouselView** — the last MAUI widget gap, filled mock-first ([[Components/CarouselView]]).
>
> CI test count is **477 / 477 green on Linux WSL** (351 → 477 across the gap-closure program). **The entire platform-neutral framework layer is now implemented** — every "ABSENT" subsystem from the MAUI-parity audit (data binding, converters, commanding, triggers, behaviors, effects, animations, DI, fonts, Essentials) plus the widget gap (CarouselView). **Android emulator e2e is live** (render + tap interaction + on-device binding, via `adb screencap`/`input` — `tools/dev/android-e2e.ps1`). Remaining program work is the heavier per-platform-native + tooling half: real handlers (gestures/images/VSM-routing/carousel/fonts/essentials backends), ~~real event loops (ADR-0019)~~ (**done** — real glib_dispatcher / WinUI DispatcherQueue / Android Looper-Handler main-thread dispatchers via a runtime install hook, `d5db702`; macOS/iOS GCD blind), the mpapp-xc XAML-compiler buildout, hot-reload/LSP, and the Apple blind-write port. **Windows is back as a green CI gate** (T-0032 Path B): core + full suite build SDK-free on MSVC. **Android build repaired** (APK builds again). Every subsystem above is verified on Linux (ctest) + Android NDK (compile, both ABIs) + Windows MSVC; the config subsystems are platform-neutral (real on all three by construction). Remaining program work (per the gap-closure plan): per-platform real handlers for widgets/gestures/images, the mpapp-xc XAML-compiler buildout, hot-reload/LSP, and the Apple blind-write port. (Real per-platform event loops landed — ADR-0019 dispatchers, `d5db702`.)

> [!important] New direction — the УИСС reference application ([[RFC-0015-uiss-reference-app]] / [[T-0060-uiss-reference-app]])
> The framework now has its first **real application**, not a single-widget spike: `examples/uiss` — a single ifdef-free `mpapp::application` replicating the TU-Sofia "Е-Студент" student portal (login gate + ≡ flyout navigation across 10 data-bound section pages: Информация / Здравно осигуряване / Заверки и оценки / Спорт / Стипендии / Общежития / Плащания / Идентификация / История на влизанията / Помощна информация). Reference screens: `examples/УИСС/*.mhtml`.
>
> **Verification:** Linux/GTK4 **builds + runs** (window composes, no crash); Windows/WinUI 3 **builds** (`uiss.exe` linked); Android NDK **cross-compiles on both ABIs** (aarch64 + x86_64, Cyrillic UTF-8 literals OK) — the full "Win + Linux + Android" build bar. On-screen capture is blocked by the WSLg/RAIL wall (Linux) — Android `adb screencap` / Windows native capture are the visual routes (deferred while the host PC is in use).
>
> **The app is a gap detector.** Building a real screen surfaced the next priorities. Closed so far: ~~`label` font/color/weight~~ (**[[T-0061-label-typography]]** — `font_size`/`font_bold`/`font_family`/`text_color` real on every platform; УИСС headings/titles styled in TU navy); ~~Плащания `picker`~~ (real `mpapp::picker` dropdown). Also closed: ~~Essentials backend~~ ([[T-0062-essentials-file-preferences]] — real file-backed `preferences`, no ifdefs; УИСС remembers the faculty number). Also started: ~~accessibility~~ ([[T-0063-accessibility-semantics]]: `view::semantic_description` applied on `basic_button` **and `basic_entry`** across platforms; УИСС names the ≡ hamburger + both login fields). Also closed: ~~image loaders~~ ([[T-0065-image-loaders]] — file/URI loading was already real on GTK/WinUI/Android; УИСС now shows the TU logo via `mpapp::image`). Also closed: ~~VSM input-routing~~ ([[T-0066-vsm-input-routing]] — `visual_state_input_router` maps enabled/pressed/pointer-over/focused → the canonical CommonStates via `go_to_state`; platform-neutral + mock-tested). Still open: data tables → `CollectionView`/`TableView` typed items; RFC-0005 styles for app-wide theming; ~~real event loops~~ (real main-thread dispatchers — glib / DispatcherQueue / Looper-Handler — installed at app startup; macOS/iOS GCD blind; `d5db702`); ~~CarouselView real~~ (**`mpapp::carousel_view`** real on Win/Linux/Android — FlipView / GtkStack+swipe / ViewFlipper `30b25b1`; Apple blind; УИСС Информация announcements carousel); ~~gestures real Win/Android~~ (**tap** real on all three now — Linux GtkGesture, Windows Tapped+pointer `1bd5eeb`, Android `MppGestureRouter` `0486df9` per [[T-0039-android-gesture-attach]]; pan/pinch/swipe/pointer the per-platform follow-up); ~~hot-reload~~ (**[[T-0064-hot-reload-linux]]** — Linux `dlopen` runtime real + tested; Windows already real). The app tells us what to harden next.

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

1. **macOS + iOS sweep** across the entire widget set. Requires an Apple host. Existing Objective-C++ handlers on app-shell are the template; the rest need to follow.
2. **M-09 tooling & DX polish.** `mpapp` CLI / `mpapp-xc` XAML compiler / `mpapp-jni-gen` / LSP / hot-reload daemon — all cross-platform per Rule 12. Hot-reload skeleton exists (`src/hot_reload/windows.cpp`); rest is greenfield.
3. **Cross-cutting tests for real handlers.** Mock-handler tests cover the surface contract; real-handler behavior is verified only through end-to-end builds + spot-checks. Worth a `tests/integration/` pass once a CI matrix is set up.
4. **Per-platform GUI screenshot capture infra.** DComp wall on WinUI 3 + missing `wlr-screencopy-unstable-v1` on WSLg blocked auto-capture of the T-0028 / T-0029 / T-0031 Linux + Android GUI shots (Windows shots succeeded via PowerShell BitBlt off MainWindowHandle). A reusable capture utility — either a snapshot extension to the tests harness or a dev-mode helper that does platform-specific window-bitblit — would unlock future closures.

## Recently shipped milestones

- [[40_Roadmap/M-04c-handler-heavy-port|M-04c]] (`shipped`) — all gated ADRs accepted; all T-0028 / T-0029 / T-0030 / T-0031 closure tasks archived per Rule 11. Canvas facade has Cairo + Skia backends (Skia auto-fetched per-platform via `cmake/MpappFindSkia.cmake` — HumbleUI for Linux/Android/macOS, MPAPP-hosted /MD prebuilt for Windows). GraphicsView + ShapeView migrated to the facade end-to-end.

## Active milestone

No active milestone — M-04c shipped. Pick from the priority list above or look at [[40_Roadmap/M-09-Tooling-DX|M-09]] / [[40_Roadmap/M-10-Ecosystem|M-10]] to start a new one.

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
