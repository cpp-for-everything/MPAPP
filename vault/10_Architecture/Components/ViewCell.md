---
type: component
mauiHandler: "ViewCell"
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

# ViewCell

> [!info] Status
> **mock** — surface at `include/mpapp/view_cell.hpp` per [[ADR-0021-tableview-cell-types]]. Mock handler tests verify `content.present` transitions.

## Overview

`view_cell` is the escape hatch in the TableView cell tree — wraps an arbitrary `view*` so apps can build custom rows the typed cells (text / entry / switch / image) don't cover.

## MPAPP C++ API

```cpp
class view_cell : public cell {
public:
    Observable<view*> content{nullptr};
};
```

Inherits `is_enabled` + `tapped` from [[Components/Cell]].

## See also

- [[ADR-0021-tableview-cell-types]] · [[TableView]] · [[ContentView]]
