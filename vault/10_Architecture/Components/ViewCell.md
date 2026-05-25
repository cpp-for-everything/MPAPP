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


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_view_cell` | [`include/mpapp/internal/basic_view_cell.hpp`](../../../include/mpapp/internal/basic_view_cell.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::view_cell` | [`include/mpapp/view_cell.hpp`](../../../include/mpapp/view_cell.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/view_cell.hpp>

mpapp::view_cell w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/view_cell.hpp>
#include <mpapp/handlers/mock/view_cell_handler.hpp>

mpapp::internal::basic_view_cell w;
mpapp::view_cell_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::view_cell_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::view_cell_handler<>` and `mpapp::view_cell_handler<platform::mock>` valid spellings without naming `internal::`.

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
