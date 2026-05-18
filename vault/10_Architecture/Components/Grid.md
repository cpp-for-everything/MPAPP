---
type: component
mauiHandler: "Grid"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/layouts/grid"
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

# Grid

> [!info] Status
> **mock** — public surface + mock handler landed in [[T-0011-app-shell-abstraction]]. The mock-surface API ships row_count / column_count / row_spacing / column_spacing — enough to validate the handler-binding contract and start the WinUI 3 real handler. Full track definitions (star sizing, min/max constraints, per-child `(row, column)` placement) are M-04 work.

## Overview

`grid_layout` is a 2D layout container with row + column tracks and
per-child cell placement. Mirrors MAUI's `Grid`, WinUI's `Grid`,
GTK4's `GtkGrid`, AppKit's `NSGridView`, UIKit's nested-StackView
composition.

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

## Tests

- Mock tests: `tests/mock_handlers/grid_layout_test.cpp` (2 cases — bind / property changes)

## See also

- [[Controls Inventory]]
- [[StackLayout]] · [[Layout]]
- [[ADR-0012-application-window-handler-abstraction]]
