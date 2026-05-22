---
type: component
mauiHandler: "SwitchCell"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/cells"
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

# SwitchCell

> [!info] Status
> **android-real** — Win `mux::Controls::Border` wrapping a 2-column Grid (TextBlock + ToggleSwitch) + Linux horizontal `GtkBox` (GtkLabel + GtkSwitch) + Android horizontal `LinearLayout` (TextView weight=1 + Switch). Two-way `on` binding: user flips on the native widget echo back into the Observable (suppress-echo guard prevents reentry). Android route reuses the shared `MppCheckedChangeListener` with `kind=4`.

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
