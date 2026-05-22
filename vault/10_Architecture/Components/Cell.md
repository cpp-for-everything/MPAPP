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

## MPAPP C++ API

```cpp
class cell : public view {
public:
    Observable<bool> is_enabled{true};
    signal<>         tapped;
};
```

## See also

- [[ADR-0021-tableview-cell-types]] — full type hierarchy.
- [[TextCell]] · [[EntryCell]] · [[SwitchCell]] · [[ViewCell]] · [[ImageCell]]
- [[TableView]]
