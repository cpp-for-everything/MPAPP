---
type: component
mauiHandler: "CheckBox"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/checkbox"
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

# CheckBox

> [!info] Status
> **3-of-5 platforms real** — `is_checked: Observable<bool>` surface real on WinUI 3 (`mux::Controls::CheckBox` + `Checked`/`Unchecked` events), GTK4 (`GtkCheckButton` + `toggled` signal), and Android (`android.widget.CheckBox` + shared `MppCheckedChangeListener` JNI bridge with `kind=2` discriminator). **Bidirectional binding** end-to-end live-verified on Android: tapping the checkbox flips a cross-platform `Observable<bool>` whose user-side observer appends "!!!" to the label. Verified screenshots: `_Archive/T-0011-app-shell-abstraction/screenshots/android-checkbox-on.png` (Count: 0 — hello, world!!!); `android-checkbox-switch-clicks3.png` (Count: 3 — HELLO, WORLD!!! after CheckBox + Switch both on and three button clicks — confirms multiple Observables compose). macOS / iOS handlers code-complete pending an Apple host.

## Overview

`CheckBox` is a two-state binary selector — `IsChecked` is true or false — typically used in forms where each item is independently opt-in. Unlike `Switch`, the value is conventionally committed only when the surrounding form is submitted. iOS has no native check-box widget, so MAUI ships a custom `MauiCheckBox` drawn with `CoreGraphics`; MPAPP follows the same approach.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_check_box` | [`include/mpapp/internal/basic_check_box.hpp`](../../../include/mpapp/internal/basic_check_box.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::check_box` | [`include/mpapp/check_box.hpp`](../../../include/mpapp/check_box.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/check_box.hpp>

mpapp::check_box w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/check_box.hpp>
#include <mpapp/handlers/mock/check_box_handler.hpp>

mpapp::internal::basic_check_box w;
mpapp::check_box_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::check_box_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::check_box_handler<>` and `mpapp::check_box_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\CheckBox\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\CheckBox\`
- **Docs:** [Microsoft .NET MAUI — CheckBox](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/checkbox)

## MPAPP C++ API

```cpp
namespace mpapp {

class check_box : public control<check_box> {
public:
    Observable<bool>   is_checked;
    Observable<paint>  foreground;

    Command<bool>      checked_changed;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<CheckBox IsChecked="True"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.CheckBox` | C++/WinRT | Indeterminate state not surfaced. |
| Android | `AndroidX.AppCompat.Widget.AppCompatCheckBox` | fbjni / JNI | Material check mark. |
| Linux | `GtkCheckButton` | GTK4 | Label child omitted (separate `Label` used). |
| macOS | `NSButton` with `setButtonType: NSSwitchButton` | AppKit | Catalyst uses `MauiCheckBox`. |
| iOS | `MauiCheckBox` (custom `UIView`) | UIKit | Drawn with `CoreGraphics` — no `UICheckBox` exists. |

## Side-by-side Examples

### MAUI

```xml
<CheckBox IsChecked="{Binding AcceptTerms}"
          Color="DodgerBlue"
          CheckedChanged="OnAcceptTermsChanged"/>
```

### MPAPP (XAML)

```xml
<CheckBox IsChecked="{Binding AcceptTerms}"
          Color="DodgerBlue"
          CheckedChanged="{Binding OnAcceptTermsChanged}"/>
```

### MPAPP (C++)

```cpp
auto cb = std::make_shared<mpapp::check_box>();
cb->is_checked = false;
cb->foreground = solid_color_paint{colors::dodger_blue};
cb->checked_changed.subscribe([](bool v) { set_accept_terms(v); });
```

## Mock implementation

- Handler: [`include/mpapp/handlers/mock/check_box_handler.hpp`](../../../include/mpapp/handlers/mock/check_box_handler.hpp)
- Tests: [`tests/mock_handlers/check_box_test.cpp`](../../../tests/mock_handlers/check_box_test.cpp)

`check_box_handler<platform::mock>` records `is_checked=<bool>` into `calls()`; tests cover initial-value capture, toggling, and the no-emit-on-same-value contract.

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Implementation

- Surface: [`include/mpapp/check_box.hpp`](../../../include/mpapp/check_box.hpp)
- Mock handler: [`include/mpapp/handlers/mock/check_box_handler.hpp`](../../../include/mpapp/handlers/mock/check_box_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/check_box_handler.hpp`](../../../include/mpapp/handlers/windows/check_box_handler.hpp) + [`src/handlers/windows/check_box_handler.cpp`](../../../src/handlers/windows/check_box_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/check_box_handler.hpp`](../../../include/mpapp/handlers/linux/check_box_handler.hpp) + [`src/handlers/linux/check_box_handler.cpp`](../../../src/handlers/linux/check_box_handler.cpp)
  - Android: [`include/mpapp/handlers/android/check_box_handler.hpp`](../../../include/mpapp/handlers/android/check_box_handler.hpp) + [`src/handlers/android/check_box_handler.cpp`](../../../src/handlers/android/check_box_handler.cpp)
- Tests: [`tests/mock_handlers/check_box_test.cpp`](../../../tests/mock_handlers/check_box_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
