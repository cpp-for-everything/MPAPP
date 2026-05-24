---
type: component
mauiHandler: "Switch"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/switch"
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

# Switch

> [!info] Status
> **3-of-5 platforms real** — `is_on: Observable<bool>` surface real on WinUI 3 (`mux::Controls::ToggleSwitch` + `Toggled` event), GTK4 (`GtkSwitch` + `state-set` signal), and Android (`android.widget.Switch` + `OnCheckedChangeListener` via the `MppCheckedChangeListener` JNI bridge). **Bidirectional binding** end-to-end live-verified on Android: toggling the switch flips a cross-platform `Observable<bool>` whose user-side observer transforms the label text to ALL CAPS + "!!!". Verified screenshot: `_Archive/T-0011-app-shell-abstraction/screenshots/android-switch-on.png` (label flips Count: 0 — hello, world → Count: 0 — HELLO, WORLD!!! on toggle); `android-switch-clicked2.png` (Count: 2 with switch ON shows clicks ALSO compose with the shout state). macOS / iOS handlers code-complete pending an Apple host.

## Overview

`Switch` is a sliding toggle showing one of two states (`IsOn` true or false). It is the on-off control of choice for settings screens where the change takes effect immediately, in contrast to `CheckBox` which is conventional for opt-in selection in forms. Per MAUI's `ISwitch`, it exposes `IsOn`, `ThumbColor`, and `TrackColor`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Switch\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Switch\`
- **Docs:** [Microsoft .NET MAUI — Switch](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/switch)

## MPAPP C++ API

```cpp
namespace mpapp {

class switch_ : public control<switch_> {
public:
    Observable<bool>   is_on;
    Observable<color>  thumb_color;
    Observable<color>  track_color;

    Command<bool>      toggled;
};

} // namespace mpapp
```

> Note: `switch` is a C++ keyword, so the class is named `switch_` per the project's reserved-word convention.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Switch IsToggled="True"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.ToggleSwitch` | C++/WinRT | Has built-in on/off content labels (suppressed). |
| Android | `AndroidX.AppCompat.Widget.SwitchCompat` | fbjni / JNI | Material thumb/track theming. |
| Linux | `GtkSwitch` | GTK4 | CSS handles thumb/track colors. |
| macOS | `NSSwitch` (10.15+) | AppKit | Catalyst falls back to `UISwitch`. |
| iOS | `UISwitch` | UIKit | `onTintColor` maps to track color. |

## Side-by-side Examples

### MAUI

```xml
<Switch IsToggled="{Binding NotificationsEnabled}"
        OnColor="DodgerBlue"
        ThumbColor="White"/>
```

### MPAPP (XAML)

```xml
<Switch IsToggled="{Binding NotificationsEnabled}"
        OnColor="DodgerBlue"
        ThumbColor="White"/>
```

### MPAPP (C++)

```cpp
auto s = std::make_shared<mpapp::switch_>();
s->is_on = true;
s->track_color = colors::dodger_blue;
s->toggled.subscribe([](bool v) { save_pref("notifications", v); });
```

## Mock implementation

- Handler: [`include/mpapp/handlers/mock/switch_handler.hpp`](../../../include/mpapp/handlers/mock/switch_handler.hpp)
- Tests: [`tests/mock_handlers/switch_test.cpp`](../../../tests/mock_handlers/switch_test.cpp)

`switch_handler<platform::mock>` records `is_on=<bool>` into `calls()`; tests cover the toggle sequence and the no-emit-on-same-value contract.

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Class name | `Switch` | `mpapp::switch_` | `switch` is a C++ reserved keyword. | N/A — XAML element name is unchanged. |

## Implementation

- Mock handler: [`include/mpapp/handlers/mock/switch_handler.hpp`](../../../include/mpapp/handlers/mock/switch_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/switch_handler.hpp`](../../../include/mpapp/handlers/windows/switch_handler.hpp) + [`src/handlers/windows/switch_handler.cpp`](../../../src/handlers/windows/switch_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/switch_handler.hpp`](../../../include/mpapp/handlers/linux/switch_handler.hpp) + [`src/handlers/linux/switch_handler.cpp`](../../../src/handlers/linux/switch_handler.cpp)
  - Android: [`include/mpapp/handlers/android/switch_handler.hpp`](../../../include/mpapp/handlers/android/switch_handler.hpp) + [`src/handlers/android/switch_handler.cpp`](../../../src/handlers/android/switch_handler.cpp)
- Tests: [`tests/mock_handlers/switch_test.cpp`](../../../tests/mock_handlers/switch_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
