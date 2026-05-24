---
type: component
mauiHandler: "ViewCell"
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

# ViewCell

> [!info] Status
> **android-real** — Win `mux::Controls::Border` with `Child` swap + Linux `GtkBox` single-slot + Android `FrameLayout` with `removeAllViews`/`addView`. Content resolved through ADR-0013 widget_dispatch so any registered view can nest. Native row padding (12/6 px on Win/Linux, 24/12 dp on Android) matches text_cell aesthetic.

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

## Implementation

- Surface: [`include/mpapp/view_cell.hpp`](../../../include/mpapp/view_cell.hpp) — `Observable<view*> content{nullptr}` — non-owning.
- Mock handler: [`include/mpapp/handlers/mock/view_cell_handler.hpp`](../../../include/mpapp/handlers/mock/view_cell_handler.hpp).
- Real handlers:
  - Windows: [`src/handlers/windows/view_cell_handler.cpp`](../../../src/handlers/windows/view_cell_handler.cpp) — `mux::Controls::Border` with `Child` swap.
  - Linux: [`src/handlers/linux/view_cell_handler.cpp`](../../../src/handlers/linux/view_cell_handler.cpp) — single-slot `GtkBox`.
  - Android: [`src/handlers/android/view_cell_handler.cpp`](../../../src/handlers/android/view_cell_handler.cpp) — `FrameLayout` with `removeAllViews` / `addView`.
- Content resolved through [[ADR-0013-data-driven-widget-dispatch]] so any registered view type can nest.

## See also

- [[ADR-0021-tableview-cell-types]] · [[TableView]] · [[ContentView]]
