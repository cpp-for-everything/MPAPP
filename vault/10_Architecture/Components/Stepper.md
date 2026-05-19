---
type: component
mauiHandler: "Stepper"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/stepper"
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

# Stepper

> [!info] Status
> **3-of-5 platforms real** — `value` + `minimum` + `maximum` + `interval` (all `Observable<double>`) surface real on WinUI 3 (`mux::Controls::NumberBox` with inline spin buttons), GTK4 (`GtkSpinButton`), and Android (`android.widget.NumberPicker` with a new `MppNumberPickerListener` JNI bridge). Native widgets that use int values (NumberPicker) remap through `step_index = round((value - minimum) / interval)`, so user code stays in doubles and gets clean discrete steps. Shares the numeric-binding pattern proven live by Slider; Stepper's real handlers compile clean on all three platforms via the same template — bidirectional `value` flow is identical to Slider's verified path. macOS / iOS handlers code-complete pending an Apple host.

> [!info] Original status
> **mock** — full mock surface and handler land in `include/mpapp/handlers/mock/stepper_handler.hpp` with tests in `tests/mock_handlers/stepper_test.cpp`. Real platform handlers follow in P3.

## Overview

`Stepper` is a pair of plus/minus buttons used to increment or decrement a numeric `Value` by a fixed `Interval` within `[Minimum, Maximum]`. It is the discrete counterpart of `Slider` and is preferred when the legal range is small and precise (quantity, page-size, font-step). Only iOS has a true native stepper (`UIStepper`); MAUI synthesizes `MauiStepper` on the other platforms by stacking two buttons.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Stepper\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Stepper\`
- **Docs:** [Microsoft .NET MAUI — Stepper](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/stepper)

## MPAPP C++ API

```cpp
namespace mpapp {

class stepper : public control<stepper> {
public:
    Observable<double>  value;
    Observable<double>  minimum;
    Observable<double>  maximum;
    Observable<double>  interval;

    Command<double>     value_changed;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Stepper Minimum="0" Maximum="10" Increment="1" Value="1"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `MauiStepper` (two `Microsoft.UI.Xaml.Controls.Button`) | C++/WinRT | Composite — no native stepper in WinUI. |
| Android | `MauiStepper` (two `Android.Widget.Button`) | fbjni / JNI | Auto-repeat on long press. |
| Linux | `MauiStepper` (two `GtkButton`) | GTK4 | Composite for parity with platforms that lack a native stepper. |
| macOS | `NSStepper` or composite | AppKit | Catalyst falls back to `UIStepper`. |
| iOS | `UIStepper` | UIKit | Native two-segment control. |

## Side-by-side Examples

### MAUI

```xml
<Stepper Minimum="1"
         Maximum="99"
         Increment="1"
         Value="{Binding Quantity}"
         ValueChanged="OnQuantityChanged"/>
```

### MPAPP (XAML)

```xml
<Stepper Minimum="1"
         Maximum="99"
         Increment="1"
         Value="{Binding Quantity}"
         ValueChanged="{Binding OnQuantityChanged}"/>
```

### MPAPP (C++)

```cpp
auto st = std::make_shared<mpapp::stepper>();
st->minimum = 1.0;
st->maximum = 99.0;
st->interval = 1.0;
st->value_changed.subscribe([](double v) { set_quantity(v); });
```

## Mock implementation

- Handler: [`include/mpapp/handlers/mock/stepper_handler.hpp`](../../../include/mpapp/handlers/mock/stepper_handler.hpp)
- Tests: [`tests/mock_handlers/stepper_test.cpp`](../../../tests/mock_handlers/stepper_test.cpp)

`stepper_handler<platform::mock>` records changes to `value`, `minimum`, `maximum`, and `interval`. Tests cover initial value capture, increments, and the no-emit-on-same-value contract.

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/mock_handlers/stepper_test.cpp`
- Windows handler: `tests/components/stepper/windows_test.cpp` (planned)
- Android handler: `tests/components/stepper/android_test.cpp` (planned)
- Linux handler: `tests/components/stepper/linux_test.cpp` (planned)
- macOS handler: `tests/components/stepper/macos_test.cpp` (planned)
- iOS handler: `tests/components/stepper/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
