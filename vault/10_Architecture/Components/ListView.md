---
type: component
mauiHandler: "ListView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/listview"
mpappStatus: not-started
platformWindows: false
platformAndroid: false
platformLinux: false
platformMacos: false
platformIos: false
tags:
  - type/component
  - status/not-started
---

# ListView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`ListView` is a vertically-scrolling, single-column collection control that renders one cell per item with built-in pull-to-refresh, grouping, selection, and header/footer support. In MAUI it derives from `ItemsView<Cell>` and is the historical predecessor to `CollectionView`; MAUI itself flags it as obsolete with `[Obsolete("ListView is deprecated. Please use CollectionView instead.")]`. MPAPP ports it for parity and migration purposes, but new code should prefer `CollectionView` (see [[CollectionView]]).

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Compatibility\Handlers\ListView\` (compat renderers; no `Microsoft.Maui.Handlers` mapper — ListView still uses the Forms-era renderer model)
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\ListView\ListView.cs`
- **Docs:** [Microsoft .NET MAUI — ListView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/listview)

## MPAPP C++ API

```cpp
namespace mpapp {

enum class listview_selection_mode { none, single };
enum class separator_visibility { platform_default, none };
enum class scrollbar_visibility { platform_default, always, never };

class listview : public control<listview> {
public:
    // Items source — backed by an observable vector so insert/remove/replace
    // events drive the platform adapter without full reloads.
    Observable<observable_vector<std::any>> items_source;
    Observable<data_template>               item_template;

    // Selection.
    Observable<std::any>                  selected_item;
    Observable<listview_selection_mode>   selection_mode;

    // Layout / appearance.
    Observable<bool>                      has_uneven_rows;
    Observable<int>                       row_height;
    Observable<separator_visibility>      separator_mode;
    Observable<color>                     separator_color;
    Observable<scrollbar_visibility>      horizontal_scrollbar_visibility;
    Observable<scrollbar_visibility>      vertical_scrollbar_visibility;

    // Header / footer.
    Observable<std::any>                  header;
    Observable<data_template>             header_template;
    Observable<std::any>                  footer;
    Observable<data_template>             footer_template;

    // Grouping.
    Observable<bool>                      is_grouping_enabled;
    Observable<data_template>             group_header_template;

    // Pull-to-refresh.
    Observable<bool>                      is_pull_to_refresh_enabled;
    Observable<bool>                      is_refreshing;
    Observable<color>                     refresh_control_color;
    Command<>                             refresh_command;

    // Events.
    event<item_tapped_args>     item_tapped;
    event<item_selected_args>   item_selected;
    event<scroll_to_args>       scroll_to_requested;

    // Methods.
    void scroll_to(const std::any& item, scroll_to_position pos, bool animated);
    void begin_refresh();
    void end_refresh();
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<ListView ItemsSource="{Binding People}"
          SelectionMode="Single"
          HasUnevenRows="true"
          IsPullToRefreshEnabled="true"
          IsRefreshing="{Binding IsBusy}"
          RefreshCommand="{Binding RefreshCommand}">
    <ListView.ItemTemplate>
        <DataTemplate>
            <TextCell Text="{Binding Name}" Detail="{Binding Email}"/>
        </DataTemplate>
    </ListView.ItemTemplate>
</ListView>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.ListView` | C++/WinRT | Wraps WinUI 3 ListView; templated items via `DataTemplate`; refresh via `RefreshContainer`. |
| Android | `android.widget.ListView` (`AListView`) wrapped in `SwipeRefreshLayout` | fbjni / JNI | Uses a `BaseAdapter` per `ListViewAdapter.cs`; grouping via `GroupedListViewAdapter`. |
| Linux | `GtkListBox` inside a `GtkScrolledWindow` (with optional `GtkOverlay` for spinner) | GTK4 | No native pull-to-refresh; MPAPP renders a custom spinner overlay when `is_pull_to_refresh_enabled` is true. |
| macOS | `NSTableView` (single-column) inside `NSScrollView` | AppKit | MAUI uses `UITableView` on Catalyst; MPAPP uses native AppKit on macOS proper per [[ADR-0005-ios-macos-separate-interop]]. |
| iOS | `UITableView` | UIKit | Cells subclass `UITableViewCell`; context actions via `ContextActionCell`; refresh via `UIRefreshControl`. |

## Side-by-side Examples

### MAUI

```xml
<ListView ItemsSource="{Binding Items}" SelectionMode="Single">
    <ListView.ItemTemplate>
        <DataTemplate>
            <TextCell Text="{Binding Title}"/>
        </DataTemplate>
    </ListView.ItemTemplate>
</ListView>
```

### MPAPP (XAML)

```xml
<ListView ItemsSource="{Binding Items}" SelectionMode="Single">
    <ListView.ItemTemplate>
        <DataTemplate>
            <TextCell Text="{Binding Title}"/>
        </DataTemplate>
    </ListView.ItemTemplate>
</ListView>
```

### MPAPP (C++)

```cpp
auto lv = std::make_shared<mpapp::listview>();
lv->items_source.get().push_back(person{"Ada", "ada@lovelace.dev"});
lv->items_source.get().push_back(person{"Alan", "alan@turing.dev"});
lv->selection_mode = mpapp::listview_selection_mode::single;
lv->item_selected.connect([](auto& args) {
    mpapp::log::info("Selected: {}", args.selected_item.template get<person>().name);
});
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/listview/mock_test.cpp` (planned)
- Windows handler: `tests/components/listview/windows_test.cpp` (planned)
- Android handler: `tests/components/listview/android_test.cpp` (planned)
- Linux handler: `tests/components/listview/linux_test.cpp` (planned)
- macOS handler: `tests/components/listview/macos_test.cpp` (planned)
- iOS handler: `tests/components/listview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Deprecation | `[Obsolete]` — kept for source-compat only | Same — documented as legacy; CollectionView is preferred | Mirror MAUI guidance | — |
| Caching strategy | `RetainElement` / `RecycleElement` enum at construction | Always recycles via `observable_vector` keying | C++ does not need GC-driven retain mode | — |
| Items source type | `IEnumerable` (any sequence) | `observable_vector<T>` required for incremental updates | Compile-time observable contract per [[ADR-0009-public-api-template-wrappers-only]] | — |
| `ListViewCachingStrategy` ctor parameter | Constructor takes `cachingStrategy` | Constructor takes no strategy | Strategy is implicit in `observable_vector` | — |
| macOS native control | `UITableView` via Mac Catalyst | `NSTableView` via AppKit | [[ADR-0005-ios-macos-separate-interop]] | — |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[CollectionView]]
- [[RefreshView]]
