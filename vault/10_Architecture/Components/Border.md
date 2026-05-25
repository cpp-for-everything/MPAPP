---
type: component
mauiHandler: "Border"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/border"
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

# Border

> [!info] Status
> **3-of-5 platforms real (compile-verified)** — `content` + `padding` + `stroke` + `stroke_thickness` + `stroke_shape` (parsed as `RoundRectangle(N)` for uniform or `RoundRectangle(tl,tr,br,bl)` for per-corner) surface real on WinUI 3 (`mux::Controls::Border` natively exposes `Background`, `BorderBrush`, `BorderThickness`, `CornerRadius`, `Padding`, `Child`), GTK4 (`GtkBox` + per-instance `GtkCssProvider` carrying `border: Npx solid rgba(...)` + `border-radius: tl tr br bl` + `padding: t r b l` — single-child via `gtk_box_append`), and Android (`android.widget.FrameLayout` with a `GradientDrawable` background rebuilt on every property change — `setStroke(width, color)` + `setCornerRadii(float[8])`; padding routed through `View.setPadding`). Dash array / line cap / line join / miter limit on the cross-platform surface remain Observable but are not yet plumbed through any real handler — they're deferred to the M-05 polish track behind the shape language work. macOS / iOS handlers planned in M-06.

> [!info] Original status
> **mock** — cross-platform header at `include/mpapp/border.hpp`; mock handler records the full stroke pen + shape + content surface. See [[Controls Inventory]].

## Overview

`Border` is a single-child decorator that draws a stroke, a background fill, and an optional non-rectangular outline around its `Content`. Its `StrokeShape` accepts any `IShape` — typically `Rectangle`, `RoundRectangle`, or `Ellipse` — letting authors create pills, cards, and circular avatars without bitmap masking. Stroke styling is the standard pen surface (`StrokeThickness`, `StrokeDashArray`, `StrokeDashOffset`, `StrokeLineCap`, `StrokeLineJoin`, `StrokeMiterLimit`). It supersedes the obsolete [[Frame]] control as of .NET 9 — new code should use `Border` everywhere.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_border` | [`include/mpapp/internal/basic_border.hpp`](../../../include/mpapp/internal/basic_border.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::border` | [`include/mpapp/border.hpp`](../../../include/mpapp/border.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/border.hpp>

mpapp::border w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/border.hpp>
#include <mpapp/handlers/mock/border_handler.hpp>

mpapp::internal::basic_border w;
mpapp::border_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::border_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::border_handler<>` and `mpapp::border_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Border\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Border\Border.cs`
- **Docs:** [Microsoft .NET MAUI — Border](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/border)

`BorderHandler.Mapper` extends `ViewMapper` with: `Background`, `Content`, `Shape`, `Stroke`, `StrokeThickness`, `StrokeLineCap`, `StrokeLineJoin`, `StrokeDashPattern`, `StrokeDashOffset`, `StrokeMiterLimit` (and `Width`/`Height` on Android). The control also adds `Padding` and `SafeAreaEdges`.

## MPAPP C++ API

```cpp
namespace mpapp {

class border : public view {
public:
    // Content
    Observable<std::shared_ptr<view>>      content;
    Observable<thickness>                  padding;
    Observable<safe_area_edges>            safe_area_edges;

    // Shape & fill
    Observable<std::shared_ptr<shape>>     stroke_shape;   // default: rectangle
    Observable<brush_ref>                  background;

    // Stroke pen
    Observable<brush_ref>                  stroke;
    Observable<double>                     stroke_thickness;       // default 1.0
    Observable<std::vector<double>>        stroke_dash_array;
    Observable<double>                     stroke_dash_offset;     // default 0.0
    Observable<pen_line_cap>               stroke_line_cap;        // flat | round | square
    Observable<pen_line_join>              stroke_line_join;       // miter | round | bevel
    Observable<double>                     stroke_miter_limit;     // default 10.0
};

} // namespace mpapp
```

## XAML Usage

```xml
<Border Stroke="Black"
        StrokeThickness="2"
        Background="LightYellow"
        Padding="12">
    <Border.StrokeShape>
        <RoundRectangle CornerRadius="12" />
    </Border.StrokeShape>
    <Label Text="Bordered content" />
</Border>
```

## Platform Notes

| Platform | Native control                                              | Header / source            | Notes |
|----------|-------------------------------------------------------------|----------------------------|-------|
| Windows  | `Microsoft.UI.Xaml.Controls.Panel` (custom `ContentPanel`)  | C++/WinRT                  | A `MauiPanel` subclass with a clipping `Path` element for non-rectangular `StrokeShape`. |
| Android  | `android.view.ViewGroup` (custom `ContentViewGroup`)        | fbjni / JNI                | Stroke drawn via `Drawable` overlay on the view group. |
| Linux    | `GtkBox` with a CSS-styled border / `GtkFrame`              | gtk4-rs                    | Non-rectangular shapes drawn via `GtkSnapshot` cairo overrides. |
| macOS    | `NSView` / `UIView` (custom `ContentView`)                  | AppKit / Catalyst          | Stroke rendered into the layer. |
| iOS      | `UIKit.UIView` (custom `ContentView` → `MauiView`)          | UIKit                      | Mask + stroke layer composited on `CALayer`. |

## Side-by-side Examples

### MAUI

```xml
<Border Stroke="Gray" StrokeThickness="1" Padding="8">
    <Border.StrokeShape>
        <RoundRectangle CornerRadius="8" />
    </Border.StrokeShape>
    <Label Text="Card" />
</Border>
```

### MPAPP (XAML)

```xml
<Border Stroke="Gray" StrokeThickness="1" Padding="8">
    <Border.StrokeShape>
        <RoundRectangle CornerRadius="8" />
    </Border.StrokeShape>
    <Label Text="Card" />
</Border>
```

### MPAPP (C++)

```cpp
auto card = mpapp::make<mpapp::border>();
card->stroke           = mpapp::colors::gray;
card->stroke_thickness = 1.0;
card->padding          = mpapp::thickness{8};
card->stroke_shape     = mpapp::shapes::round_rectangle(8);
card->content          = mpapp::make<mpapp::label>("Card");
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Mock implementation

The P2 mock surface (ADR-0008) lands in this repository:

- **Cross-platform header:** `include/mpapp/border.hpp` — `mpapp::border : view` with `content`, `padding`, `stroke_shape`, `stroke`, `stroke_thickness`, `stroke_dash_array`, `stroke_dash_offset`, `stroke_line_cap`, `stroke_line_join`, `stroke_miter_limit`.
- **Mock handler:** `include/mpapp/handlers/mock/border_handler.hpp` — `border_handler<platform::mock>` records every property mapper; `content` is recorded as a presence boolean (no useful `std::format` repr for `shared_ptr<view>`) and `stroke_dash_array` as a size.
- **Mock tests:** `tests/mock_handlers/border_test.cpp`.

The rich `shape` / `brush_ref` types are lightweight stand-ins in the mock layer (`stroke_shape_desc` is a textual descriptor); the full graphics types arrive in P3.

## Implementation

- Surface: [`include/mpapp/border.hpp`](../../../include/mpapp/border.hpp)
- Mock handler: [`include/mpapp/handlers/mock/border_handler.hpp`](../../../include/mpapp/handlers/mock/border_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/border_handler.hpp`](../../../include/mpapp/handlers/windows/border_handler.hpp) + [`src/handlers/windows/border_handler.cpp`](../../../src/handlers/windows/border_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/border_handler.hpp`](../../../include/mpapp/handlers/linux/border_handler.hpp) + [`src/handlers/linux/border_handler.cpp`](../../../src/handlers/linux/border_handler.cpp)
  - Android: [`include/mpapp/handlers/android/border_handler.hpp`](../../../include/mpapp/handlers/android/border_handler.hpp) + [`src/handlers/android/border_handler.cpp`](../../../src/handlers/android/border_handler.cpp)
- Tests: [`tests/mock_handlers/border_test.cpp`](../../../tests/mock_handlers/border_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Frame]]
- [[View]]
