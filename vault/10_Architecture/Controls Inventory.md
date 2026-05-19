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

## Snapshot (2026-05-12)

| Component | MAUI handler folder | Status |
|---|---|---|
| [[Components/ActivityIndicator\|ActivityIndicator]] | `Handlers/ActivityIndicator/` | not-started |
| [[Components/Application\|Application]] | `Handlers/Application/` | android-real (Windows + Linux + Android verified live) |
| [[Components/BindableLayout\|BindableLayout]] | `Controls/BindableLayout/` | mock |
| [[Components/Border\|Border]] | `Handlers/Border/` | mock |
| [[Components/BoxView\|BoxView]] | `Controls/BoxView/` | mock |
| [[Components/Button\|Button]] | `Handlers/Button/` | android-real (Windows + Linux + Android verified live) |
| [[Components/CheckBox\|CheckBox]] | `Handlers/CheckBox/` | android-real (Windows + Linux + Android; bidirectional bool-binding live-verified on Android, shared compound-button JNI bridge with Switch) |
| [[Components/ContentPage\|ContentPage]] | `Controls/ContentPage/` | not-started |
| [[Components/ContentView\|ContentView]] | `Handlers/ContentView/` | not-started |
| [[Components/DatePicker\|DatePicker]] | `Handlers/DatePicker/` | not-started |
| [[Components/Editor\|Editor]] | `Handlers/Editor/` | android-real (Windows multi-line TextBox + Linux GtkTextView + Android multi-line EditText; shares text-binding bridge with Entry via shared MppTextWatcher kind discriminator) |
| [[Components/Element\|Element]] | `Handlers/Element/` | not-started |
| [[Components/Entry\|Entry]] | `Handlers/Entry/` | android-real (Windows + Linux + Android; bidirectional text-binding live-verified on Android) |
| [[Components/FlyoutPage\|FlyoutPage]] | `Controls/FlyoutPage/` | not-started |
| [[Components/FlyoutView\|FlyoutView]] | `Handlers/FlyoutView/` | not-started |
| [[Components/Grid\|Grid]] | `Layouts/GridLayoutManager/` | mock |
| [[Components/Frame\|Frame]] | `Controls/Frame/` | mock |
| [[Components/GraphicsView\|GraphicsView]] | `Handlers/GraphicsView/` | not-started |
| [[Components/HybridWebView\|HybridWebView]] | `Handlers/HybridWebView/` | not-started |
| [[Components/Image\|Image]] | `Handlers/Image/` | not-started |
| [[Components/ImageButton\|ImageButton]] | `Handlers/ImageButton/` | not-started |
| [[Components/IndicatorView\|IndicatorView]] | `Handlers/IndicatorView/` | not-started |
| [[Components/Label\|Label]] | `Handlers/Label/` | android-real (Windows + Linux + Android verified live) |
| [[Components/Layout\|Layout]] | `Handlers/Layout/` | mock |
| [[Components/ListView\|ListView]] | `Controls/ListView/` | not-started |
| [[Components/MenuBar\|MenuBar]] | `Handlers/MenuBar/` | not-started |
| [[Components/MenuBarItem\|MenuBarItem]] | `Handlers/MenuBarItem/` | not-started |
| [[Components/MenuFlyout\|MenuFlyout]] | `Handlers/MenuFlyoutHandler/` | not-started |
| [[Components/MenuFlyoutItem\|MenuFlyoutItem]] | `Handlers/MenuFlyoutItem/` | not-started |
| [[Components/MenuFlyoutSeparator\|MenuFlyoutSeparator]] | `Handlers/MenuFlyoutSeparator/` | not-started |
| [[Components/MenuFlyoutSubItem\|MenuFlyoutSubItem]] | `Handlers/MenuFlyoutSubItem/` | not-started |
| [[Components/NavigationPage\|NavigationPage]] | `Handlers/NavigationPage/` | not-started |
| [[Components/Page\|Page]] | `Handlers/Page/` | mock |
| [[Components/Picker\|Picker]] | `Handlers/Picker/` | not-started |
| [[Components/ProgressBar\|ProgressBar]] | `Handlers/ProgressBar/` | not-started |
| [[Components/RadioButton\|RadioButton]] | `Handlers/RadioButton/` | android-real (Windows + Linux + Android; auto-grouping via group_name; shared compound-button JNI bridge kind=3) |
| [[Components/RefreshView\|RefreshView]] | `Handlers/RefreshView/` | not-started |
| [[Components/ScrollView\|ScrollView]] | `Handlers/ScrollView/` | android-real (Windows ScrollViewer + Linux GtkScrolledWindow + Android ScrollView; `bind_content(scroll_view&, view&)` helper wraps a non-owning child as null-deleter `shared_ptr<view>`; Android spike wraps the full widget stack and renders through it) |
| [[Components/SearchBar\|SearchBar]] | `Handlers/SearchBar/` | not-started |
| [[Components/ShapeView\|ShapeView]] | `Handlers/ShapeView/` | not-started |
| [[Components/Shell\|Shell]] | `Controls/Shell/` | not-started |
| [[Components/Slider\|Slider]] | `Handlers/Slider/` | android-real (Windows + Linux + Android; bidirectional double-range binding live-verified on Android via SeekBar→int progress→double remap) |
| [[Components/StackLayout\|StackLayout]] | `Layouts/StackLayoutManager/` | android-real (Windows + Linux + Android verified live) |
| [[Components/Stepper\|Stepper]] | `Handlers/Stepper/` | android-real (Windows NumberBox + Linux GtkSpinButton + Android NumberPicker; double↔int step-index remap honoring `interval`) |
| [[Components/SwipeItemMenuItem\|SwipeItemMenuItem]] | `Handlers/SwipeItemMenuItem/` | not-started |
| [[Components/SwipeItemView\|SwipeItemView]] | `Handlers/SwipeItemView/` | not-started |
| [[Components/SwipeView\|SwipeView]] | `Handlers/SwipeView/` | not-started |
| [[Components/Switch\|Switch]] | `Handlers/Switch/` | android-real (Windows + Linux + Android; bidirectional bool-binding live-verified on Android) |
| [[Components/TabbedPage\|TabbedPage]] | `Controls/TabbedPage/` | not-started |
| [[Components/TabbedView\|TabbedView]] | `Handlers/TabbedView/` | not-started |
| [[Components/TableView\|TableView]] | `Controls/TableView/` | not-started |
| [[Components/TemplatedView\|TemplatedView]] | `Controls/TemplatedView/` | not-started |
| [[Components/TimePicker\|TimePicker]] | `Handlers/TimePicker/` | not-started |
| [[Components/TitleBar\|TitleBar]] | `Controls/TitleBar/` | not-started |
| [[Components/Toolbar\|Toolbar]] | `Handlers/Toolbar/` | not-started |
| [[Components/View\|View]] | `Handlers/View/` | mock |
| [[Components/WebView\|WebView]] | `Handlers/WebView/` | not-started |
| [[Components/Window\|Window]] | `Handlers/Window/` | android-real (Windows + Linux + Android verified live) |

**Total: 55 components.**

## How to update

When a control reaches a new status:

1. Update its frontmatter `mpappStatus:` in `Components/<Name>.md`.
2. Flip the matching `platformWindows`/`Android`/`Linux`/`Macos`/`Ios` boolean.
3. The [[_Bases/Components.base]] live view will reflect the change automatically.
4. The static snapshot above is updated by hand when a milestone closes.

## See also

- [[XAML Compatibility]]
- [[Components/README]]
- [[Handlers]]
- [[ADR-0004-maui-xaml-superset-compat]]
- `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\`
- `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\`
