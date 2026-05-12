---
type: component
mauiHandler: "ImageButton"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/imagebutton"
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

# ImageButton

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`ImageButton` is a tappable button whose visual is an image rather than a text label — semantically `Button` + `Image`. It exposes the full button surface (`command`, `clicked`/`pressed`/`released` events, `is_pressed`, border, corner radius, padding) plus image-loading state. Use it for icon-only toolbars and toolbar-shelf actions.

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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/imagebutton/mock_test.cpp` (planned)
- Windows handler: `tests/components/imagebutton/windows_test.cpp` (planned)
- Android handler: `tests/components/imagebutton/android_test.cpp` (planned)
- Linux handler: `tests/components/imagebutton/linux_test.cpp` (planned)
- macOS handler: `tests/components/imagebutton/macos_test.cpp` (planned)
- iOS handler: `tests/components/imagebutton/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Button]]
- [[Image]]
