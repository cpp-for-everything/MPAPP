---
type: component
mauiHandler: "Layout"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/layout"
mpappStatus: not-started
platformWindows: false
platformAndroid: false
platformLinux: false
platformMacos: false
platformIos: false
tags:
  - type/component
  - status/not-started
---

# Layout

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Layout` is the abstract base of every multi-child container in MAUI — `Grid`, `StackLayout`, `HorizontalStackLayout`, `VerticalStackLayout`, `AbsoluteLayout`, and `FlexLayout` all derive from it. It owns a flat `IList<IView>` of child views, surfaces `Padding`, `IsClippedToBounds`, `CascadeInputTransparent`, and `SafeAreaEdges`, and delegates measurement and arrangement to an `ILayoutManager`. The cross-platform `LayoutHandler` wires add / insert / remove / clear / update / z-index commands down to a platform-specific container (a `MauiPanel` on Windows, a `LayoutViewGroup` on Android, a `LayoutView` on iOS/macOS). In MPAPP, `layout` plays the same role: a `view` that owns a children collection and a pluggable layout-manager strategy.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Layout\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Layout\`
- **Docs:** [Microsoft .NET MAUI — Layout](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/layout)

`LayoutHandler.Mapper` adds `Background` and `ClipsToBounds` to the inherited `ViewMapper`. The `CommandMapper` adds `Add`, `Remove`, `Clear`, `Insert`, `Update`, and `UpdateZIndex`. Measurement / arrangement is delegated to `ILayoutManager` returned from the control's `CreateLayoutManager()` override (per-subclass: `GridLayoutManager`, `StackLayoutManager`, `AbsoluteLayoutManager`, `FlexLayoutManager`).

## MPAPP C++ API

```cpp
namespace mpapp {

// Abstract container. Concrete subclasses (grid, stack_layout, ...) provide
// the layout strategy via create_layout_manager().
class layout : public view {
public:
    Observable<thickness>            padding;
    Observable<bool>                 is_clipped_to_bounds;
    Observable<bool>                 cascade_input_transparent;
    // (safe_area_edges, background, etc. are inherited from view.)

    // Children - observable list. Mutations trigger handler add/remove commands.
    ObservableList<std::shared_ptr<view>> children;

    // Cross-platform measure / arrange. Subclasses implement via the manager.
    Size measure(double width_constraint, double height_constraint);
    Size arrange(const Rect& bounds);

protected:
    virtual std::unique_ptr<layout_manager> create_layout_manager() = 0;
};

} // namespace mpapp
```

## XAML Usage

`Layout` is abstract — XAML always names a concrete subclass:

```xml
<VerticalStackLayout Padding="12" Spacing="8">
    <Label Text="One" />
    <Label Text="Two" />
</VerticalStackLayout>

<Grid RowDefinitions="Auto,*" ColumnDefinitions="*,*" Padding="8">
    <Label Grid.Row="0" Grid.Column="0" Text="Top-left" />
    <Label Grid.Row="1" Grid.ColumnSpan="2" Text="Bottom" />
</Grid>
```

## Platform Notes

| Platform | Native control                                              | Header / source            | Notes |
|----------|-------------------------------------------------------------|----------------------------|-------|
| Windows  | `Microsoft.UI.Xaml.Controls.Panel` (custom `MauiPanel` / `LayoutPanel`) | C++/WinRT       | MAUI uses a `Panel` subclass so cross-platform layout owns measure + arrange. |
| Android  | `android.view.ViewGroup` (custom `LayoutViewGroup`)         | fbjni / JNI                | Subclasses `PlatformViewGroup`; implements `ICrossPlatformLayoutBacking`. |
| Linux    | `GtkFixed` / `GtkBox` (depends on subclass)                 | gtk4-rs                    | MPAPP layer owns measure; GTK is used in fixed/raw-positioning mode. |
| macOS    | `NSView` (custom `LayoutView`)                              | AppKit / Catalyst          | Catalyst path uses `UIView`. |
| iOS      | `UIKit.UIView` (custom `LayoutView`)                        | UIKit                      | Subclass of `MauiView`. |

## Side-by-side Examples

### MAUI

```xml
<VerticalStackLayout Padding="16" Spacing="8">
    <Label Text="Title" />
    <Button Text="OK" />
</VerticalStackLayout>
```

### MPAPP (XAML)

```xml
<VerticalStackLayout Padding="16" Spacing="8">
    <Label Text="Title" />
    <Button Text="OK" />
</VerticalStackLayout>
```

### MPAPP (C++)

```cpp
auto root  = mpapp::make<mpapp::vertical_stack_layout>();
root->padding = mpapp::thickness{16};
root->spacing = 8;
root->children.add(mpapp::make<mpapp::label>("Title"));
root->children.add(mpapp::make<mpapp::button>("OK"));
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/layout/mock_test.cpp` (planned)
- Windows handler: `tests/components/layout/windows_test.cpp` (planned)
- Android handler: `tests/components/layout/android_test.cpp` (planned)
- Linux handler: `tests/components/layout/linux_test.cpp` (planned)
- macOS handler: `tests/components/layout/macos_test.cpp` (planned)
- iOS handler: `tests/components/layout/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[View]]
- [[BindableLayout]]
