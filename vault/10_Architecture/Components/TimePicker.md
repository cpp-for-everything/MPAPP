---
type: component
mauiHandler: "TimePicker"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/timepicker"
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

# TimePicker

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`TimePicker` is the time-of-day counterpart to [[DatePicker]]. The selected wall-clock time is stored as a 24-hour duration (`time`), validated to `[00:00, 24:00)`. The format string controls the displayed text (default short time `"%X"`). The native chooser is a popup clock on Windows, a wheel on iOS, and a dialog on Android.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\TimePicker\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\TimePicker\`
- **Docs:** [Microsoft .NET MAUI — TimePicker](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/timepicker)

## MPAPP C++ API

```cpp
namespace mpapp {

class time_picker : public control<time_picker> {
public:
    // Selection (duration since midnight; validated to < 24h)
    Observable<std::chrono::minutes> time{std::chrono::minutes{0}};

    // Presentation
    Observable<std::string> format{"%X"};
    Observable<color>       text_color;
    Observable<font>        font;
    Observable<double>      character_spacing{0.0};

    // State
    Observable<bool> is_open{false};

    // Commands / events
    Command<std::chrono::minutes /*new*/> time_selected;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<TimePicker Time="{Binding AlarmAt, Mode=TwoWay}" Format="HH:mm"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.TimePicker` | C++/WinRT | Pull-down spinner. |
| Android | `MauiTimePicker` over `TimePickerDialog` | fbjni / JNI | 12h or 24h depending on locale. |
| Linux | Custom `GtkSpinButton` pair (h/m) | GTK4 | GTK lacks a stock time picker. |
| macOS | `NSDatePicker` (time mode) | AppKit | The same AppKit control configured for time only. |
| iOS | `UIDatePicker` (time mode, MacCatalyst uses the same) | UIKit | Wheel or compact inline. |

## Side-by-side Examples

### MAUI

```xml
<TimePicker Time="{Binding StartAt}" Format="t"/>
```

### MPAPP (XAML)

```xml
<TimePicker Time="{Binding StartAt}" Format="%H:%M"/>
```

### MPAPP (C++)

```cpp
auto tp = std::make_shared<mpapp::time_picker>();
tp->time = std::chrono::minutes{9 * 60 + 30}; // 09:30
tp->time_selected.subscribe([](auto m){
    std::cout << m.count() << " minutes\n";
});
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/timepicker/mock_test.cpp` (planned)
- Windows handler: `tests/components/timepicker/windows_test.cpp` (planned)
- Android handler: `tests/components/timepicker/android_test.cpp` (planned)
- Linux handler: `tests/components/timepicker/linux_test.cpp` (planned)
- macOS handler: `tests/components/timepicker/macos_test.cpp` (planned)
- iOS handler: `tests/components/timepicker/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Time type | `TimeSpan?` (validated `< 24h`) | `std::chrono::minutes` initially; `std::chrono::hh_mm_ss` once exposed | Avoid representing impossible durations per [[Type System]] | RFC TBD |
| Format syntax | .NET format specifiers (`"t"`, `"T"`) | `strftime`-style (`"%H:%M"`) | C++ standard library alignment | RFC TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[DatePicker]]
