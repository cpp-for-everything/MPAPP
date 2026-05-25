---
type: component
mauiHandler: "Slider"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/slider"
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

# Slider

> [!info] Status
> **3-of-5 platforms real** — `value: Observable<double>` + `minimum: Observable<double>` + `maximum: Observable<double>` surface real on WinUI 3 (`mux::Controls::Slider` + `ValueChanged` RangeBase event), GTK4 (`GtkScale` + `value-changed` GSignal), and Android (`android.widget.SeekBar` + a new `MppSeekBarChangeListener` JNI bridge). **Bidirectional numeric binding** end-to-end live-verified on Android: dragging the SeekBar emits int progress in [0, 10000] which the native handler remaps to the cross-platform [minimum, maximum] double range; user observers re-render the label LIVE. Screenshots: `android-slider-initial.png` (slider at min, value=1, label says "hello, world" once); `android-slider-dragged.png` (slider at ~80%, value=4, label repeats "hello, world · hello, world · hello, world · hello, world"). macOS / iOS handlers code-complete pending an Apple host.

> [!info] Original status
> **mock** — full mock surface and handler land in `include/mpapp/handlers/mock/slider_handler.hpp` with tests in `tests/mock_handlers/slider_test.cpp`. Real platform handlers follow in P3.

## Overview

`Slider` is a horizontal continuous-range input: the user drags a thumb along a track between `Minimum` and `Maximum` to pick a `double` value. It exposes drag-start and drag-completed signals to distinguish committed selection from intermediate scrubbing — important when adjustments are expensive (audio scrubbing, brush size, network volume). Track segments before and after the thumb can be colored independently.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_slider` | [`include/mpapp/internal/basic_slider.hpp`](../../../include/mpapp/internal/basic_slider.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::slider` | [`include/mpapp/slider.hpp`](../../../include/mpapp/slider.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/slider.hpp>

mpapp::slider w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/slider.hpp>
#include <mpapp/handlers/mock/slider_handler.hpp>

mpapp::internal::basic_slider w;
mpapp::slider_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::slider_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::slider_handler<>` and `mpapp::slider_handler<platform::mock>` valid spellings without naming `internal::`.

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

## Mock implementation

- Handler: [`include/mpapp/handlers/mock/slider_handler.hpp`](../../../include/mpapp/handlers/mock/slider_handler.hpp)
- Tests: [`tests/mock_handlers/slider_test.cpp`](../../../tests/mock_handlers/slider_test.cpp)

`slider_handler<platform::mock>` records changes to `value`, `minimum`, and `maximum`. Tests cover the initial-range capture, per-channel propagation, and the no-emit-on-same-value contract.

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Implementation

- Surface: [`include/mpapp/slider.hpp`](../../../include/mpapp/slider.hpp)
- Mock handler: [`include/mpapp/handlers/mock/slider_handler.hpp`](../../../include/mpapp/handlers/mock/slider_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/slider_handler.hpp`](../../../include/mpapp/handlers/windows/slider_handler.hpp) + [`src/handlers/windows/slider_handler.cpp`](../../../src/handlers/windows/slider_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/slider_handler.hpp`](../../../include/mpapp/handlers/linux/slider_handler.hpp) + [`src/handlers/linux/slider_handler.cpp`](../../../src/handlers/linux/slider_handler.cpp)
  - Android: [`include/mpapp/handlers/android/slider_handler.hpp`](../../../include/mpapp/handlers/android/slider_handler.hpp) + [`src/handlers/android/slider_handler.cpp`](../../../src/handlers/android/slider_handler.cpp)
- Tests: [`tests/mock_handlers/slider_test.cpp`](../../../tests/mock_handlers/slider_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
