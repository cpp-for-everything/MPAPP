---
type: component
mauiHandler: "DatePicker"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/datepicker"
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

# DatePicker

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`DatePicker` lets the user choose a single calendar day. The selected day is exposed via `date`, bounded by `minimum_date` and `maximum_date`. The displayed string is formatted using the `format` property (analogous to `DateTime.ToString` format strings in .NET). The native chooser appears as a calendar popup on Windows, a wheel on iOS, and a dialog on Android.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\DatePicker\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\DatePicker\`
- **Docs:** [Microsoft .NET MAUI — DatePicker](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/datepicker)

## MPAPP C++ API

```cpp
namespace mpapp {

class date_picker : public control<date_picker> {
public:
    // Selection
    Observable<std::chrono::year_month_day> date;
    Observable<std::chrono::year_month_day> minimum_date{ std::chrono::January / 1 / 1900y };
    Observable<std::chrono::year_month_day> maximum_date{ std::chrono::December / 31 / 2100y };

    // Presentation
    Observable<std::string> format{"%x"};
    Observable<color>       text_color;
    Observable<font>        font;
    Observable<double>      character_spacing{0.0};

    // State
    Observable<bool> is_open{false};

    // Commands / events
    Command<std::chrono::year_month_day /*new*/> date_selected;
};

} // namespace mpapp
```

`std::chrono::year_month_day` is the C++20 calendar type; see [[Observable Properties]] for binding semantics.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<DatePicker Date="{Binding BirthDate, Mode=TwoWay}"
            MinimumDate="1950-01-01"
            MaximumDate="2050-12-31"
            Format="yyyy-MM-dd"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.CalendarDatePicker` | C++/WinRT | Calendar flyout. |
| Android | `MauiDatePicker` over `DatePickerDialog` | fbjni / JNI | Dialog launched on focus. |
| Linux | `GtkCalendar` in a popover | GTK4 | Mirrors Windows look. |
| macOS | `NSDatePicker` (graphical style) | AppKit | Popover-anchored. |
| iOS | `UIDatePicker` (date mode) | UIKit | Wheel or compact inline depending on iOS version. |

## Side-by-side Examples

### MAUI

```xml
<DatePicker Date="{Binding BirthDate}" Format="D"/>
```

### MPAPP (XAML)

```xml
<DatePicker Date="{Binding BirthDate}" Format="%A, %B %d, %Y"/>
```

### MPAPP (C++)

```cpp
auto dp = std::make_shared<mpapp::date_picker>();
dp->minimum_date = std::chrono::January / 1 / 2000y;
dp->date_selected.subscribe([](auto d){
    std::cout << static_cast<int>(d.year()) << "\n";
});
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/datepicker/mock_test.cpp` (planned)
- Windows handler: `tests/components/datepicker/windows_test.cpp` (planned)
- Android handler: `tests/components/datepicker/android_test.cpp` (planned)
- Linux handler: `tests/components/datepicker/linux_test.cpp` (planned)
- macOS handler: `tests/components/datepicker/macos_test.cpp` (planned)
- iOS handler: `tests/components/datepicker/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Date type | `DateTime?` (with time component) | `std::chrono::year_month_day` (date only) | Stronger type per [[Type System]]; matches the control's domain | RFC TBD |
| Format syntax | .NET format specifiers (`"d"`, `"D"`) | `std::format` / `strftime` style (`"%x"`, `"%Y-%m-%d"`) | C++ standard library alignment | RFC TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[TimePicker]]
