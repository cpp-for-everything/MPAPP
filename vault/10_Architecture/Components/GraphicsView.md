---
type: component
mauiHandler: "GraphicsView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/graphicsview"
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

# GraphicsView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`GraphicsView` is an arbitrary 2D drawing surface backed by a `drawable` callback. The framework asks the drawable to render into a platform-neutral `canvas` (the [Microsoft.Maui.Graphics](https://github.com/dotnet/Microsoft.Maui.Graphics) abstraction on MAUI). It also forwards pointer/touch events as `start_interaction`, `drag_interaction`, `end_interaction`, plus hover variants. Use it when you want custom rendering without dropping to native graphics APIs per platform.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\GraphicsView\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\GraphicsView\`
- **Docs:** [Microsoft .NET MAUI — GraphicsView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/graphicsview)

## MPAPP C++ API

```cpp
namespace mpapp {

// Abstract drawing target. Implemented by the framework over Skia / D2D / CoreGraphics.
struct canvas;

// User-supplied draw callback.
struct drawable {
    virtual ~drawable() = default;
    virtual void draw(canvas& target, rect dirty) = 0;
};

class graphics_view : public control<graphics_view> {
public:
    // Drawing
    Observable<std::shared_ptr<drawable>> drawable_;

    // Trigger a redraw
    Command<>            invalidate;

    // Touch / pointer
    Command<touch_event> start_interaction;
    Command<touch_event> drag_interaction;
    Command<touch_event> end_interaction;
    Command<>            cancel_interaction;
    Command<touch_event> start_hover_interaction;
    Command<touch_event> move_hover_interaction;
    Command<>            end_hover_interaction;
};

} // namespace mpapp
```

The native handler hosts a `PlatformTouchGraphicsView` on every platform — see [[Handlers]] and [[Platform Interop]].

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<GraphicsView Drawable="{StaticResource SparklineDrawable}"
              HeightRequest="80" WidthRequest="240"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `PlatformTouchGraphicsView` over Win2D | C++/WinRT | Backed by `W2DGraphicsView`. |
| Android | `PlatformTouchGraphicsView` (`View`) | fbjni / JNI | Software canvas; SkiaSharp optional. |
| Linux | Custom `GtkDrawingArea` | GTK4 | Backed by Cairo or Skia. |
| macOS | `PlatformTouchGraphicsView` (`NSView`) | AppKit | CoreGraphics-backed drawing context. |
| iOS | `PlatformTouchGraphicsView` (`UIView`) | UIKit | CoreGraphics drawing context. |

## Side-by-side Examples

### MAUI

```xml
<GraphicsView x:Name="canvas"
              Drawable="{StaticResource ChartDrawable}"
              StartInteraction="OnStart"
              DragInteraction="OnDrag"/>
```

### MPAPP (XAML)

```xml
<GraphicsView x:Name="canvas"
              Drawable="{StaticResource ChartDrawable}"
              StartInteraction="OnStart"
              DragInteraction="OnDrag"/>
```

### MPAPP (C++)

```cpp
struct sparkline : mpapp::drawable {
    std::vector<float> values;
    void draw(mpapp::canvas& g, mpapp::rect r) override {
        g.stroke_color(mpapp::colors::azure);
        // ... path drawing ...
    }
};

auto gv = std::make_shared<mpapp::graphics_view>();
gv->drawable_ = std::make_shared<sparkline>();
gv->drag_interaction.subscribe([&](mpapp::touch_event e){ /* hit-test */ });
gv->invalidate();
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/graphicsview/mock_test.cpp` (planned)
- Windows handler: `tests/components/graphicsview/windows_test.cpp` (planned)
- Android handler: `tests/components/graphicsview/android_test.cpp` (planned)
- Linux handler: `tests/components/graphicsview/linux_test.cpp` (planned)
- macOS handler: `tests/components/graphicsview/macos_test.cpp` (planned)
- iOS handler: `tests/components/graphicsview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Drawing API | `Microsoft.Maui.Graphics` (`ICanvas`) | `mpapp::canvas` thin C++ wrapper; intent to mirror surface 1:1 | Independent C++ implementation per [[Type System]] | RFC TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ShapeView]]
