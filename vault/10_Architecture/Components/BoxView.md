---
type: component
mauiHandler: "BoxView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/boxview"
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

# BoxView

> [!info] Status
> **3-of-5 platforms real** — `fill` + `corners` surface real on WinUI 3 (`mux::Controls::Border` with `Background = SolidColorBrush` and `CornerRadius` per corner; default 40×40 dip from `Width`/`Height`), GTK4 (`GtkBox` with a per-handler `GtkCssProvider` carrying inline `background-color: rgba(...)` + `border-radius: tl tr br bl`, attached to the default display so the unique CSS class on each instance picks it up without using the deprecated style-context APIs), and Android (`android.view.View` with a `GradientDrawable` background rebuilt on every fill or corner change — `setColor` for ARGB, `setCornerRadii(float[8])` for per-corner radii). Android spike's box's corner radius tracks the slider value (1.0 → 4px, 5.0 → 20px) — proves the `corners` observable propagates through to the Drawable at runtime. macOS / iOS handlers planned in M-06.

> [!info] Original status
> **mock** — cross-platform header at `include/mpapp/box_view.hpp`; mock handler records `fill` and `corners` mappers. See [[Controls Inventory]].

## Overview

`BoxView` is the simplest visible primitive in MAUI — a `View` that draws a solid-colored, optionally-rounded rectangle. It has no content and no children; it is intended for separators, dividers, decorative blocks, color swatches, and prototyping placeholders. `BoxView` derives from `View` and implements `IShape`/`IShapeView`, so under the hood it is rendered by the same `ShapeViewHandler` as `Rectangle` and `Ellipse`. Its default measured size is 40 × 40. The control predates [[Border]] and is preserved for compatibility and its zero-overhead solid-color use case.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\ShapeView\` (BoxView is rendered as a Shape; `BoxViewHandler : ShapeViewHandler` lives in `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Handlers\Shapes\BoxView\`)
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\BoxView\BoxView.cs`
- **Docs:** [Microsoft .NET MAUI — BoxView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/boxview)

The control exposes two bindable properties on top of `View`: `Color` (the fill — distinct from `BackgroundColor`) and `CornerRadius` (a `CornerRadius` with independent top-left / top-right / bottom-left / bottom-right values). The shape's `PathForBounds` builds a rounded rectangle from those four corner values.

## MPAPP C++ API

```cpp
namespace mpapp {

// Solid-colored rectangle primitive. Use for dividers, swatches, placeholders.
class box_view : public view {
public:
    Observable<color>            color;          // fill color
    Observable<corner_radius>    corner_radius;  // top-left, top-right, bottom-left, bottom-right
};

} // namespace mpapp
```

Default desired size mirrors MAUI: 40 × 40 device-independent units when the parent imposes no constraint.

## XAML Usage

```xml
<!-- Horizontal separator -->
<BoxView Color="LightGray" HeightRequest="1" HorizontalOptions="Fill" />

<!-- Rounded swatch -->
<BoxView Color="DodgerBlue"
         CornerRadius="8"
         WidthRequest="64"
         HeightRequest="64" />

<!-- Asymmetric corners -->
<BoxView Color="Coral" CornerRadius="0,8,0,8" WidthRequest="80" HeightRequest="40" />
```

## Platform Notes

| Platform | Native control                                              | Header / source            | Notes |
|----------|-------------------------------------------------------------|----------------------------|-------|
| Windows  | `Microsoft.Maui.Graphics.Win2D.W2DGraphicsView`             | C++/WinRT + Win2D          | Rendered via the shared graphics canvas, not as a `Microsoft.UI.Xaml` shape. |
| Android  | `android.view.View` (custom `MauiShapeView`)                | fbjni / JNI                | Draws into the canvas in `onDraw`. |
| Linux    | `GtkDrawingArea`                                            | gtk4-rs                    | Filled via cairo / `GtkSnapshot`. |
| macOS    | `NSView` (custom `MauiShapeView` via Catalyst-shared code)  | AppKit / Catalyst          | Catalyst reuses iOS path. |
| iOS      | `UIKit.UIView` (custom `MauiShapeView`)                     | UIKit                      | Drawn via `CAShapeLayer` / `CGContext`. |

## Side-by-side Examples

### MAUI

```xml
<BoxView Color="Red" HeightRequest="2" />
```

### MPAPP (XAML)

```xml
<BoxView Color="Red" HeightRequest="2" />
```

### MPAPP (C++)

```cpp
auto divider = mpapp::make<mpapp::box_view>();
divider->color          = mpapp::colors::red;
divider->height_request = 2.0;
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/boxview/mock_test.cpp` (planned)
- Windows handler: `tests/components/boxview/windows_test.cpp` (planned)
- Android handler: `tests/components/boxview/android_test.cpp` (planned)
- Linux handler: `tests/components/boxview/linux_test.cpp` (planned)
- macOS handler: `tests/components/boxview/macos_test.cpp` (planned)
- iOS handler: `tests/components/boxview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Mock implementation

The P2 mock surface (ADR-0008) lands in this repository:

- **Cross-platform header:** `include/mpapp/box_view.hpp` — `mpapp::box_view : view` with `Observable<color> fill` and `Observable<corner_radius> corners`. The lightweight `color` (rgba) and `corner_radius` (per-corner) types are defined in this header.
- **Mock handler:** `include/mpapp/handlers/mock/box_view_handler.hpp` — `box_view_handler<platform::mock>` records both mappers.
- **Mock tests:** `tests/mock_handlers/box_view_test.cpp`.

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[View]]
- [[Border]]
