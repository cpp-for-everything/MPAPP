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


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_collection_view` | [`include/mpapp/internal/basic_collection_view.hpp`](../../../include/mpapp/internal/basic_collection_view.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::collection_view` | [`include/mpapp/collection_view.hpp`](../../../include/mpapp/collection_view.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_cv_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/collection_view.hpp>

mpapp::collection_view w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/collection_view.hpp>
#include <mpapp/handlers/mock/collection_view_handler.hpp>

mpapp::internal::basic_collection_view w;
mpapp::collection_view_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::collection_view_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::collection_view_handler<>` and `mpapp::collection_view_handler<platform::mock>` valid spellings without naming `internal::`.

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

## Implementation

- Surface: [`include/mpapp/collection_view.hpp`](../../../include/mpapp/collection_view.hpp) — observable members + the `collection_layout` / `collection_selection_mode` enums.
- Mock handler: [`include/mpapp/handlers/mock/collection_view_handler.hpp`](../../../include/mpapp/handlers/mock/collection_view_handler.hpp) — records `map_layout` / `map_items_source` / selection set changes for the unit-test surface.
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/collection_view_handler.hpp`](../../../include/mpapp/handlers/windows/collection_view_handler.hpp) + [`src/handlers/windows/collection_view_handler.cpp`](../../../src/handlers/windows/collection_view_handler.cpp) — outer `mux::Border` / inner `ListView`-or-`GridView` swap with `ItemsPanelTemplate` orientation flip.
  - Linux: [`src/handlers/linux/collection_view_handler.cpp`](../../../src/handlers/linux/collection_view_handler.cpp) — outer `GtkScrolledWindow` / inner `GtkListBox` / `GtkBox(HORIZONTAL)` / `GtkFlowBox` swap.
  - Android: [`src/handlers/android/collection_view_handler.cpp`](../../../src/handlers/android/collection_view_handler.cpp) + Java glue [`examples/android_hello/app/src/main/java/io/mpapp/MppCollectionAdapter.java`](../../../examples/android_hello/app/src/main/java/io/mpapp/MppCollectionAdapter.java) + [`MppItemClickRouter.java`](../../../examples/android_hello/app/src/main/java/io/mpapp/MppItemClickRouter.java) — `RecyclerView` + `LinearLayoutManager` / `GridLayoutManager` swap (post-T-0028 migration from the legacy `ListView`/`GridView`).
- Tests: [`tests/mock_handlers/collection_view_test.cpp`](../../../tests/mock_handlers/collection_view_test.cpp) — layout-enum cycles, items_source + selection survival across layout changes.
- Demo apps: [`examples/windows_collectionview_layout_demo/`](../../../examples/windows_collectionview_layout_demo/) + [`examples/gtk4_collectionview_layout_demo/`](../../../examples/gtk4_collectionview_layout_demo/) — four-layout matrix in one window (T-0028 closure evidence in [[_Archive/T-0028-collectionview-orientation]]).

## See also

- [[ListView]] — predecessor; CollectionView supersedes it in modern MAUI code.
- [[TableView]] — static-row sibling for settings-style UIs.
- [[Controls Inventory]] · [[M-04c-handler-heavy-port]]
