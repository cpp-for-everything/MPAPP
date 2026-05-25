---
type: component
mauiHandler: "Cell"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/cells"
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

# Cell

> [!info] Terminal status (abstract base)
> **mock** is the **terminal state** for `cell`. It is the abstract base every TableView cell subclass derives from; the typed subclasses (text_cell, entry_cell, switch_cell, view_cell, image_cell) own the real handlers. The base carries only the cross-cutting `is_enabled` toggle and a `tapped` signal.



## Wrapper + Surface

> [!info] Abstract base class
> `mpapp::cell` is a CRTP / abstract base inherited by concrete components — it is not a leaf component itself and does not follow the [[ADR-0024-wrapper-component-pattern]] wrapper / surface split.
>
> Concrete components that inherit `cell` each have their own `mpapp::internal::basic_<...>` surface and `mpapp::<...>` wrapper; this base class participates in the chain as the inheritance root.

## MPAPP C++ API

```cpp
class cell : public view {
public:
    Observable<bool> is_enabled{true};
    signal<>         tapped;
};
```

## Implementation

- Surface: [`include/mpapp/cell.hpp`](../../../include/mpapp/cell.hpp) — abstract base, no handler set (typed subclasses own theirs).
- No mock handler or per-platform handler at this level — every concrete cell (text / entry / switch / view / image) ships its own. See the per-subclass docs linked below.
- Type hierarchy enforced by [[ADR-0021-tableview-cell-types]]; the table_view's `cell_at(...)` dispatch knows to look up the concrete handler via [[ADR-0013-data-driven-widget-dispatch]].

## See also

- [[ADR-0021-tableview-cell-types]] — full type hierarchy.
- [[TextCell]] · [[EntryCell]] · [[SwitchCell]] · [[ViewCell]] · [[ImageCell]]
- [[TableView]]
