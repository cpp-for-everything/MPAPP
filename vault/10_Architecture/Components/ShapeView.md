---
type: component
mauiHandler: "ShapeView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/shapeview"
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

# ShapeView

> [!info] Status
> **android-real (v2 — all 3 platforms migrated via [[T-0031]] phases 1+2)** — Every platform now renders through the shared `detail::graphics::render_shape_view` helper feeding the ADR-0015 canvas facade.
>
> - **Linux** (`GtkDrawingArea`) — GTK draw callback wraps the facade's BGRA32 pixels via `cairo_image_surface_create_for_data` and blits through GTK's `cairo_t*`. No channel swap.
> - **Windows** (`muxc::Image` + `WriteableBitmap`) — facade pixels copied into the WriteableBitmap's `PixelBuffer` (BGRA8 premultiplied, same byte order). `SizeChanged` triggers re-render at the new layout-assigned dimensions.
> - **Android** (`android.widget.ImageView` + `android.graphics.Bitmap` ARGB_8888) — facade pixels copied with a per-pixel B↔R swap inside `AndroidBitmap_lockPixels` / `unlockPixels`. Layout changes tracked via `MppShapeViewLayoutListener` (a 5-line `View.OnLayoutChangeListener` that fires a JNI callback into the C++ handler).
>
> `path::from_svg` parses real SVG path data (M/L/Q/C/Z), falling back to the bounding rectangle if the data string is empty or invalid — strict improvement over the legacy bounding-rect-always behavior the per-platform v1 handlers had for polygon/path kinds.
>
> The previous per-platform implementations (`muxs::Shape` primitives on Windows, direct cairo calls on Linux, `MppShapeView` custom Java view on Android) are gone — `MppShapeView.java` deleted, ~300 LOC of per-platform paint dispatch removed. Output is now byte-for-byte identical across platforms (modulo per-platform pixel-blit conversions), and swapping the canvas backend (Cairo today, Skia once installed via `-DMPAPP_GRAPHICS_BACKEND=skia`) swaps the rendering on every platform at once.

## Overview

`ShapeView` is the host element for vector shapes (rectangle, ellipse, line, polyline, polygon, path). It owns a `shape` geometry plus paint controls: `fill`, `stroke`, `stroke_thickness`, dash pattern, line cap/join, and miter limit. The geometry stretches inside the view according to `aspect`. Under the hood every concrete `Rectangle`/`Ellipse`/`Path` derives from `Shape` and renders through `ShapeView`.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_shape_view` | [`include/mpapp/internal/basic_shape_view.hpp`](../../../include/mpapp/internal/basic_shape_view.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::shape_view` | [`include/mpapp/shape_view.hpp`](../../../include/mpapp/shape_view.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_sv_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/shape_view.hpp>

mpapp::shape_view w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/shape_view.hpp>
#include <mpapp/handlers/mock/shape_view_handler.hpp>

mpapp::internal::basic_shape_view w;
mpapp::shape_view_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::shape_view_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::shape_view_handler<>` and `mpapp::shape_view_handler<platform::mock>` valid spellings without naming `internal::`.

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

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `Brush` hierarchy | Polymorphic class tree | `brush` variant (`solid_brush`, `linear_gradient_brush`, ...) | Static dispatch per [[Type System]] | RFC TBD |
| Geometry | Polymorphic `Geometry` | `geometry` interface with concrete types as `std::shared_ptr` | Closer to platform Skia/CoreGraphics surfaces | RFC TBD |

## Implementation

- Surface: [`include/mpapp/shape_view.hpp`](../../../include/mpapp/shape_view.hpp)
- Mock handler: [`include/mpapp/handlers/mock/shape_view_handler.hpp`](../../../include/mpapp/handlers/mock/shape_view_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/shape_view_handler.hpp`](../../../include/mpapp/handlers/windows/shape_view_handler.hpp) + [`src/handlers/windows/shape_view_handler.cpp`](../../../src/handlers/windows/shape_view_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/shape_view_handler.hpp`](../../../include/mpapp/handlers/linux/shape_view_handler.hpp) + [`src/handlers/linux/shape_view_handler.cpp`](../../../src/handlers/linux/shape_view_handler.cpp)
  - Android: [`include/mpapp/handlers/android/shape_view_handler.hpp`](../../../include/mpapp/handlers/android/shape_view_handler.hpp) + [`src/handlers/android/shape_view_handler.cpp`](../../../src/handlers/android/shape_view_handler.cpp)
- Tests: [`tests/mock_handlers/shape_view_test.cpp`](../../../tests/mock_handlers/shape_view_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[GraphicsView]]
