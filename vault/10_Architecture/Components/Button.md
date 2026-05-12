---
type: component
mauiHandler: "Button"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/button"
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

# Button

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Button` is a tappable command surface that shows a text label and/or an image and raises a click event when activated. It is the canonical primary action control: every supported platform maps it to a native push-button widget so platform conventions for focus, ripple, hover, and accessibility carry over without re-implementation. Per MAUI's `IButton`, it composes text styling, padding, stroke, and image-source contracts.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Button\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Button\`
- **Docs:** [Microsoft .NET MAUI — Button](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/button)

## MPAPP C++ API

```cpp
namespace mpapp {

class button : public control<button> {
public:
    Observable<std::string>   text;
    Observable<color>         text_color;
    Observable<font>          font;
    Observable<double>        character_spacing;
    Observable<image_source>  image_source;
    Observable<thickness>     padding;
    Observable<color>         stroke_color;
    Observable<double>        stroke_thickness;
    Observable<double>        corner_radius;
    Observable<brush>         background;

    Command<>                 clicked;
    Command<>                 pressed;
    Command<>                 released;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Button Text="Save" Clicked="OnSaveClicked"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.Button` | C++/WinRT | Stroke maps to a contained `Border`. |
| Android | `Google.Android.Material.Button.MaterialButton` (AppCompat) | fbjni / JNI | Material 3 ripple, elevation honored. |
| Linux | `GtkButton` | GTK4 | Image-plus-text via `GtkBox` child. |
| macOS | `NSButton` (push-button style) | AppKit | Catalyst path falls back to `UIButton`. |
| iOS | `UIButton` (system style) | UIKit | Padding mapped to `contentEdgeInsets`. |

## Side-by-side Examples

### MAUI

```xml
<Button Text="Save"
        TextColor="White"
        BackgroundColor="DodgerBlue"
        Clicked="OnSaveClicked"/>
```

### MPAPP (XAML)

```xml
<Button Text="Save"
        TextColor="White"
        Background="DodgerBlue"
        Clicked="{Binding OnSaveClicked}"/>
```

### MPAPP (C++)

```cpp
auto b = std::make_shared<mpapp::button>();
b->text = "Save";
b->text_color = colors::white;
b->background = brush{colors::dodger_blue};
b->clicked.subscribe([] { save(); });
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/button/mock_test.cpp` (planned)
- Windows handler: `tests/components/button/windows_test.cpp` (planned)
- Android handler: `tests/components/button/android_test.cpp` (planned)
- Linux handler: `tests/components/button/linux_test.cpp` (planned)
- macOS handler: `tests/components/button/macos_test.cpp` (planned)
- iOS handler: `tests/components/button/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
