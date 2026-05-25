---
type: component
mauiHandler: "RadioButton"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/radiobutton"
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

# RadioButton

> [!info] Status
> **3-of-5 platforms real** — `is_checked: Observable<bool>` + `group_name: Observable<std::string>` surface real on WinUI 3 (`mux::Controls::RadioButton` with native `GroupName` auto-grouping), GTK4 (`GtkCheckButton` with `gtk_check_button_set_group` via a per-process group-leader registry keyed on `group_name`), and Android (`android.widget.RadioButton` inside an auto-allocated `android.widget.RadioGroup` per `group_name`, with reverse binding through the shared `MppCheckedChangeListener` JNI bridge using `kind=3`). Compiles + builds clean on all three platforms; binding pipeline shares its code path with the CheckBox handler (also live-verified on Android — same bridge, just a different `kind`). macOS / iOS handlers code-complete pending an Apple host.

## Overview

`RadioButton` is a single-selection toggle: at most one button in a named group is checked at a time. Each instance carries a text label (or content view), is grouped via a `GroupName` string, and exposes the standard text-style and stroke-style contracts so the indicator ring can be themed. Only Windows ships a native radio button; the other platforms render through `ContentView`/custom group containers, with selection mutual-exclusion enforced by the handler.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_radio_button` | [`include/mpapp/internal/basic_radio_button.hpp`](../../../include/mpapp/internal/basic_radio_button.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::radio_button` | [`include/mpapp/radio_button.hpp`](../../../include/mpapp/radio_button.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/radio_button.hpp>

mpapp::radio_button w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/radio_button.hpp>
#include <mpapp/handlers/mock/radio_button_handler.hpp>

mpapp::internal::basic_radio_button w;
mpapp::radio_button_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::radio_button_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::radio_button_handler<>` and `mpapp::radio_button_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\RadioButton\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\RadioButton\`
- **Docs:** [Microsoft .NET MAUI — RadioButton](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/radiobutton)

## MPAPP C++ API

```cpp
namespace mpapp {

class radio_button : public control<radio_button> {
public:
    Observable<bool>            is_checked;
    Observable<std::string>     group_name;
    Observable<std::any>        content;
    Observable<color>           text_color;
    Observable<font>            font;
    Observable<double>          character_spacing;
    Observable<color>           stroke_color;
    Observable<double>          stroke_thickness;
    Observable<corner_radius>   corner_radius;

    Command<bool>               checked_changed;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<RadioButton Content="Option A" GroupName="choice"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.RadioButton` | C++/WinRT | Native group semantics via shared `GroupName`. |
| Android | Custom `Android.Views.View` (uses `MaterialRadioButton` internally) | fbjni / JNI | Group enforced by handler. |
| Linux | `GtkCheckButton` with `set-group` linking siblings | GTK4 | Renders as radio when grouped. |
| macOS | `NSButton` with `setButtonType: NSRadioButton` | AppKit | Catalyst uses `ContentView`. |
| iOS | `MauiContentView` host | UIKit | No native `UIRadioButton`; handler draws ring. |

## Side-by-side Examples

### MAUI

```xml
<StackLayout>
    <RadioButton Content="Small"  GroupName="size" IsChecked="True"/>
    <RadioButton Content="Medium" GroupName="size"/>
    <RadioButton Content="Large"  GroupName="size"/>
</StackLayout>
```

### MPAPP (XAML)

```xml
<StackLayout>
    <RadioButton Content="Small"  GroupName="size" IsChecked="True"/>
    <RadioButton Content="Medium" GroupName="size"/>
    <RadioButton Content="Large"  GroupName="size"/>
</StackLayout>
```

### MPAPP (C++)

```cpp
auto small = std::make_shared<mpapp::radio_button>();
small->content = std::string{"Small"};
small->group_name = "size";
small->is_checked = true;

auto medium = std::make_shared<mpapp::radio_button>();
medium->content = std::string{"Medium"};
medium->group_name = "size";
```

## Mock implementation

- Handler: [`include/mpapp/handlers/mock/radio_button_handler.hpp`](../../../include/mpapp/handlers/mock/radio_button_handler.hpp)
- Tests: [`tests/mock_handlers/radio_button_test.cpp`](../../../tests/mock_handlers/radio_button_test.cpp)

`radio_button_handler<platform::mock>` records changes to `is_checked` and `group_name`. The mock surface drops the `content` (Observable<std::any>) slot for now — primitive Observables suffice to validate the mapper plumbing.

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Implementation

- Surface: [`include/mpapp/radio_button.hpp`](../../../include/mpapp/radio_button.hpp)
- Mock handler: [`include/mpapp/handlers/mock/radio_button_handler.hpp`](../../../include/mpapp/handlers/mock/radio_button_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/radio_button_handler.hpp`](../../../include/mpapp/handlers/windows/radio_button_handler.hpp) + [`src/handlers/windows/radio_button_handler.cpp`](../../../src/handlers/windows/radio_button_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/radio_button_handler.hpp`](../../../include/mpapp/handlers/linux/radio_button_handler.hpp) + [`src/handlers/linux/radio_button_handler.cpp`](../../../src/handlers/linux/radio_button_handler.cpp)
  - Android: [`include/mpapp/handlers/android/radio_button_handler.hpp`](../../../include/mpapp/handlers/android/radio_button_handler.hpp) + [`src/handlers/android/radio_button_handler.cpp`](../../../src/handlers/android/radio_button_handler.cpp)
- Tests: [`tests/mock_handlers/radio_button_test.cpp`](../../../tests/mock_handlers/radio_button_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
