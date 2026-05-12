---
type: component
mauiHandler: "Label"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/label"
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

# Label

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Label` is the read-only text display primitive. It renders single- or multi-line strings with a font, color, alignment, line height, and optional underline/strike decorations, but exposes no input or focus surface. Per MAUI's `ILabel`, it composes `IText`, `ITextAlignment`, and `IPadding` — making it the foundational widget for static copy, headings, and inline labels in forms.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Label\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Label\`
- **Docs:** [Microsoft .NET MAUI — Label](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/label)

## MPAPP C++ API

```cpp
namespace mpapp {

class label : public control<label> {
public:
    Observable<std::string>           text;
    Observable<color>                 text_color;
    Observable<font>                  font;
    Observable<double>                character_spacing;
    Observable<text_alignment>        horizontal_text_alignment;
    Observable<text_alignment>        vertical_text_alignment;
    Observable<text_decorations>      text_decorations;
    Observable<double>                line_height;
    Observable<thickness>             padding;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Label Text="Welcome"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.TextBlock` | C++/WinRT | Inlines used for `FormattedString`. |
| Android | `AndroidX.AppCompat.Widget.AppCompatTextView` | fbjni / JNI | Honors AppCompat theming. |
| Linux | `GtkLabel` | GTK4 | Pango markup for decorations. |
| macOS | `NSTextField` (non-editable) | AppKit | Bezel/border disabled. |
| iOS | `UILabel` | UIKit | `numberOfLines = 0` for multi-line. |

## Side-by-side Examples

### MAUI

```xml
<Label Text="Hello, world"
       FontSize="24"
       TextColor="Black"
       HorizontalTextAlignment="Center"/>
```

### MPAPP (XAML)

```xml
<Label Text="Hello, world"
       FontSize="24"
       TextColor="Black"
       HorizontalTextAlignment="Center"/>
```

### MPAPP (C++)

```cpp
auto l = std::make_shared<mpapp::label>();
l->text = "Hello, world";
l->font = font{ .size = 24 };
l->text_color = colors::black;
l->horizontal_text_alignment = text_alignment::center;
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/label/mock_test.cpp` (planned)
- Windows handler: `tests/components/label/windows_test.cpp` (planned)
- Android handler: `tests/components/label/android_test.cpp` (planned)
- Linux handler: `tests/components/label/linux_test.cpp` (planned)
- macOS handler: `tests/components/label/macos_test.cpp` (planned)
- iOS handler: `tests/components/label/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
