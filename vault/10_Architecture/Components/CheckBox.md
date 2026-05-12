---
type: component
mauiHandler: "CheckBox"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/checkbox"
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

# CheckBox

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`CheckBox` is a two-state binary selector — `IsChecked` is true or false — typically used in forms where each item is independently opt-in. Unlike `Switch`, the value is conventionally committed only when the surrounding form is submitted. iOS has no native check-box widget, so MAUI ships a custom `MauiCheckBox` drawn with `CoreGraphics`; MPAPP follows the same approach.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\CheckBox\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\CheckBox\`
- **Docs:** [Microsoft .NET MAUI — CheckBox](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/checkbox)

## MPAPP C++ API

```cpp
namespace mpapp {

class check_box : public control<check_box> {
public:
    Observable<bool>   is_checked;
    Observable<paint>  foreground;

    Command<bool>      checked_changed;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<CheckBox IsChecked="True"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.CheckBox` | C++/WinRT | Indeterminate state not surfaced. |
| Android | `AndroidX.AppCompat.Widget.AppCompatCheckBox` | fbjni / JNI | Material check mark. |
| Linux | `GtkCheckButton` | GTK4 | Label child omitted (separate `Label` used). |
| macOS | `NSButton` with `setButtonType: NSSwitchButton` | AppKit | Catalyst uses `MauiCheckBox`. |
| iOS | `MauiCheckBox` (custom `UIView`) | UIKit | Drawn with `CoreGraphics` — no `UICheckBox` exists. |

## Side-by-side Examples

### MAUI

```xml
<CheckBox IsChecked="{Binding AcceptTerms}"
          Color="DodgerBlue"
          CheckedChanged="OnAcceptTermsChanged"/>
```

### MPAPP (XAML)

```xml
<CheckBox IsChecked="{Binding AcceptTerms}"
          Color="DodgerBlue"
          CheckedChanged="{Binding OnAcceptTermsChanged}"/>
```

### MPAPP (C++)

```cpp
auto cb = std::make_shared<mpapp::check_box>();
cb->is_checked = false;
cb->foreground = solid_color_paint{colors::dodger_blue};
cb->checked_changed.subscribe([](bool v) { set_accept_terms(v); });
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/check_box/mock_test.cpp` (planned)
- Windows handler: `tests/components/check_box/windows_test.cpp` (planned)
- Android handler: `tests/components/check_box/android_test.cpp` (planned)
- Linux handler: `tests/components/check_box/linux_test.cpp` (planned)
- macOS handler: `tests/components/check_box/macos_test.cpp` (planned)
- iOS handler: `tests/components/check_box/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
