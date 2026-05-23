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
> **android-real** — Cross-platform recycler with stable-outer-container + swappable-inner-widget pattern. **All four layout modes wired on all three platforms** (T-0028): `vertical_list`, `horizontal_list`, `vertical_grid`, `horizontal_grid`. Multi-select event round-trip wired on all 3 platforms. Three item-source surfaces: **items_source** (flat strings, virtualized), **typed_items** (vector<view*> non-owning), and **item_template** (factory `function<unique_ptr<view>(int)>` — the collection_view owns the materialized cells). Handlers render each typed entry through ADR-0013 dispatch.
>
> Native plumbing:
> - **Windows** — outer `mux::Border` wraps `muxc::ListViewBase` (concretely `ListView` for *_list or `GridView` for *_grid). The matching `ItemsPanelTemplate` (built from a small XAML literal via `XamlReader::Load`) sets `ItemsStackPanel`/`ItemsWrapGrid` orientation per layout. `ScrollViewer.{Horizontal,Vertical}ScrollMode` attached props flip per primary axis. `SelectionMode` mapped to None/Single/Multiple; multi-select echoes via `SelectedItems` + `Items.IndexOf`.
> - **Linux** — outer `GtkScrolledWindow` wraps one of `GtkListBox` (vertical_list), `GtkBox(HORIZONTAL)` (horizontal_list), or `GtkFlowBox` with per-axis orientation (`HORIZONTAL` for vertical_grid, `VERTICAL` for horizontal_grid). The scrolled window's h/v scrollbar policy flips per primary axis. `GtkSelectionMode` mapped from the cross-platform enum on list/flow widgets; the GtkBox path tracks `selected_index` via per-child `GtkGestureClick` controllers (no native multi-select visual highlight in v1).
> - **Android** — outer `FrameLayout` wraps `androidx.recyclerview.widget.RecyclerView` (T-0028 migration from the legacy `ListView`/`GridView`). The active LayoutManager swaps per layout: `LinearLayoutManager(VERTICAL|HORIZONTAL)` for list, `GridLayoutManager(span, VERTICAL|HORIZONTAL)` for grid. `MppCollectionAdapter` owns the canonical selection set and pushes multi-select changes back via `MppItemClickRouter.nativeDispatchCheckedSet`. `MppItemClickRouter` (kind=1) still routes single-tap events.
>
> Render precedence in each handler's `rebuild_active()`: typed_items non-empty → typed render; otherwise materialized_count > 0 (item_template) → typed render of materialized_views; otherwise flat items_source render.
>
> **V1 limitations:** Linux `horizontal_list` has no native selection-state highlight (`GtkBox` design choice — `selected_index` still tracks taps correctly). Android `GridLayoutManager` has no `AUTO_FIT` equivalent — `span=1` is promoted to 2 columns for grid layouts so default rendering matches the previous `GridView.AUTO_FIT` behavior. Selection / multi-select don't apply in typed_items / item_template modes (cells own their interaction). item_template materializes the full items_source eagerly (non-virtualizing) — true virtualized cell factories on Win + Linux require ItemsRepeater (Win) and an equivalent on Linux and stay future work.

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
