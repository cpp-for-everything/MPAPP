---
type: component
mauiHandler: "ImageButton"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/imagebutton"
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

# ImageButton

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`ImageButton` is a tappable button whose visual is an image rather than a text label — semantically `Button` + `Image`. It exposes the full button surface (`command`, `clicked`/`pressed`/`released` events, `is_pressed`, border, corner radius, padding) plus image-loading state. Use it for icon-only toolbars and toolbar-shelf actions.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_image_button` | [`include/mpapp/internal/basic_image_button.hpp`](../../../include/mpapp/internal/basic_image_button.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::image_button` | [`include/mpapp/image_button.hpp`](../../../include/mpapp/image_button.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/image_button.hpp>

mpapp::image_button w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/image_button.hpp>
#include <mpapp/handlers/mock/image_button_handler.hpp>

mpapp::internal::basic_image_button w;
mpapp::image_button_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::image_button_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::image_button_handler<>` and `mpapp::image_button_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\ImageButton\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\ImageButton\`
- **Docs:** [Microsoft .NET MAUI — ImageButton](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/imagebutton)

## MPAPP C++ API

```cpp
namespace mpapp {

class image_button : public control<image_button> {
public:
    // Image
    Observable<image_source> source;
    Observable<aspect_mode>  aspect{aspect_mode::aspect_fit};
    Observable<bool>         is_opaque{false};

    // Button surface
    Observable<color>    border_color;
    Observable<double>   border_width{0.0};
    Observable<int>      corner_radius{-1};
    Observable<thickness> padding;

    // Read-only status
    Observable<bool> is_loading{false};
    Observable<bool> is_pressed{false};

    // Commands / events
    Command<>          clicked;
    Command<>          pressed;
    Command<>          released;
    Command<std::any>  command;           // mirrors MAUI ICommand
};

} // namespace mpapp
```

See [[Button]] for the shared button surface and [[Image]] for the source side.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<ImageButton Source="favorite.png"
             Command="{Binding ToggleFavoriteCmd}"
             CornerRadius="16"
             Padding="12"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.Button` wrapping `Image` | C++/WinRT | Inner `Image` is exposed for the image-handler mapper. |
| Android | `Google.Android.Material.ImageView.ShapeableImageView` | fbjni / JNI | Tap surface is the image itself; ripple drawable for press feedback. |
| Linux | `GtkButton` containing `GtkPicture` | GTK4 | Native press states. |
| macOS | `NSButton` configured as image-only | AppKit | `bezelStyle = .regularSquare` with image. |
| iOS | `UIKit.UIButton` (image-only) | UIKit | Inner `UIImageView` used by the image-handler mapper. |

## Side-by-side Examples

### MAUI

```xml
<ImageButton Source="star.png"
             Clicked="OnStarClicked"
             CornerRadius="8"/>
```

### MPAPP (XAML)

```xml
<ImageButton Source="star.png"
             Clicked="OnStarClicked"
             CornerRadius="8"/>
```

### MPAPP (C++)

```cpp
auto ib = std::make_shared<mpapp::image_button>();
ib->source = mpapp::image_source::from_file("star.png");
ib->corner_radius = 8;
ib->clicked.subscribe([]{ /* toggle favorite */ });
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Implementation

- Surface: [`include/mpapp/image_button.hpp`](../../../include/mpapp/image_button.hpp)
- Mock handler: [`include/mpapp/handlers/mock/image_button_handler.hpp`](../../../include/mpapp/handlers/mock/image_button_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/image_button_handler.hpp`](../../../include/mpapp/handlers/windows/image_button_handler.hpp) + [`src/handlers/windows/image_button_handler.cpp`](../../../src/handlers/windows/image_button_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/image_button_handler.hpp`](../../../include/mpapp/handlers/linux/image_button_handler.hpp) + [`src/handlers/linux/image_button_handler.cpp`](../../../src/handlers/linux/image_button_handler.cpp)
  - Android: [`include/mpapp/handlers/android/image_button_handler.hpp`](../../../include/mpapp/handlers/android/image_button_handler.hpp) + [`src/handlers/android/image_button_handler.cpp`](../../../src/handlers/android/image_button_handler.cpp)
- Tests: [`tests/mock_handlers/image_button_test.cpp`](../../../tests/mock_handlers/image_button_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Button]]
- [[Image]]
