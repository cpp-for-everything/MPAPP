---
type: adr
id: ADR-0020
title: "Virtualized item host — wrap platform recyclers"
status: accepted
decisionDate: 2026-05-21
deciders:
  - alex
supersedes: ""
supersededBy: ""
area: handlers
tags:
  - type/adr
  - status/accepted
  - area/handlers
---

# ADR-0020 — Virtualized item host: wrap platform recyclers

> [!info] Status
> **proposed** — unblocks ListView / CollectionView / TableView real handlers.

## Context

The three list-family widgets (`list_view`, `collection_view`, `table_view`) all need item virtualization: a scrollable container that materializes child views only for visible (and near-visible) rows. Each platform ships a battle-tested recycler:

- Windows: `mux::Controls::ItemsRepeater` + `ScrollViewer`, or `mux::Controls::ListView` for selection-heavy scenarios.
- Linux: `GtkListView` (4.10+) backed by a `GListModel` + `GtkListItemFactory`.
- Android: `androidx.recyclerview.widget.RecyclerView` with `LinearLayoutManager` / `GridLayoutManager`.
- macOS: `NSTableView` / `NSCollectionView`.
- iOS: `UICollectionView`.

Each is mature, accessibility-aware, performance-tuned, and integrated with the native scroll experience. Rebuilding this layer cross-platform is months of work for marginal gain.

## Decision

We will **wrap the platform recycler** on each platform. The MPAPP-side `list_view_handler<P>` / `collection_view_handler<P>` / `table_view_handler<P>` translates the cross-platform `items_source: Observable<vector<T>>` into the platform recycler's data-source contract, and the `item_template` into the recycler's factory.

```cpp
// Conceptual shape (per-platform implementation varies):
class collection_view_handler<platform::windows> {
public:
    void map_items_source(collection_view& cv) {
        cv.items_source.changed.subscribe(slot_, [&](const auto& v){
            data_source_.replace(v);   // ItemsRepeater rebinds
        });
    }
    void map_item_template(collection_view& cv) {
        repeater_.ItemTemplate(make_template_selector(cv));
    }
};
```

**Item templates** are a follow-up question (cell-subclass tree for TableView is ADR-0021; CollectionView/ListView item_template is item-template ADR TBD). For v1 we ship the surface shipped at mock — `vector<std::string>` items rendered as text. Template-instantiating real handlers land after the item_template ADR.

**Cross-platform differences** that get documented as "Known Differences" on each component:

- Selection-mode behaviors differ in edge cases (Android multi-select on long-press by default vs. Windows always-tap).
- Virtualization thresholds differ (Android RecyclerView pre-fetches 5 items by default; ItemsRepeater pre-fetches 1).
- Scroll inertia / overscroll style is native to the platform — not abstracted.

These divergences are intentional: each platform's user expects their platform's behavior.

## Consequences

### Positive

- Best-in-class virtualization, accessibility, and scroll feel on every platform for free.
- We don't maintain a layout / measurement / recycling engine across 5 platforms.
- New items_source shapes (grouping, headers, footers) compose naturally with the platform's existing facilities.

### Negative

- Edge-case behavior diverges per platform (documented as Known Differences).
- The "item template" abstraction must reconcile with each platform's template/factory model. Some loss of fidelity is inevitable.
- Apps targeting deep customization (e.g. snap-to-item scroll on Android, sticky headers) need platform-specific code paths today; we add a uniform API for those in follow-up ADRs as users hit the cases.

### Neutral

- Performance is bounded by each platform's recycler — generally excellent. No animation pipeline of our own.

## Alternatives Considered

- **Roll our own portable recycler** — rejected; reimplementing virtualization, hit-testing, and scroll inertia cross-platform is a months-of-work distraction.
- **Hybrid** — rejected; complexity penalty of two recycler paths exceeds the value.
- **No virtualization** — rejected for prod; fine for the mock; documented as "real handlers wrap platform recyclers" so apps know what they're getting.

## Implementation Notes

The three list-family widgets, each wrapping its platform's recycler:

- **CollectionView**
  Surface: [`include/mpapp/collection_view.hpp`](../../include/mpapp/collection_view.hpp).
  Real handlers wrap `mux::Controls::ListView`/`GridView` on Windows ([`src/handlers/windows/collection_view_handler.cpp`](../../src/handlers/windows/collection_view_handler.cpp)), `GtkListBox`/`GtkBox`/`GtkFlowBox` on Linux ([`src/handlers/linux/collection_view_handler.cpp`](../../src/handlers/linux/collection_view_handler.cpp)), `RecyclerView` + `LinearLayoutManager`/`GridLayoutManager` on Android ([`src/handlers/android/collection_view_handler.cpp`](../../src/handlers/android/collection_view_handler.cpp) — the T-0028 migration off the legacy `ListView`/`GridView`).
- **ListView**
  Surface: [`include/mpapp/list_view.hpp`](../../include/mpapp/list_view.hpp).
  Real handlers: [`src/handlers/windows/list_view_handler.cpp`](../../src/handlers/windows/list_view_handler.cpp) (`mux::Controls::ListView`) + [`src/handlers/linux/list_view_handler.cpp`](../../src/handlers/linux/list_view_handler.cpp) (`GtkListBox` in `GtkScrolledWindow`) + [`src/handlers/android/list_view_handler.cpp`](../../src/handlers/android/list_view_handler.cpp) (`android.widget.ListView` + `ArrayAdapter`).
- **TableView**
  Surface: [`include/mpapp/table_view.hpp`](../../include/mpapp/table_view.hpp).
  Real handlers + per-platform recyclers similar to ListView; flat and typed-cell-tree section surfaces both dispatch via [[ADR-0013-data-driven-widget-dispatch]] + [[ADR-0021-tableview-cell-types]].
- Tests: [`tests/mock_handlers/collection_view_test.cpp`](../../tests/mock_handlers/collection_view_test.cpp), [`list_view_test.cpp`](../../tests/mock_handlers/list_view_test.cpp), [`table_view_test.cpp`](../../tests/mock_handlers/table_view_test.cpp) cover items-source binding + selection round-trip + layout-mode survival.

## References

- [[ADR-0006-interop-parity]] — observable behavior must be uniform; native scroll/recycler nuances are documented divergences.
- [[Components/ListView]] · [[Components/CollectionView]] · [[Components/TableView]]
- [[40_Roadmap/M-04c-handler-heavy-port]]
