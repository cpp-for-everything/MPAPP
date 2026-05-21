---
type: component
mauiHandler: "CollectionView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/collectionview"
mpappStatus: android-real
platformWindows: true
platformAndroid: true
platformLinux: true
platformMacos: false
platformIos: false
tags:
  - type/component
  - status/android-real
---

# CollectionView

> [!info] Status
> **android-real** — Windows wraps `mux::Controls::ListView` with `SelectionMode` mapped to the cross-platform `collection_selection_mode` enum (None / Single / Multiple). Linux uses `GtkListBox` inside `GtkScrolledWindow` with matching `GtkSelectionMode`. Android wraps `android.widget.ListView` with `setChoiceMode` honoring the same mapping. All three wrap the native recycler per [[ADR-0020-virtualized-item-host-wrap-platform]]. The selection round-trip + tap-to-set is wired on Win/Linux; the Android `OnItemClickListener` router lands with M-05. **V1 limitations:** horizontal + grid layouts (governed by the `layout` enum) ship without per-platform implementation today; multi-select event reporting (events into `selected_indices`) lands when CollectionView's selection event surface lands. Item templates (custom row views) are tied to the cell-type tree in [[ADR-0021-tableview-cell-types]] (cells already shipped 2026-05-22) and a future item_template ADR.

## Overview

`CollectionView` is the modern MAUI replacement for `ListView` — same items-source-binding pattern, but with first-class layout switching (vertical/horizontal list, vertical/horizontal grid with span) and richer selection semantics (none / single / multiple). Item templates are deferred to the upcoming virtualized-item-host ADR; the mock holds plain `std::vector<std::string>` to make the surface compile without dragging in templating infrastructure.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Items\` — see `ItemsViewHandler`, `CollectionViewHandler`, and per-platform `.Windows.cs` / `.Android.cs` / `.iOS.cs` partials.
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Items\CollectionView\CollectionView.cs`
- **Docs:** [Microsoft .NET MAUI — CollectionView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/collectionview)

## MPAPP C++ API (mock surface)

```cpp
namespace mpapp {

enum class collection_selection_mode { none, single, multiple };
enum class collection_layout         { vertical_list, horizontal_list, vertical_grid, horizontal_grid };

class collection_view : public view {
public:
    Observable<std::vector<std::string>>   items_source;
    Observable<collection_selection_mode>  selection_mode{collection_selection_mode::single};
    Observable<int>                        selected_index{-1};
    Observable<std::vector<int>>           selected_indices;
    Observable<collection_layout>          layout{collection_layout::vertical_list};
    Observable<int>                        span{1};

    signal<int> item_tapped;

    void select(int idx);
    void deselect(int idx);
    void clear_selection();
};

} // namespace mpapp
```

The real surface — `item_template`, grouping, header/footer, empty view, scroll-to, group expand/collapse — lands with the virtualized-item-host ADR and per-platform real handlers.

## See also

- [[ListView]] — predecessor; CollectionView supersedes it in modern MAUI code.
- [[TableView]] — static-row sibling for settings-style UIs.
- [[Controls Inventory]] · [[M-04c-handler-heavy-port]]
