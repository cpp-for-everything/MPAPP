---
type: component
mauiHandler: "Slider"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/slider"
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

# Slider

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Slider` is a horizontal continuous-range input: the user drags a thumb along a track between `Minimum` and `Maximum` to pick a `double` value. It exposes drag-start and drag-completed signals to distinguish committed selection from intermediate scrubbing — important when adjustments are expensive (audio scrubbing, brush size, network volume). Track segments before and after the thumb can be colored independently.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Slider\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Slider\`
- **Docs:** [Microsoft .NET MAUI — Slider](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/slider)

## MPAPP C++ API

```cpp
namespace mpapp {

class slider : public control<slider> {
public:
    Observable<double>        value;
    Observable<double>        minimum;
    Observable<double>        maximum;
    Observable<color>         minimum_track_color;
    Observable<color>         maximum_track_color;
    Observable<color>         thumb_color;
    Observable<image_source>  thumb_image_source;

    Command<double>           value_changed;
    Command<>                 drag_started;
    Command<>                 drag_completed;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Slider Minimum="0" Maximum="100" Value="50"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.Slider` | C++/WinRT | `IsMoveToPointEnabled=true`. |
| Android | `Android.Widget.SeekBar` | fbjni / JNI | Drag events derived from touch listener. |
| Linux | `GtkScale` | GTK4 | `draw-value=false`. |
| macOS | `NSSlider` (linear) | AppKit | `isContinuous=true`. |
| iOS | `UISlider` | UIKit | `minimumTrackTintColor`, `maximumTrackTintColor`, `thumbTintColor`. |

## Side-by-side Examples

### MAUI

```xml
<Slider Minimum="0"
        Maximum="100"
        Value="{Binding Volume}"
        ValueChanged="OnVolumeChanged"/>
```

### MPAPP (XAML)

```xml
<Slider Minimum="0"
        Maximum="100"
        Value="{Binding Volume}"
        ValueChanged="{Binding OnVolumeChanged}"/>
```

### MPAPP (C++)

```cpp
auto sl = std::make_shared<mpapp::slider>();
sl->minimum = 0.0;
sl->maximum = 100.0;
sl->value = 50.0;
sl->drag_completed.subscribe([&] { commit_volume(sl->value.get()); });
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/slider/mock_test.cpp` (planned)
- Windows handler: `tests/components/slider/windows_test.cpp` (planned)
- Android handler: `tests/components/slider/android_test.cpp` (planned)
- Linux handler: `tests/components/slider/linux_test.cpp` (planned)
- macOS handler: `tests/components/slider/macos_test.cpp` (planned)
- iOS handler: `tests/components/slider/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
