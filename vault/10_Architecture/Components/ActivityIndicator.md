---
type: component
mauiHandler: "ActivityIndicator"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/activityindicator"
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

# ActivityIndicator

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`ActivityIndicator` is a visual cue that "something is happening" without conveying progress — i.e., an indeterminate spinner. It is animated while `IsRunning` is `true` and is collapsed/hidden when `false` (the [[Handler]] couples `Visibility` and `IsRunning` on iOS and Android because the native controls do not respect both flags independently). It exposes exactly two bindable properties — `IsRunning` and `Color` — and inherits the rest of its surface from [[View]].

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\ActivityIndicator\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\ActivityIndicator\`
- **Docs:** [Microsoft .NET MAUI — ActivityIndicator](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/activityindicator)

## MPAPP C++ API

```cpp
namespace mpapp {

class activity_indicator : public view<activity_indicator> {
public:
    // Whether the indicator is animating. Default: false.
    Observable<bool> is_running { false };

    // Tint of the spinner glyph; default falls back to the platform accent.
    Observable<color> color { color::accent() };
};

} // namespace mpapp
```

There are no commands — `ActivityIndicator` has no user-invocable verbs. Use a `Computed<bool>` bound to `is_running` to mirror a "busy" flag from a view model.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<ActivityIndicator IsRunning="{Binding IsBusy}"
                   Color="DodgerBlue" />
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.ProgressRing` | C++/WinRT | Width/Height/Background re-mapped because the ring respects its own template. |
| Android | `android.widget.ProgressBar` (indeterminate) | fbjni / JNI | `Visibility` is coalesced into `IsRunning` — a stopped indicator is GONE, not just invisible. |
| Linux | `GtkSpinner` | GTK4 | `gtk_spinner_start` / `gtk_spinner_stop` drive `IsRunning`. |
| macOS | `NSProgressIndicator` (style `Spinning`) | AppKit via [[Objective-Cpp]] | `startAnimation:` / `stopAnimation:`. |
| iOS | `UIActivityIndicatorView` | UIKit via [[Objective-Cpp]] | `MauiActivityIndicator` wraps it to expose color via `color`. |

## Side-by-side Examples

### MAUI

```xml
<ActivityIndicator IsRunning="True" Color="Orange" />
```

### MPAPP (XAML)

```xml
<ActivityIndicator IsRunning="True" Color="Orange" />
```

### MPAPP (C++)

```cpp
auto spinner = std::make_shared<mpapp::activity_indicator>();
spinner->is_running = true;
spinner->color = mpapp::color::from_hex("#FFA500");
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/activityindicator/mock_test.cpp` (planned)
- Windows handler: `tests/components/activityindicator/windows_test.cpp` (planned)
- Android handler: `tests/components/activityindicator/android_test.cpp` (planned)
- Linux handler: `tests/components/activityindicator/linux_test.cpp` (planned)
- macOS handler: `tests/components/activityindicator/macos_test.cpp` (planned)
- iOS handler: `tests/components/activityindicator/ios_test.cpp` (planned)

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Default color | Platform accent (varies) | `color::accent()` template, resolved at handler-attach time | Lets a single observable bind across themes without re-running XAML | RFC tracking theme tokens (not yet written) |
| `IsRunning` ↔ `Visibility` coupling | Implicit on Android/iOS, explicit on Windows | Always explicit through the handler — collapsing is opt-in | Avoid the "why did my spinner disappear" footgun | TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ProgressBar]]
- [[View]]
