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
> **android-real** — Cross-platform recycler with stable-outer-container + swappable-inner-widget pattern. Three layout modes shipped (vertical_list, vertical_grid; horizontal modes degrade to vertical for v1). Multi-select event round-trip wired on all 3 platforms. **Typed_items** parallel surface lets apps supply `vector<view*>` of cells/views; handler renders each through ADR-0013 dispatch (Android swaps inner to ScrollView+LinearLayout in typed mode).
>
> Native plumbing:
> - **Windows** — outer `mux::Border` wraps `muxc::ListViewBase` (concretely `ListView` for vertical_list or `GridView` for vertical_grid). `SelectionMode` mapped to None/Single/Multiple; multi-select echoes via `SelectedItems` + `Items.IndexOf`.
> - **Linux** — outer `GtkScrolledWindow` wraps `GtkListBox` (list) or `GtkFlowBox` (grid). `GtkSelectionMode` mapped from the cross-platform enum; multi-select echoes via `gtk_list_box_get_selected_rows` / `gtk_flow_box_get_selected_children`.
> - **Android** — outer `FrameLayout` wraps `android.widget.ListView` (list) or `android.widget.GridView` (grid). Both extend `AbsListView` so `setChoiceMode` + `getCheckedItemPositions` work uniformly. `MppItemClickRouter` (kind=1) routes taps + multi-select refresh.
>
> **V1 limitations:** horizontal_list / horizontal_grid degrade to their vertical counterparts (would require ItemsPanelTemplate on Win + GtkOrientable on Linux + RecyclerView on Android). Selection / multi-select don't apply in typed_items mode (cells own their interaction). Android's layout enum is ignored in typed mode (always vertical LinearLayout). Item templates (true virtualized cell factories) are still future work — typed_items is a non-virtualizing static approach.

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
