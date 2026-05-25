---
type: component
mauiHandler: "Image"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/image"
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

# Image

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Image` displays a bitmap or animated picture. Sources may be a file path, a URL, an embedded resource, or a stream — encapsulated by `image_source`. The `aspect` property selects scaling behaviour: fit, fill, or center. Loading is asynchronous; `is_loading` reports progress, and `is_animation_playing` controls GIF/APNG playback.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_image` | [`include/mpapp/internal/basic_image.hpp`](../../../include/mpapp/internal/basic_image.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::image` | [`include/mpapp/image.hpp`](../../../include/mpapp/image.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/image.hpp>

mpapp::image w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/image.hpp>
#include <mpapp/handlers/mock/image_handler.hpp>

mpapp::internal::basic_image w;
mpapp::image_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::image_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::image_handler<>` and `mpapp::image_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Image\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Image\`
- **Docs:** [Microsoft .NET MAUI — Image](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/image)

## MPAPP C++ API

```cpp
namespace mpapp {

enum class aspect_mode { aspect_fit, aspect_fill, fill, center };

class image : public control<image> {
public:
    // Source
    Observable<image_source> source;             // file/uri/stream/resource
    Observable<aspect_mode>  aspect{aspect_mode::aspect_fit};
    Observable<bool>         is_opaque{false};

    // Animation
    Observable<bool> is_animation_playing{true};

    // Read-only status
    Observable<bool> is_loading{false};
};

} // namespace mpapp
```

`image_source` is a discriminated union mirroring `Microsoft.Maui.Controls.ImageSource` (file / uri / stream / font / resource). See [[Observable Properties]].

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Image Source="logo.png" Aspect="AspectFit"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.Image` | C++/WinRT | Decodes via WIC; supports SVG via `SvgImageSource`. |
| Android | `android.widget.ImageView` | fbjni / JNI | Glide or stock `BitmapFactory` for decoding. |
| Linux | `GtkPicture` | GTK4 | Backed by `GdkTexture`. |
| macOS | `NSImageView` | AppKit | Accepts `NSImage` (including PDF + raster). |
| iOS | `UIKit.UIImageView` | UIKit | Accepts `UIImage`; animation via `UIImage.animatedImage`. |

## Side-by-side Examples

### MAUI

```xml
<Image Source="https://example.com/logo.png" Aspect="AspectFit"/>
```

### MPAPP (XAML)

```xml
<Image Source="https://example.com/logo.png" Aspect="AspectFit"/>
```

### MPAPP (C++)

```cpp
auto img = std::make_shared<mpapp::image>();
img->source = mpapp::image_source::from_uri("https://example.com/logo.png");
img->aspect = mpapp::aspect_mode::aspect_fit;
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `ImageSource` | Polymorphic class hierarchy | `image_source` variant (discriminated union) | Static dispatch per [[Type System]] | RFC TBD |

## Implementation

- Surface: [`include/mpapp/image.hpp`](../../../include/mpapp/image.hpp)
- Mock handler: [`include/mpapp/handlers/mock/image_handler.hpp`](../../../include/mpapp/handlers/mock/image_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/image_handler.hpp`](../../../include/mpapp/handlers/windows/image_handler.hpp) + [`src/handlers/windows/image_handler.cpp`](../../../src/handlers/windows/image_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/image_handler.hpp`](../../../include/mpapp/handlers/linux/image_handler.hpp) + [`src/handlers/linux/image_handler.cpp`](../../../src/handlers/linux/image_handler.cpp)
  - Android: [`include/mpapp/handlers/android/image_handler.hpp`](../../../include/mpapp/handlers/android/image_handler.hpp) + [`src/handlers/android/image_handler.cpp`](../../../src/handlers/android/image_handler.cpp)
- Tests: [`tests/mock_handlers/image_test.cpp`](../../../tests/mock_handlers/image_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ImageButton]]
