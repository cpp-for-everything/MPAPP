---
type: component
mauiHandler: "SwitchCell"
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

# SwitchCell

> [!info] Status
> **mock** — surface at `include/mpapp/switch_cell.hpp` per [[ADR-0021-tableview-cell-types]]. Mock handler tests cover `text` + `on` mappers and verify the `toggle()` helper emits `on_changed`.

## Overview

`switch_cell` is a TableView row with a `text` label + a native toggle switch bound to `on`. Two-way; emits `on_changed` after each flip.

## MPAPP C++ API

```cpp
class switch_cell : public cell {
public:
    Observable<std::string> text;
    Observable<bool>        on{false};

    signal<bool> on_changed;

    void toggle();   // flips `on` + emits on_changed
};
```

Inherits `is_enabled` + `tapped` from [[Components/Cell]].

## See also

- [[ADR-0021-tableview-cell-types]] · [[TableView]] · [[Switch]]
