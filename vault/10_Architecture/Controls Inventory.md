---
type: moc
area: handlers
tags:
  - area/handlers
  - area/markup
---

# Controls Inventory

Authoritative list of every MAUI control / handler MPAPP must implement, with porting status. Each row links to its per-component note at `Components/<Name>.md`.

Generated from `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\` and `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\`.

**Porting status** state machine: `not-started → mock → windows-real → android-real → linux-real → macos-real → ios-real → parity-complete`.

> [!info] Live view
> Open [[_Bases/Components.base]] for the live, filterable, sortable view. The table below is a static snapshot.

> [!tip] Wrapper-component layering (ADR-0024)
> Every concrete component in this inventory has been migrated to the two-layer pattern from [[ADR-0024-wrapper-component-pattern]] (commit `4754ac1`): a `mpapp::internal::basic_<name>` surface in `include/mpapp/internal/basic_<name>.hpp` + a `mpapp::<name>` wrapper in `include/mpapp/<name>.hpp` that embeds the platform handler by value and auto-binds it in its constructor. The handler classes themselves are unchanged.
>
> Two narrow categories opt out of the wrapper layer (documented exceptions in their per-component notes):
> - [[Components/Application|Application]] — program-entry class; handler is created externally by `mpapp::run<App>`.
> - [[Components/BindableLayout|BindableLayout]] — static attached-property facility; no instance to wrap.
>
> CRTP / abstract bases ([[Components/View|View]], [[Components/Layout|Layout]], [[Components/Cell|Cell]], [[Components/Element|Element]]) are inheritance roots, not leaf components, and do not participate in the surface / wrapper split.

## Snapshot (2026-06-04)

| Component | MAUI handler folder | Status |
|---|---|---|
| [[Components/AbsoluteLayout\|AbsoluteLayout]] | `Controls/Layout/AbsoluteLayout/` | mock (surface + mock handler + 13 tests; per-child `layout_bounds` rect + `layout_flags` proportional bitmask attached store mirroring Grid. Win `Canvas` / Linux `GtkFixed` / Android absolute-placement real handlers written blind, **unverified** — `<platform>-real` follow-up) |
| [[Components/ActivityIndicator\|ActivityIndicator]] | `Handlers/ActivityIndicator/` | android-real (Windows mux::ProgressRing + Linux GtkSpinner + Android indeterminate ProgressBar; is_running toggle + color tint via brush_ref) |
| [[Components/Application\|Application]] | `Handlers/Application/` | android-real (Windows + Linux + Android verified live) |
| [[Components/BindableLayout\|BindableLayout]] | `Controls/BindableLayout/` | android-real (Win StackPanel + Linux GtkBox vertical + Android LinearLayout; items_source rebuilds children on collection change; item_template is recorded but template instantiation deferred to templating ADR) |
| [[Components/Border\|Border]] | `Handlers/Border/` | android-real (Windows mux::Border + Linux GtkBox CSS provider + Android FrameLayout with GradientDrawable; stroke + per-corner radius from `RoundRectangle(N)` descriptor; dash/cap/join deferred) |
| [[Components/BoxView\|BoxView]] | `Controls/BoxView/` | android-real (Windows Border+SolidColorBrush+CornerRadius + Linux GtkBox with per-instance CSS provider + Android View with GradientDrawable; spike's box corner-radius tracks slider value live) |
| [[Components/Button\|Button]] | `Handlers/Button/` | android-real (Windows + Linux + Android verified live) |
| [[Components/CheckBox\|CheckBox]] | `Handlers/CheckBox/` | android-real (Windows + Linux + Android; bidirectional bool-binding live-verified on Android, shared compound-button JNI bridge with Switch) |
| [[Components/ContentPage\|ContentPage]] | `Controls/ContentPage/` | android-real (Win Page wrapping Grid + Linux GtkBox vertical w/ GtkLabel + content + Android LinearLayout vertical w/ TextView + FrameLayout; title + content + padding) |
| [[Components/ContentView\|ContentView]] | `Handlers/ContentView/` | android-real (Win mux::ContentControl + Linux GtkBox + Android FrameLayout; single content slot resolved via ADR-0013 dispatch registry; legacy dynamic_cast chain removed) |
| [[Components/DatePicker\|DatePicker]] | `Handlers/DatePicker/` | android-real (Win CalendarDatePicker + Linux GtkCalendar + Android DatePicker; date_value POD year/month/day; format string applied on Windows only) |
| [[Components/Editor\|Editor]] | `Handlers/Editor/` | android-real (Windows multi-line TextBox + Linux GtkTextView + Android multi-line EditText; shares text-binding bridge with Entry via shared MppTextWatcher kind discriminator) |
| [[Components/CarouselView\|CarouselView]] | `Handlers/CarouselView/` | android-real (`mpapp::carousel_view` wrapper + real per-platform handlers: Windows mux::FlipView (SelectedIndex↔position, SelectionChanged→position_changed) + Linux GtkStack pages + GtkGestureSwipe fling→scroll_to + Android android.widget.ViewFlipper setDisplayedChild. `basic_carousel_view` surface: items_source + position + loop + is_swipe_enabled + peek_count + PositionChanged; scroll_to wraps/clamps per loop. macOS/iOS NSView/UIView one-page-visible blind (compiled+run on a Mac: PENDING). Exercised in УИСС (Информация announcements carousel). Native-swipe gating + peek-insets + ViewPager2/Adw.Carousel are per-platform follow-ups.) |
| [[Components/CollectionView\|CollectionView]] | `Handlers/CollectionView/` | android-real (stable outer container — mux::Border / GtkScrolledWindow / FrameLayout — wraps an inner widget that swaps per `layout` enum. **All four layouts wired across all three platforms** (T-0028): Windows uses ItemsPanelTemplate-driven ListView/GridView with orientation per layout + ScrollViewer attached props; Linux uses GtkListBox / GtkBox(HORIZONTAL) / GtkFlowBox(HORIZONTAL or VERTICAL) with matching scrollbar policy; Android migrated to androidx.recyclerview.widget.RecyclerView with LinearLayoutManager / GridLayoutManager per layout. Multi-select round-trip + item_tapped + selected_indices wired on all 3 platforms; the new MppCollectionAdapter pushes Android's multi-select set via the package-private MppItemClickRouter.nativeDispatchCheckedSet JNI hook. `typed_items` parallel surface for rendering arbitrary view*/cell* per item via ADR-0013 dispatch — flows through the same RecyclerView on Android. Linux horizontal_list has no native multi-select visual highlight (GtkBox design); Android GridLayoutManager default-span=2 since RecyclerView has no AUTO_FIT equivalent.) |
| [[Components/Element\|Element]] | `Handlers/Element/` | mock — terminal (abstract base; no native primitive) |
| [[Components/Entry\|Entry]] | `Handlers/Entry/` | android-real (Windows + Linux + Android; bidirectional text-binding live-verified on Android) |
| [[Components/FlyoutPage\|FlyoutPage]] | `Controls/FlyoutPage/` | android-real (Win mux::SplitView w/ Pane+Content + Linux GtkPaned horizontal + Android LinearLayout horizontal w/ flyout/detail FrameLayout pair; is_presented toggles flyout visibility / IsPaneOpen) |
| [[Components/FlyoutView\|FlyoutView]] | `Handlers/FlyoutView/` | android-real (Win NavigationView + Linux GtkPaned drawer + Android DrawerLayout w/ LinearLayout fallback; flyout + detail + is_presented) |
| [[Components/FlexLayout\|FlexLayout]] | `Controls/Layout/FlexLayout/` | mock (surface + mock handler + 11 tests; container direction/wrap/justify/align_items/align_content/position + per-child order/grow/shrink/basis/align_self attached store mirroring Grid. Win/Linux/Android real handlers written blind, **unverified**; a faithful flexbox arrange engine is the substantive `<platform>-real` follow-up) |
| [[Components/Grid\|Grid]] | `Layouts/GridLayoutManager/` | android-real (Win mux::Controls::Grid w/ RowDefinitions+ColumnDefinitions populated from track_def via GridLength; Linux GtkGrid w/ track_def→hexpand/vexpand bridging; Android android.widget.GridLayout w/ GridLayout.LayoutParams from grid_layout::cell_placement. Per-child placement via grid.set_row/set_column/set_row_span/set_column_span keyed on the child view*. Row/column spacing wired on Win + Linux; Android spacing deferred per platform.) |
| [[Components/Frame\|Frame]] | `Controls/Frame/` | android-real (Win mux::Border + Linux GtkBox + Android FrameLayout; [[deprecated]] alias for Border) |
| [[Components/GraphicsView\|GraphicsView]] | `Handlers/GraphicsView/` | android-real (v1 native drawing host — Win muxc::Canvas + Linux GtkDrawingArea + Android android.view.View; width/height propagated to native widget. User-facing draw API still gated on ADR-0015 v2 canvas facade — invalidate()/draw_count/draw_requested signal keep the mock contract for consumers.) |
| [[Components/HybridWebView\|HybridWebView]] | `Handlers/HybridWebView/` | android-real (Win muxc::WebView2 + CoreWebView2.WebMessageReceived/PostWebMessageAsString + AddScriptToExecuteOnDocumentCreatedAsync for shim; Linux WebKitGTK UserContentManager + register_script_message_handler("mpapp_send") + WebKitUserScript at document-start + evaluate_javascript for outbound; Android android.webkit.WebView + @JavascriptInterface MppJsBridge + addJavascriptInterface("mpapp_native") + evaluateJavascript for outbound. Each platform injects `window.mpapp.{send,on,_receive}` shim into the loaded document. Linux stubs out when WebKitGTK is absent.) |
| [[Components/Image\|Image]] | `Handlers/Image/` | android-real (Win mux::Image + BitmapImage from file:// URI + Linux GtkPicture set_filename + Android ImageView + BitmapFactory.decodeFile; aspect_mode → Stretch/ContentFit/ScaleType) |
| [[Components/ImageButton\|ImageButton]] | `Handlers/ImageButton/` | android-real (Win Button + Image content; Linux GtkButton + GtkPicture child; Android ImageButton + BitmapFactory.decodeFile; click event deferred to M-05 polish) |
| [[Components/IndicatorView\|IndicatorView]] | `Handlers/IndicatorView/` | android-real (Win StackPanel of Ellipses + Linux GtkBox of dot labels + Android LinearLayout of GradientDrawable-backed Views; count + position + colors; companion to CarouselView which is M-04c) |
| [[Components/Label\|Label]] | `Handlers/Label/` | android-real (Windows + Linux + Android verified live) |
| [[Components/Layout\|Layout]] | `Handlers/Layout/` | mock — terminal (abstract base; concrete layouts like StackLayout/Grid implement real handlers) |
| [[Components/ListView\|ListView]] | `Controls/ListView/` | android-real (Win mux::Controls::ListView w/ Items() collection + Linux GtkListBox in GtkScrolledWindow + Android android.widget.ListView w/ ArrayAdapter<String>; items_source binds via wrap-platform-recycler per ADR-0020; selection round-trips Win↔C++ and Linux↔C++; Android OnItemClick router deferred to M-05) |
| [[Components/MenuBar\|MenuBar]] | `Handlers/MenuBar/` | android-real (Win mux::MenuBar + Linux GtkBox of GtkMenuButton + Android Toolbar acting as menu host; items rebuild on collection change, drops child views the dispatch registry does not resolve) |
| [[Components/MenuBarItem\|MenuBarItem]] | `Handlers/MenuBarItem/` | android-real (Win mux::MenuBarItem + Linux GtkMenuButton with title label + Android TextView surfaced into parent menu_bar's Menu; title + items collection observed, popover/flyout children land with M-04c menu_flyout family) |
| [[Components/MenuFlyout\|MenuFlyout]] | `Handlers/MenuFlyoutHandler/` | android-real (Win mux::Controls::MenuFlyout + Linux GtkPopover w/ vertical GtkBox + Android vertical LinearLayout; items + is_open via ADR-0013 dispatch registry) |
| [[Components/MenuFlyoutItem\|MenuFlyoutItem]] | `Handlers/MenuFlyoutItem/` | android-real (Win mux::Controls::MenuFlyoutItem w/ Click → clicked + Linux flat-styled GtkButton w/ "clicked" signal + Android android.widget.Button; text + is_enabled + clicked, Android click router deferred to M-05) |
| [[Components/MenuFlyoutSeparator\|MenuFlyoutSeparator]] | `Handlers/MenuFlyoutSeparator/` | android-real (Win mux::Controls::MenuFlyoutSeparator + Linux horizontal GtkSeparator + Android 1-px-min-height android.view.View; pure marker type, no observable properties) |
| [[Components/MenuFlyoutSubItem\|MenuFlyoutSubItem]] | `Handlers/MenuFlyoutSubItem/` | android-real (Win mux::Controls::MenuFlyoutSubItem + Linux GtkMenuButton wrapping nested GtkPopover + Android vertical LinearLayout w/ TextView header + inner items host; text + items via ADR-0013 dispatch) |
| [[Components/NavigationPage\|NavigationPage]] | `Handlers/NavigationPage/` | android-real (Win mux::Page+Grid w/ back-button bar + Linux GtkBox vertical w/ horizontal bar + Android LinearLayout vertical w/ tab strip; page_stack engine drives apply_top swap on did_appear; back-button visibility tracks stack_depth > 1) |
| [[Components/Page\|Page]] | `Handlers/Page/` | android-real (Win mux::Page + Linux GtkBox vertical + Android LinearLayout vertical; title via TextBlock/GtkLabel/TextView, content via FrameLayout/Grid/Box) |
| [[Components/Picker\|Picker]] | `Handlers/Picker/` | android-real (Win ComboBox + Linux GtkDropDown/GtkStringList + Android Spinner+ArrayAdapter; items + selected_index + title; title deferred on Linux/Android) |
| [[Components/ProgressBar\|ProgressBar]] | `Handlers/ProgressBar/` | android-real (Windows ProgressBar + Linux GtkProgressBar + Android determinate ProgressBar; normalized 0..1 progress + color + background_color) |
| [[Components/RadioButton\|RadioButton]] | `Handlers/RadioButton/` | android-real (Windows + Linux + Android; auto-grouping via group_name; shared compound-button JNI bridge kind=3) |
| [[Components/RefreshView\|RefreshView]] | `Handlers/RefreshView/` | android-real (Win RefreshContainer + Linux GtkBox with GtkSpinner overlay + Android SwipeRefreshLayout w/ FrameLayout+ProgressBar fallback; is_refreshing surface) |
| [[Components/ScrollView\|ScrollView]] | `Handlers/ScrollView/` | android-real (Windows ScrollViewer + Linux GtkScrolledWindow + Android ScrollView; `bind_content(scroll_view&, view&)` helper wraps a non-owning child as null-deleter `shared_ptr<view>`; Android spike wraps the full widget stack and renders through it) |
| [[Components/SearchBar\|SearchBar]] | `Handlers/SearchBar/` | android-real (Win AutoSuggestBox with Find icon + Linux GtkSearchEntry + Android SearchView; text + placeholder) |
| [[Components/ShapeView\|ShapeView]] | `Handlers/ShapeView/` | android-real (v1 per-platform native primitives — Win muxc::Border w/ muxc::Shapes::Rectangle/Ellipse/Line; Linux GtkDrawingArea + cairo draw callback; Android custom MppShapeView w/ onDraw. hex color parsing for fill/stroke; "M x1 y1 L x2 y2" for line data. polygon+path render as bounding rect — full SVG path parsing + unified canvas facade deferred to ADR-0015 v2.) |
| [[Components/Shell\|Shell]] | `Controls/Shell/` | android-real (Win mux::SplitView w/ tab StackPanel + Linux GtkPaned w/ tab GtkBox + Android LinearLayout w/ tab strip + content swap; tabs trigger current_tab_index on click via MppActionRouter kind=1; is_flyout_open toggles Pane visibility; current_content swaps via ADR-0013 dispatch. Android tab buttons restyle on selection — primary-blue+bold for active, grey+regular for inactive (shares palette w/ tabbed_page). Compile-time route table per ADR-0016 deferred.) |
| [[Components/Slider\|Slider]] | `Handlers/Slider/` | android-real (Windows + Linux + Android; bidirectional double-range binding live-verified on Android via SeekBar→int progress→double remap) |
| [[Components/StackLayout\|StackLayout]] | `Layouts/StackLayoutManager/` | android-real (Windows + Linux + Android verified live) |
| [[Components/Stepper\|Stepper]] | `Handlers/Stepper/` | android-real (Windows NumberBox + Linux GtkSpinButton + Android NumberPicker; double↔int step-index remap honoring `interval`) |
| [[Components/SwipeItemMenuItem\|SwipeItemMenuItem]] | `Handlers/SwipeItemMenuItem/` | android-real (Win `mux::Controls::Button` Content + Click → invoked + Linux `GtkButton` `clicked` → invoked + Android `android.widget.Button` text-only; text + icon_uri live; Android OnClickListener routing deferred — gestures + richer background/is_destructive surface deferred) |
| [[Components/SwipeItemView\|SwipeItemView]] | `Handlers/SwipeItemView/` | android-real (Win `mux::Controls::ContentControl` + Linux `GtkBox` content host + Android `FrameLayout`; renders custom action content inline — gesture-reveal deferred) |
| [[Components/SwipeView\|SwipeView]] | `Handlers/SwipeView/` | android-real (Win `mux::Controls::SwipeControl` w/ `SwipeItems` collection from menu-item entries + Linux `GtkBox` content-only + Android `FrameLayout` content-only — gestures deferred on Linux/Android, items registered via ADR-0013 dispatch) |
| [[Components/Switch\|Switch]] | `Handlers/Switch/` | android-real (Windows + Linux + Android; bidirectional bool-binding live-verified on Android) |
| [[Components/TabbedPage\|TabbedPage]] | `Controls/TabbedPage/` | android-real (Win mux::Pivot + Linux GtkNotebook + Android LinearLayout w/ horizontal tab strip + FrameLayout content host. Native widgets on Win/Linux handle tab clicks + selected-tab visual emphasis automatically. Android: each tab TextView wires MppActionRouter kind=2 routing taps into selected_index; apply_selection restyles the active tab w/ primary-blue color + bold typeface and others w/ grey + regular.) |
| [[Components/TabbedView\|TabbedView]] | `Handlers/TabbedView/` | android-real (Win mux::TabView + Linux GtkNotebook + Android vertical LinearLayout host w/ Button tab strip + FrameLayout page area; tab_titles + selected_index) |
| [[Components/TableView\|TableView]] | `Controls/TableView/` | android-real (Win mux::ListView + Linux GtkListBox + Android android.widget.FrameLayout outer hosting either ListView (flat) or ScrollView+LinearLayout (typed). Two parallel surfaces: `sections` (flat title+vec<string> rows) and `typed_sections` (title+vec<cell*> rows from the ADR-0021 cell tree). When typed_sections is non-empty the handler renders each cell via its native handle through ADR-0013 dispatch; otherwise falls back to flat-string rendering. row_height honoring deferred per platform.) |
| [[Components/TextCell\|TextCell]] | `Controls/Cells/TextCell/` | android-real (Win Border+StackPanel+TextBlock pair + Linux GtkBox+GtkLabel pair + Android LinearLayout+TextView pair; detail row hides on empty; 12/6 padding for native row feel; self-registered for nesting anywhere a view fits) |
| [[Components/EntryCell\|EntryCell]] | `Controls/Cells/EntryCell/` | android-real (Win Border + 2-col Grid (TextBlock auto + TextBox star, InputScope mapping for keyboard_kind, TextChanged echo, KeyDown Enter → completed) + Linux horizontal GtkBox (GtkLabel + GtkEntry, input_purpose mapping, "changed" echo, "activate" → completed) + Android horizontal LinearLayout (TextView + EditText weight=1, setInputType mapping, MppTextWatcher kind=3 echo, new MppEditorActionListener routes IME terminal actions → completed)) |
| [[Components/SwitchCell\|SwitchCell]] | `Controls/Cells/SwitchCell/` | android-real (Win Border+2-col Grid w/ TextBlock + ToggleSwitch.Toggled echo + Linux horizontal GtkBox w/ GtkLabel + GtkSwitch state-set echo + Android horizontal LinearLayout w/ TextView weight=1 + Switch via shared MppCheckedChangeListener kind=4; suppress-echo guard everywhere; emits on_changed on user flips) |
| [[Components/ViewCell\|ViewCell]] | `Controls/Cells/ViewCell/` | android-real (Win Border w/ Child swap + Linux GtkBox single-slot + Android FrameLayout; content resolved via ADR-0013 dispatch; native row padding to match text_cell aesthetic) |
| [[Components/ImageCell\|ImageCell]] | `Controls/Cells/ImageCell/` | android-real (Win Border+Grid (Image 40px auto + StackPanel(TextBlock+TextBlock) star) — BitmapImage handles file/http/ms-appx URIs + Linux horizontal GtkBox (GtkImage 40px + label pair) — `icon:` prefix → themed icon-name, else file path + Android horizontal LinearLayout (ImageView 80px + label pair weight=1) — BitmapFactory.decodeFile) |
| [[Components/TemplatedView\|TemplatedView]] | `Controls/TemplatedView/` | android-real (Win ContentControl + Linux GtkBox single-child + Android FrameLayout; content slot live, template_id deferred to templating engine ADR) |
| [[Components/TimePicker\|TimePicker]] | `Handlers/TimePicker/` | android-real (Win TimePicker + Linux GtkSpinButton pair + Android TimePicker 24h; time_value hour/minute POD) |
| [[Components/TitleBar\|TitleBar]] | `Controls/TitleBar/` | android-real (Win mux::TitleBar + Linux GtkHeaderBar + Android android.widget.Toolbar; title + subtitle) |
| [[Components/Toolbar\|Toolbar]] | `Handlers/Toolbar/` | android-real (Win CommandBar + AppBarButton per item + Linux GtkActionBar + GtkButton per item + Android Toolbar with Menu items; items rebuilt on collection change) |
| [[Components/View\|View]] | `Handlers/View/` | mock — terminal (abstract base; all concrete widgets derive from view) |
| [[Components/WebView\|WebView]] | `Handlers/WebView/` | android-real (Win muxc::WebView2 + NavigationStarting/Completed bound to is_loading/navigating/navigated + can_go_back/forward; Linux WebKitGTK 6.x via "load-changed" signal — LGPL dynamic-link per Rule 9; Android android.webkit.WebView w/ custom MppWebViewClient routing onPageStarted/onPageFinished + INTERNET perm + JS enabled. Linux stubs out cleanly if WebKitGTK is missing at configure time.) |
| [[Components/Window\|Window]] | `Handlers/Window/` | android-real (Windows + Linux + Android verified live) |

**Total: 68 components.** Most widgets are at `android-real` on Win / Linux / Android. **Two new MAUI-parity layouts — [[Components/AbsoluteLayout\|AbsoluteLayout]] and [[Components/FlexLayout\|FlexLayout]] — landed at `mock`** (2026-W23 parity push, [[2026-W23-Weekly]]): surface + mock handler + tests verified on the host harness; their Win/Linux/Android real handlers are written blind and remain `<platform>-real` follow-ups per [[ADR-0008-mock-first-implementation]]. The three abstract bases (View / Layout / Element) and the `Cell` abstract base remain terminal-mock by design — they don't have native primitives, only their subclasses do. ShapeView + GraphicsView v1 ship with per-platform native primitives; the unified canvas facade (Cairo / Skia compile-time selectable) remains v2 scope per [[ADR-0015-graphics-backend-dual]]. macOS + iOS pending Apple host.

> [!note] Beyond the control inventory — MAUI-parity subsystems (2026-W23)
> The same push added non-control surface that lives outside this table: **gradient brushes** (`view::background_brush`, `shadow`, `clip`), **page dialog services** (DisplayAlert / DisplayActionSheet / DisplayPrompt), **modal navigation**, **AppThemeBinding + Application.RequestedTheme**, **templating** (DataTemplateSelector / ControlTemplate / ContentPresenter), and **~33 .NET MAUI Essentials device APIs** (sensors, battery, geolocation, permissions, clipboard, file/media pickers, share, etc. — Essentials went 4 → ~37). See [[2026-W23-Weekly]].

## How to update

When a control reaches a new status:

1. Update its frontmatter `mpappStatus:` in `Components/<Name>.md`.
2. Flip the matching `platformWindows`/`Android`/`Linux`/`Macos`/`Ios` boolean.
3. The [[_Bases/Components.base]] live view will reflect the change automatically.
4. The static snapshot above is updated by hand when a milestone closes.

## See in code

- [`include/mpapp/`](../../include/mpapp/) — one header per component (`button.hpp`, `label.hpp`, `collection_view.hpp`, etc.). The cross-platform surface — same source compiles unchanged on every supported platform.
- [`include/mpapp/handlers/mock/`](../../include/mpapp/handlers/mock/) — 66 mock handlers, one per component, each recording calls into a test-observable vector.
- [`src/handlers/windows/`](../../src/handlers/windows/) · [`src/handlers/linux/`](../../src/handlers/linux/) · [`src/handlers/android/`](../../src/handlers/android/) — 62 + 62 + 69 real platform handlers respectively. Each is the `<component>_handler` partial specialization on the matching platform tag.
- [`src/handlers/macos/`](../../src/handlers/macos/) · [`src/handlers/ios/`](../../src/handlers/ios/) — Objective-C++ `.mm` handlers for the app-shell layer (application / button / label / window — the seed set). Per-component fill-in pending an Apple host (M-07 / M-08).
- [`tests/mock_handlers/`](../../tests/mock_handlers/) — 66 `<component>_test.cpp` files exercising the surface contract; `CONFIGURE_DEPENDS` glob means adding a new component's tests doesn't require touching the test CMakeLists.

## See also

- [[XAML Compatibility]]
- [[Components/README]]
- [[Handlers]]
- [[ADR-0004-maui-xaml-superset-compat]]
- `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\`
- `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\`
