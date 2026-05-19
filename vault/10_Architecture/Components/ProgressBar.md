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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/progressbar/mock_test.cpp` (planned)
- Windows handler: `tests/components/progressbar/windows_test.cpp` (planned)
- Android handler: `tests/components/progressbar/android_test.cpp` (planned)
- Linux handler: `tests/components/progressbar/linux_test.cpp` (planned)
- macOS handler: `tests/components/progressbar/macos_test.cpp` (planned)
- iOS handler: `tests/components/progressbar/ios_test.cpp` (planned)

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Out-of-range values | Silently clamped via `coerceValue` | Clamped, plus a debug-build warning | Out-of-range almost always indicates a bug | TBD |
| `ProgressTo` return type | `Task<bool>` (true = completed) | `std::future<bool>` plus coroutine awaitable | Idiomatic C++; no managed `Task` to model | [[ADR-0009-public-api-template-wrappers-only]] |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ActivityIndicator]]
- [[View]]
