---
type: component
mauiHandler: "ProgressBar"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/progressbar"
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

# ProgressBar

> [!info] Status
> **3-of-5 platforms real (compile-verified)** — `mpapp::progress_bar` with `progress: Observable<double>` (0..1) + `color` + `background_color`. WinUI 3 `mux::Controls::ProgressBar` (Min/Max 0..1, Value = progress; Foreground/Background SolidColorBrush). GTK4 `GtkProgressBar` (`gtk_progress_bar_set_fraction` + per-instance CSS provider targeting `trough > progress` and `trough` for color/background). Android `android.widget.ProgressBar` determinate (Max=10000, setProgress(progress*10000), setProgressTintList + setProgressBackgroundTintList). macOS / iOS planned in M-06.

## Overview

`ProgressBar` is a determinate horizontal bar that fills proportionally to a `Progress` value clamped to `[0.0, 1.0]`. Unlike [[ActivityIndicator]], it expresses *how much* work has been done — useful for downloads, installers, and any task with a known total. MAUI also exposes `ProgressTo(value, length, easing)` to animate the fill; MPAPP mirrors this as an explicit coroutine-returning method on the C++ class.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_progress_bar` | [`include/mpapp/internal/basic_progress_bar.hpp`](../../../include/mpapp/internal/basic_progress_bar.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::progress_bar` | [`include/mpapp/progress_bar.hpp`](../../../include/mpapp/progress_bar.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/progress_bar.hpp>

mpapp::progress_bar w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/progress_bar.hpp>
#include <mpapp/handlers/mock/progress_bar_handler.hpp>

mpapp::internal::basic_progress_bar w;
mpapp::progress_bar_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::progress_bar_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::progress_bar_handler<>` and `mpapp::progress_bar_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\ProgressBar\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\ProgressBar\`
- **Docs:** [Microsoft .NET MAUI — ProgressBar](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/progressbar)

## MPAPP C++ API

```cpp
namespace mpapp {

class progress_bar : public view<progress_bar> {
public:
    // Filled fraction, clamped to [0.0, 1.0] on assignment.
    Observable<double> progress { 0.0 };

    // Foreground/fill color of the bar.
    Observable<color> progress_color { color::accent() };

    // Animate Progress from its current value to `target` over `length_ms`.
    // Returns a future that resolves to `true` if the animation ran to completion.
    Command<std::future<bool>(double target, std::uint32_t length_ms, easing curve)>
        progress_to;
};

} // namespace mpapp
```

The `progress` observable applies clamping in its setter (matching MAUI's `coerceValue`). `progress_to` is the only command — there are no other verbs.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<ProgressBar Progress="{Binding DownloadFraction}"
             ProgressColor="LimeGreen" />
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.ProgressBar` | C++/WinRT | `Progress` is mapped onto `Value` after scaling from `[0,1]` to `[0,100]`. |
| Android | `android.widget.ProgressBar` (horizontal style) | fbjni / JNI | Two handler implementations exist in MAUI (`ProgressBarHandler` and `ProgressBarHandler2`) — MPAPP will adopt the newer one. |
| Linux | `GtkProgressBar` | GTK4 | `gtk_progress_bar_set_fraction`. |
| macOS | `NSProgressIndicator` (style `Bar`, determinate) | AppKit via [[Objective-Cpp]] | |
| iOS | `UIProgressView` | UIKit via [[Objective-Cpp]] | `FlowDirection` is explicitly mapped on iOS to flip the bar. |

## Side-by-side Examples

### MAUI

```xml
<ProgressBar Progress="0.5" ProgressColor="DodgerBlue" />
```

```csharp
await progressBar.ProgressTo(1.0, 1500, Easing.Linear);
```

### MPAPP (XAML)

```xml
<ProgressBar Progress="0.5" ProgressColor="DodgerBlue" />
```

### MPAPP (C++)

```cpp
auto bar = std::make_shared<mpapp::progress_bar>();
bar->progress = 0.5;
bar->progress_color = mpapp::color::from_hex("#1E90FF");

co_await bar->progress_to(1.0, 1500, mpapp::easing::linear);
```

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Out-of-range values | Silently clamped via `coerceValue` | Clamped, plus a debug-build warning | Out-of-range almost always indicates a bug | TBD |
| `ProgressTo` return type | `Task<bool>` (true = completed) | `std::future<bool>` plus coroutine awaitable | Idiomatic C++; no managed `Task` to model | [[ADR-0009-public-api-template-wrappers-only]] |

## Implementation

- Surface: [`include/mpapp/progress_bar.hpp`](../../../include/mpapp/progress_bar.hpp)
- Mock handler: [`include/mpapp/handlers/mock/progress_bar_handler.hpp`](../../../include/mpapp/handlers/mock/progress_bar_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/progress_bar_handler.hpp`](../../../include/mpapp/handlers/windows/progress_bar_handler.hpp) + [`src/handlers/windows/progress_bar_handler.cpp`](../../../src/handlers/windows/progress_bar_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/progress_bar_handler.hpp`](../../../include/mpapp/handlers/linux/progress_bar_handler.hpp) + [`src/handlers/linux/progress_bar_handler.cpp`](../../../src/handlers/linux/progress_bar_handler.cpp)
  - Android: [`include/mpapp/handlers/android/progress_bar_handler.hpp`](../../../include/mpapp/handlers/android/progress_bar_handler.hpp) + [`src/handlers/android/progress_bar_handler.cpp`](../../../src/handlers/android/progress_bar_handler.cpp)
- Tests: [`tests/mock_handlers/progress_bar_test.cpp`](../../../tests/mock_handlers/progress_bar_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ActivityIndicator]]
- [[View]]
