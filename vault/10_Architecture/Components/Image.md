---
type: component
mauiHandler: "Image"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/image"
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

# Image

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Image` displays a bitmap or animated picture. Sources may be a file path, a URL, an embedded resource, or a stream — encapsulated by `image_source`. The `aspect` property selects scaling behaviour: fit, fill, or center. Loading is asynchronous; `is_loading` reports progress, and `is_animation_playing` controls GIF/APNG playback.

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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/image/mock_test.cpp` (planned)
- Windows handler: `tests/components/image/windows_test.cpp` (planned)
- Android handler: `tests/components/image/android_test.cpp` (planned)
- Linux handler: `tests/components/image/linux_test.cpp` (planned)
- macOS handler: `tests/components/image/macos_test.cpp` (planned)
- iOS handler: `tests/components/image/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `ImageSource` | Polymorphic class hierarchy | `image_source` variant (discriminated union) | Static dispatch per [[Type System]] | RFC TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ImageButton]]
