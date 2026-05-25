---
type: component
mauiHandler: "Label"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/label"
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

# Label

> [!info] Status
> **3-of-5 platforms real** — `text` surface verified live on WinUI 3 (`mux::TextBlock`), GTK4 (`GtkLabel`), and Android (`android.widget.TextView`) under [[../50_Tasks/T-0011-app-shell-abstraction/T-0011-app-shell-abstraction]]. macOS (`NSTextField` in label mode) and iOS (`UILabel`) handlers are code-complete pending an Apple host.

## Overview

`Label` is the read-only text display primitive. It renders single- or multi-line strings with a font, color, alignment, line height, and optional underline/strike decorations, but exposes no input or focus surface. Per MAUI's `ILabel`, it composes `IText`, `ITextAlignment`, and `IPadding` — making it the foundational widget for static copy, headings, and inline labels in forms.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_label` | [`include/mpapp/internal/basic_label.hpp`](../../../include/mpapp/internal/basic_label.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::label` | [`include/mpapp/label.hpp`](../../../include/mpapp/label.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/label.hpp>

mpapp::label w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/label.hpp>
#include <mpapp/handlers/mock/label_handler.hpp>

mpapp::internal::basic_label w;
mpapp::label_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::label_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::label_handler<>` and `mpapp::label_handler<platform::mock>` valid spellings without naming `internal::`.

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

## Mock implementation

- Handler: [`include/mpapp/handlers/mock/label_handler.hpp`](../../../include/mpapp/handlers/mock/label_handler.hpp)
- Tests: [`tests/mock_handlers/label_test.cpp`](../../../tests/mock_handlers/label_test.cpp)

`label_handler<platform::mock>` records `text=<value>` into `calls()`; tests cover initial-value capture on map, sequential updates, and the no-op-on-same-value contract.

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Implementation

- Surface: [`include/mpapp/label.hpp`](../../../include/mpapp/label.hpp)
- Mock handler: [`include/mpapp/handlers/mock/label_handler.hpp`](../../../include/mpapp/handlers/mock/label_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/label_handler.hpp`](../../../include/mpapp/handlers/windows/label_handler.hpp) + [`src/handlers/windows/label_handler.cpp`](../../../src/handlers/windows/label_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/label_handler.hpp`](../../../include/mpapp/handlers/linux/label_handler.hpp) + [`src/handlers/linux/label_handler.cpp`](../../../src/handlers/linux/label_handler.cpp)
  - Android: [`include/mpapp/handlers/android/label_handler.hpp`](../../../include/mpapp/handlers/android/label_handler.hpp) + [`src/handlers/android/label_handler.cpp`](../../../src/handlers/android/label_handler.cpp)
- Tests: [`tests/mock_handlers/label_test.cpp`](../../../tests/mock_handlers/label_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
