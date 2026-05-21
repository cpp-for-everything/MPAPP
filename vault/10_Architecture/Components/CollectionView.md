---
type: component
mauiHandler: "CollectionView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/collectionview"
mpappStatus: mock
platformWindows: false
platformAndroid: false
platformLinux: false
platformMacos: false
platformIos: false
tags:
  - type/component
  - status/mock
---

# CollectionView

> [!info] Status
> **mock** — surface lives at `include/mpapp/collection_view.hpp`. Catch2 mock-handler tests cover items_source binding, selection_mode none/single/multiple, select/deselect/clear_selection helpers, and the selection observables (selected_index + selected_indices). Real virtualization is gated on the **virtualized-item-host ADR** (TBD); real per-platform handlers land under [[M-04c-handler-heavy-port|M-04c]].

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
