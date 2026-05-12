---
type: component
mauiHandler: "RadioButton"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/radiobutton"
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

# RadioButton

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`RadioButton` is a single-selection toggle: at most one button in a named group is checked at a time. Each instance carries a text label (or content view), is grouped via a `GroupName` string, and exposes the standard text-style and stroke-style contracts so the indicator ring can be themed. Only Windows ships a native radio button; the other platforms render through `ContentView`/custom group containers, with selection mutual-exclusion enforced by the handler.

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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/radio_button/mock_test.cpp` (planned)
- Windows handler: `tests/components/radio_button/windows_test.cpp` (planned)
- Android handler: `tests/components/radio_button/android_test.cpp` (planned)
- Linux handler: `tests/components/radio_button/linux_test.cpp` (planned)
- macOS handler: `tests/components/radio_button/macos_test.cpp` (planned)
- iOS handler: `tests/components/radio_button/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
