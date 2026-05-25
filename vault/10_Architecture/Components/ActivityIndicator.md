---
type: component
mauiHandler: "ActivityIndicator"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/activityindicator"
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

# ActivityIndicator

> [!info] Status
> **3-of-5 platforms real (compile-verified)** — `mpapp::activity_indicator` with `is_running: Observable<bool>` + `color: Observable<brush_ref>`. WinUI 3 wraps `mux::Controls::ProgressRing` (IsActive + Foreground + Visibility); GTK4 wraps `GtkSpinner` (gtk_spinner_start/stop + gtk_widget_set_visible + per-instance CSS provider for `color`); Android wraps `android.widget.ProgressBar` in its default indeterminate style (setVisibility VISIBLE/GONE for is_running + setIndeterminateTintList(ColorStateList) for color). Mock handler + tests cover the bind/change protocol. macOS / iOS handlers planned in M-06.

## Overview

`ActivityIndicator` is a visual cue that "something is happening" without conveying progress — i.e., an indeterminate spinner. It is animated while `IsRunning` is `true` and is collapsed/hidden when `false` (the [[Handler]] couples `Visibility` and `IsRunning` on iOS and Android because the native controls do not respect both flags independently). It exposes exactly two bindable properties — `IsRunning` and `Color` — and inherits the rest of its surface from [[View]].


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_activity_indicator` | [`include/mpapp/internal/basic_activity_indicator.hpp`](../../../include/mpapp/internal/basic_activity_indicator.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::activity_indicator` | [`include/mpapp/activity_indicator.hpp`](../../../include/mpapp/activity_indicator.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/activity_indicator.hpp>

mpapp::activity_indicator w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/activity_indicator.hpp>
#include <mpapp/handlers/mock/activity_indicator_handler.hpp>

mpapp::internal::basic_activity_indicator w;
mpapp::activity_indicator_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::activity_indicator_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::activity_indicator_handler<>` and `mpapp::activity_indicator_handler<platform::mock>` valid spellings without naming `internal::`.

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

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Default color | Platform accent (varies) | `color::accent()` template, resolved at handler-attach time | Lets a single observable bind across themes without re-running XAML | RFC tracking theme tokens (not yet written) |
| `IsRunning` ↔ `Visibility` coupling | Implicit on Android/iOS, explicit on Windows | Always explicit through the handler — collapsing is opt-in | Avoid the "why did my spinner disappear" footgun | TBD |

## Implementation

- Surface: [`include/mpapp/activity_indicator.hpp`](../../../include/mpapp/activity_indicator.hpp)
- Mock handler: [`include/mpapp/handlers/mock/activity_indicator_handler.hpp`](../../../include/mpapp/handlers/mock/activity_indicator_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/activity_indicator_handler.hpp`](../../../include/mpapp/handlers/windows/activity_indicator_handler.hpp) + [`src/handlers/windows/activity_indicator_handler.cpp`](../../../src/handlers/windows/activity_indicator_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/activity_indicator_handler.hpp`](../../../include/mpapp/handlers/linux/activity_indicator_handler.hpp) + [`src/handlers/linux/activity_indicator_handler.cpp`](../../../src/handlers/linux/activity_indicator_handler.cpp)
  - Android: [`include/mpapp/handlers/android/activity_indicator_handler.hpp`](../../../include/mpapp/handlers/android/activity_indicator_handler.hpp) + [`src/handlers/android/activity_indicator_handler.cpp`](../../../src/handlers/android/activity_indicator_handler.cpp)
- Tests: [`tests/mock_handlers/activity_indicator_test.cpp`](../../../tests/mock_handlers/activity_indicator_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ProgressBar]]
- [[View]]
