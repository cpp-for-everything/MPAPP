---
type: component
mauiHandler: "Grid"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/layouts/grid"
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

# Grid

> [!info] Status
> **android-real** — `row_definitions` / `column_definitions` of `track_def` (Auto / Star(w) / Fixed(px)) per [[ADR-0017-grid-track-definitions]]. Per-child placement via attached properties (`grid.set_row(view, r)` etc.) stored in a side map keyed on `view*`. Windows wraps `mux::Controls::Grid` with `RowDefinitions` / `ColumnDefinitions` populated via `GridLength{value, GridUnitType::{Pixel,Star,Auto}}`; Linux wraps `GtkGrid` and bridges Star tracks to `hexpand`/`vexpand` on the attached child (GtkGrid has no explicit Star sizing); Android wraps `android.widget.GridLayout` with `GridLayout.LayoutParams{spec(row, rowSpan), spec(col, colSpan)}` built per attach. **V1 limitations**: Linux Star sizing is approximate (no equivalent of explicit weights); Android grid spacing per-row/col is not exposed (use per-child margins); macOS/iOS pending Apple host.

## Overview

`grid_layout` is a 2D layout container with row + column tracks and
per-child cell placement. Mirrors MAUI's `Grid`, WinUI's `Grid`,
GTK4's `GtkGrid`, AppKit's `NSGridView`, UIKit's nested-StackView
composition.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_grid_layout` | [`include/mpapp/internal/basic_grid_layout.hpp`](../../../include/mpapp/internal/basic_grid_layout.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::grid_layout` | [`include/mpapp/grid_layout.hpp`](../../../include/mpapp/grid_layout.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/grid_layout.hpp>

mpapp::grid_layout w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/grid_layout.hpp>
#include <mpapp/handlers/mock/grid_layout_handler.hpp>

mpapp::internal::basic_grid_layout w;
mpapp::grid_layout_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::grid_layout_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::grid_layout_handler<>` and `mpapp::grid_layout_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Layout\`
- **Layout manager:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Layouts\GridLayoutManager.cs`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Layout\Grid.cs`
- **Docs:** [Microsoft .NET MAUI — Grid](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/layouts/grid)

## MPAPP C++ API

Initial mock surface. The full track-definition / per-child placement
API lands in M-04.

```cpp
namespace mpapp {

class grid_layout : public layout {
public:
    Observable<int>    row_count{1};
    Observable<int>    column_count{1};
    Observable<double> row_spacing{0.0};
    Observable<double> column_spacing{0.0};

    // M-04: row_definitions, column_definitions (star / auto / abs sizing),
    //       per-child grid.row / grid.column / grid.row_span / grid.column_span
    //       attached properties.
};

} // namespace mpapp
```

## XAML Usage (M-04 target)

```xml
<Grid RowDefinitions="*,Auto" ColumnDefinitions="*,*">
    <Label Text="Cell 0,0" Grid.Row="0" Grid.Column="0"/>
    <Button Text="Cell 1,1" Grid.Row="1" Grid.Column="1"/>
</Grid>
```

## Platform Notes

| Platform | Native control | Notes |
|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.Grid` (planned) | Real handler lands in M-04. |
| Android | `androidx.gridlayout.widget.GridLayout` (planned) | M-04. |
| Linux | `GtkGrid` (planned) | M-04. |
| macOS | `NSGridView` (planned) | M-04. |
| iOS | Nested `UIStackView`s with `UILayoutGuide` (planned) | M-04. |

## Implementation

- Surface: [`include/mpapp/grid_layout.hpp`](../../../include/mpapp/grid_layout.hpp) — `track_def` value type (Auto / Star(w) / Fixed(px)), row/column definitions, attached-property helpers `set_row(view, r)` etc.
- Mock handler: [`include/mpapp/handlers/mock/grid_layout_handler.hpp`](../../../include/mpapp/handlers/mock/grid_layout_handler.hpp).
- Real handlers:
  - Windows: [`src/handlers/windows/grid_layout_handler.cpp`](../../../src/handlers/windows/grid_layout_handler.cpp) — `mux::Controls::Grid` with `RowDefinitions` / `ColumnDefinitions` populated via `GridLength{value, GridUnitType::{Pixel,Star,Auto}}`.
  - Linux: [`src/handlers/linux/grid_layout_handler.cpp`](../../../src/handlers/linux/grid_layout_handler.cpp) — `GtkGrid`; Star tracks bridged to `hexpand` / `vexpand` on the attached child (GtkGrid has no explicit Star sizing).
  - Android: [`src/handlers/android/grid_layout_handler.cpp`](../../../src/handlers/android/grid_layout_handler.cpp) — `android.widget.GridLayout` with `LayoutParams{spec(row, rowSpan), spec(col, colSpan)}`.
- Track-def parser: `track_def::parse(...)` inside [`include/mpapp/grid_layout.hpp`](../../../include/mpapp/grid_layout.hpp) — parses the `RowDefinitions="*,Auto,200"` MAUI-string DSL per [[ADR-0017-grid-track-definitions]].
- Tests: [`tests/mock_handlers/grid_layout_test.cpp`](../../../tests/mock_handlers/grid_layout_test.cpp) (bind + property changes) + [`tests/mock_handlers/grid_track_parser_test.cpp`](../../../tests/mock_handlers/grid_track_parser_test.cpp) (parser unit tests).

## See also

- [[Controls Inventory]]
- [[StackLayout]] · [[Layout]]
- [[ADR-0017-grid-track-definitions]] — track-def value type design.
- [[ADR-0012-application-window-handler-abstraction]]
