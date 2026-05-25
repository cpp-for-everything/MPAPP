---
type: component
mauiHandler: "GraphicsView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/graphicsview"
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

# GraphicsView

> [!info] Status
> **android-real (v2 — all 3 platforms landed via [[T-0029]] phases 1+2)** — Native drawing host wired on all 3 platforms with real canvas-facade integration:
>
> - **Linux** (`GtkDrawingArea`) — the GTK draw callback wraps the facade's BGRA32 pixels via `cairo_image_surface_create_for_data` and blits through GTK's `cairo_t*`. No channel swap.
> - **Windows** (`muxc::Image` + `WriteableBitmap`) — facade pixels copied into the WriteableBitmap's `PixelBuffer` (BGRA8 premultiplied, same byte order — single memcpy per row). `WriteableBitmap.Invalidate()` flips the surface.
> - **Android** (`android.widget.ImageView` + `android.graphics.Bitmap` ARGB_8888) — facade pixels copied with a per-pixel B↔R swap (`ANDROID_BITMAP_FORMAT_RGBA_8888` is RGBA byte order in memory) inside an `AndroidBitmap_lockPixels`/`unlockPixels` pair, then `setImageBitmap` re-renders.
>
> Surface API is uniform across platforms: user installs a `drawable` Observable holding `std::function<void(canvas&)>` from the ADR-0015 facade; `width` / `height` propagate to the native widget; `invalidate()` (or replacing the `drawable` callback) triggers a repaint. `map_draw_count` + `map_drawable` subscribe to the relevant Observable on every platform and trigger the per-platform repaint path.

## Overview

`GraphicsView` is an arbitrary 2D drawing surface backed by a `drawable` callback. The framework asks the drawable to render into a platform-neutral `canvas` (the [Microsoft.Maui.Graphics](https://github.com/dotnet/Microsoft.Maui.Graphics) abstraction on MAUI). It also forwards pointer/touch events as `start_interaction`, `drag_interaction`, `end_interaction`, plus hover variants. Use it when you want custom rendering without dropping to native graphics APIs per platform.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_graphics_view` | [`include/mpapp/internal/basic_graphics_view.hpp`](../../../include/mpapp/internal/basic_graphics_view.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::graphics_view` | [`include/mpapp/graphics_view.hpp`](../../../include/mpapp/graphics_view.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_gv_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/graphics_view.hpp>

mpapp::graphics_view w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/graphics_view.hpp>
#include <mpapp/handlers/mock/graphics_view_handler.hpp>

mpapp::internal::basic_graphics_view w;
mpapp::graphics_view_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::graphics_view_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::graphics_view_handler<>` and `mpapp::graphics_view_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\GraphicsView\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\GraphicsView\`
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

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Drawing API | `Microsoft.Maui.Graphics` (`ICanvas`) | `mpapp::canvas` thin C++ wrapper; intent to mirror surface 1:1 | Independent C++ implementation per [[Type System]] | RFC TBD |

## Implementation

- Surface: [`include/mpapp/graphics_view.hpp`](../../../include/mpapp/graphics_view.hpp)
- Mock handler: [`include/mpapp/handlers/mock/graphics_view_handler.hpp`](../../../include/mpapp/handlers/mock/graphics_view_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/graphics_view_handler.hpp`](../../../include/mpapp/handlers/windows/graphics_view_handler.hpp) + [`src/handlers/windows/graphics_view_handler.cpp`](../../../src/handlers/windows/graphics_view_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/graphics_view_handler.hpp`](../../../include/mpapp/handlers/linux/graphics_view_handler.hpp) + [`src/handlers/linux/graphics_view_handler.cpp`](../../../src/handlers/linux/graphics_view_handler.cpp)
  - Android: [`include/mpapp/handlers/android/graphics_view_handler.hpp`](../../../include/mpapp/handlers/android/graphics_view_handler.hpp) + [`src/handlers/android/graphics_view_handler.cpp`](../../../src/handlers/android/graphics_view_handler.cpp)
- Tests: [`tests/mock_handlers/graphics_view_test.cpp`](../../../tests/mock_handlers/graphics_view_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ShapeView]]
