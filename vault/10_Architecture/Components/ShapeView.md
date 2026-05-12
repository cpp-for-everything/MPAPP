---
type: component
mauiHandler: "ShapeView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/shapeview"
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

# ShapeView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`ShapeView` is the host element for vector shapes (rectangle, ellipse, line, polyline, polygon, path). It owns a `shape` geometry plus paint controls: `fill`, `stroke`, `stroke_thickness`, dash pattern, line cap/join, and miter limit. The geometry stretches inside the view according to `aspect`. Under the hood every concrete `Rectangle`/`Ellipse`/`Path` derives from `Shape` and renders through `ShapeView`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\ShapeView\`
- **Control (base):** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Shapes\Shape.cs`
- **Docs:** [Microsoft .NET MAUI — Shapes](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/shapes/)

## MPAPP C++ API

```cpp
namespace mpapp {

enum class line_cap  { butt, round, square };
enum class line_join { miter, bevel, round };
enum class stretch   { none, fill, uniform, uniform_to_fill };

struct geometry; // base for rectangle/ellipse/line/path geometries

class shape_view : public control<shape_view> {
public:
    Observable<std::shared_ptr<geometry>> shape;
    Observable<stretch>                   aspect{stretch::none};

    // Paint
    Observable<brush>     fill;
    Observable<brush>     stroke;
    Observable<double>    stroke_thickness{1.0};
    Observable<std::vector<double>> stroke_dash_pattern;
    Observable<double>    stroke_dash_offset{0.0};
    Observable<line_cap>  stroke_line_cap{line_cap::butt};
    Observable<line_join> stroke_line_join{line_join::miter};
    Observable<double>    stroke_miter_limit{10.0};
};

} // namespace mpapp
```

`brush` is the gradient/solid brush variant tracked in [[Observable Properties]].

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. Most apps use derived shapes. -->
<Rectangle Fill="Tomato"
           Stroke="Black"
           StrokeThickness="2"
           WidthRequest="120" HeightRequest="80"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.Maui.Graphics.Win2D.W2DGraphicsView` | C++/WinRT | Win2D path rendering. |
| Android | `MauiShapeView` (`View`) | fbjni / JNI | Skia / canvas-drawn paths. |
| Linux | `GtkDrawingArea` driving a path renderer | GTK4 | Cairo backend. |
| macOS | `MauiShapeView` (`NSView` w/ `CAShapeLayer`) | AppKit | Layer-backed stroke + fill. |
| iOS | `MauiShapeView` (`UIView` w/ `CAShapeLayer`) | UIKit | Layer-backed stroke + fill. |

## Side-by-side Examples

### MAUI

```xml
<Ellipse Fill="DodgerBlue"
         Stroke="Navy"
         StrokeThickness="3"
         WidthRequest="100" HeightRequest="100"/>
```

### MPAPP (XAML)

```xml
<Ellipse Fill="DodgerBlue"
         Stroke="Navy"
         StrokeThickness="3"
         WidthRequest="100" HeightRequest="100"/>
```

### MPAPP (C++)

```cpp
auto e = std::make_shared<mpapp::ellipse>();
e->fill = mpapp::solid_brush(mpapp::colors::dodger_blue);
e->stroke = mpapp::solid_brush(mpapp::colors::navy);
e->stroke_thickness = 3.0;
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/shapeview/mock_test.cpp` (planned)
- Windows handler: `tests/components/shapeview/windows_test.cpp` (planned)
- Android handler: `tests/components/shapeview/android_test.cpp` (planned)
- Linux handler: `tests/components/shapeview/linux_test.cpp` (planned)
- macOS handler: `tests/components/shapeview/macos_test.cpp` (planned)
- iOS handler: `tests/components/shapeview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `Brush` hierarchy | Polymorphic class tree | `brush` variant (`solid_brush`, `linear_gradient_brush`, ...) | Static dispatch per [[Type System]] | RFC TBD |
| Geometry | Polymorphic `Geometry` | `geometry` interface with concrete types as `std::shared_ptr` | Closer to platform Skia/CoreGraphics surfaces | RFC TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[GraphicsView]]
