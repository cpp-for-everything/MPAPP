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


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_switch_cell` | [`include/mpapp/internal/basic_switch_cell.hpp`](../../../include/mpapp/internal/basic_switch_cell.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::switch_cell` | [`include/mpapp/switch_cell.hpp`](../../../include/mpapp/switch_cell.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/switch_cell.hpp>

mpapp::switch_cell w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/switch_cell.hpp>
#include <mpapp/handlers/mock/switch_cell_handler.hpp>

mpapp::internal::basic_switch_cell w;
mpapp::switch_cell_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::switch_cell_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::switch_cell_handler<>` and `mpapp::switch_cell_handler<platform::mock>` valid spellings without naming `internal::`.

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

## Implementation

- Surface: [`include/mpapp/switch_cell.hpp`](../../../include/mpapp/switch_cell.hpp) — `text` label + `on` bool with `on_changed` signal + `toggle()` helper.
- Mock handler: [`include/mpapp/handlers/mock/switch_cell_handler.hpp`](../../../include/mpapp/handlers/mock/switch_cell_handler.hpp).
- Real handlers:
  - Windows: [`src/handlers/windows/switch_cell_handler.cpp`](../../../src/handlers/windows/switch_cell_handler.cpp) — `mux::Controls::Border` + 2-col Grid (`TextBlock` + `ToggleSwitch`).
  - Linux: [`src/handlers/linux/switch_cell_handler.cpp`](../../../src/handlers/linux/switch_cell_handler.cpp) — horizontal `GtkBox` (`GtkLabel` + `GtkSwitch`).
  - Android: [`src/handlers/android/switch_cell_handler.cpp`](../../../src/handlers/android/switch_cell_handler.cpp) — horizontal `LinearLayout` (`TextView` weight=1 + `Switch`); shares the `MppCheckedChangeListener` (kind=4) router per [[ADR-0022-android-kind-discriminated-routers]].

## See also

- [[ADR-0021-tableview-cell-types]] · [[TableView]] · [[Switch]]
