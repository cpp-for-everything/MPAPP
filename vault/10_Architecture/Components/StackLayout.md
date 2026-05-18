---
type: component
mauiHandler: "StackLayout"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/layouts/stacklayout"
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

# StackLayout

> [!info] Status
> **3-of-5 platforms real** — mock + WinUI 3 + GTK4 + Android (`LinearLayout` via JNI) handlers all landed and **live-verified end-to-end** in [[T-0011-app-shell-abstraction]]. Replaces the raw `muxc::StackPanel` / `GtkBox` / `LinearLayout` user code used to name; first-class cross-platform layout primitive.

## Overview

`stack_layout` arranges children in a single direction (vertical or
horizontal) with uniform spacing and configurable alignment. Mirrors
MAUI's `StackLayout`, WinUI's `StackPanel`, GTK4's `GtkBox`, AppKit's
`NSStackView`, UIKit's `UIStackView`.

Inherits the child-list API from [[Layout]] — call `add()`, `insert()`,
`remove()`, `clear()`. The four stack-specific properties drive the
per-platform native widget via `stack_layout_handler<platform::current>`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Layout\` (shared with all layouts; stack-specific arrangement in `Layouts/StackLayoutManager.cs`)
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Layout\StackLayout.cs`
- **Docs:** [Microsoft .NET MAUI — StackLayout](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/layouts/stacklayout)

## MPAPP C++ API

```cpp
namespace mpapp {

class stack_layout : public layout {
public:
    Observable<orientation> stack_orientation{orientation::vertical};
    Observable<double>      spacing{0.0};
    Observable<h_align>     horizontal_alignment{h_align::stretch};
    Observable<v_align>     vertical_alignment{v_align::stretch};

    // Inherited from `layout`:
    //   add(view&), insert(size_t, view&), remove(view&), clear()
    //   Observable<thickness> padding;
};

} // namespace mpapp
```

`orientation`, `h_align`, `v_align`, `thickness` are framework-owned
types in `<mpapp/layout_types.hpp>` (and `<mpapp/layout.hpp>` for
thickness). User code never names `muxc::Orientation`, `mux::Thickness`,
`GtkOrientation`, `UIStackViewAlignment`.

## XAML Usage

```xml
<StackLayout Orientation="Vertical"
             Spacing="12"
             Padding="24"
             HorizontalOptions="Center"
             VerticalOptions="Center">
    <Label Text="Count: 0"/>
    <Button Text="Click me"/>
</StackLayout>
```

## Platform Notes

| Platform | Native control | Notes |
|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.StackPanel` | Full property + child-list mapping. Spacing maps to `StackPanel.Spacing`. |
| Android | `LinearLayoutManager`-backed `LinearLayout` (planned) | Not yet implemented (T-0011 follow-up). |
| Linux | `GtkBox` (planned) | Not yet implemented. |
| macOS | `NSStackView` (planned) | Not yet implemented. |
| iOS | `UIStackView` (planned) | Not yet implemented. |

## Side-by-side Examples

### MPAPP (C++)

```cpp
mpapp::stack_layout sl{};
sl.stack_orientation    = mpapp::orientation::vertical;
sl.spacing              = 12.0;
sl.padding              = mpapp::thickness{24.0};
sl.horizontal_alignment = mpapp::h_align::center;
sl.add(label_widget);
sl.add(button_widget);
```

## Tests

- Mock tests: `tests/mock_handlers/stack_layout_test.cpp` (3 cases — bind / property changes / child mutations)
- Windows handler: covered by the `windows_button_spike` rewrite

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior |
|---|---|---|
| Property name `Orientation` | `Orientation` enum | `stack_orientation` (avoids shadowing `view::orientation`-like cross-cutting names) |
| `HorizontalOptions` | `LayoutOptions` (struct with `Alignment` + `Expand`) | `horizontal_alignment` (`h_align` enum); the `Expand` flag will rejoin the API in M-04 if it makes the parity cut. |

## See also

- [[Controls Inventory]]
- [[Window]] · [[Application]] · [[Page]] · [[Grid]]
- [[Handlers]]
- [[ADR-0012-application-window-handler-abstraction]]
