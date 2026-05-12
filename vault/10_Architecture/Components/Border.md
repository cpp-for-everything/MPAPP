---
type: component
mauiHandler: "Border"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/border"
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

# Border

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Border` is a single-child decorator that draws a stroke, a background fill, and an optional non-rectangular outline around its `Content`. Its `StrokeShape` accepts any `IShape` — typically `Rectangle`, `RoundRectangle`, or `Ellipse` — letting authors create pills, cards, and circular avatars without bitmap masking. Stroke styling is the standard pen surface (`StrokeThickness`, `StrokeDashArray`, `StrokeDashOffset`, `StrokeLineCap`, `StrokeLineJoin`, `StrokeMiterLimit`). It supersedes the obsolete [[Frame]] control as of .NET 9 — new code should use `Border` everywhere.

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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/border/mock_test.cpp` (planned)
- Windows handler: `tests/components/border/windows_test.cpp` (planned)
- Android handler: `tests/components/border/android_test.cpp` (planned)
- Linux handler: `tests/components/border/linux_test.cpp` (planned)
- macOS handler: `tests/components/border/macos_test.cpp` (planned)
- iOS handler: `tests/components/border/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Frame]]
- [[View]]
