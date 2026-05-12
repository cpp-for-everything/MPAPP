---
type: component
mauiHandler: "Button"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/button"
mpappStatus: mock
platformWindows: true
platformAndroid: false
platformLinux: false
platformMacos: false
platformIos: false
tags:
  - type/component
  - status/mock
---

# Button

> [!info] Status
> **mock** — Windows handler validated by [[../50_Tasks/T-0003-winui3-button-spike/T-0003-winui3-button-spike]]; the rest of the API surface is mocked, with the full property/event set landing in M-03.

## Overview

`Button` is a tappable command surface that shows a text label and/or an image and raises a click event when activated. It is the canonical primary action control: every supported platform maps it to a native push-button widget so platform conventions for focus, ripple, hover, and accessibility carry over without re-implementation. Per MAUI's `IButton`, it composes text styling, padding, stroke, and image-source contracts.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Button\ButtonHandler.cs`
  - Windows: `ButtonHandler.Windows.cs` — wraps `Microsoft.UI.Xaml.Controls.Button` and registers `Click` as the trigger for `IButton.Clicked`. The MPAPP port mirrors this surface — see `src/handlers/windows/button_handler.cpp`.
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Button\`
- **Docs:** [Microsoft .NET MAUI — Button](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/button)

## MPAPP C++ API

### Mock surface (T-0003 — current)

What ships today in `include/mpapp/button.hpp`:

```cpp
namespace mpapp {

template <class Platform>
class button_handler; // specialised per platform (see Platform Notes).

class button : public control<button> {
public:
    Observable<std::string> text{""};

    // Cross-platform click event — fires on every native Click.
    mpapp::signal<>         clicked;

    // ADR-0009 binding hook (XAML `Command="…"` will detect this).
    void clicked_command(Command<> = {});

    // Access the platform-specific native widget via the handler.
    button_handler<platform::current>&       handler() noexcept;
    void set_handler(button_handler<platform::current>&) noexcept;
};

} // namespace mpapp
```

### Full surface (planned for M-03)

```cpp
namespace mpapp {

class button : public control<button> {
public:
    Observable<std::string>   text;
    Observable<color>         text_color;
    Observable<font>          font;
    Observable<double>        character_spacing;
    Observable<image_source>  image_source;
    Observable<thickness>     padding;
    Observable<color>         stroke_color;
    Observable<double>        stroke_thickness;
    Observable<double>        corner_radius;
    Observable<brush>         background;

    Command<>                 clicked;
    Command<>                 pressed;
    Command<>                 released;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Button Text="Save" Clicked="OnSaveClicked"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.Button` | C++/WinRT | Stroke maps to a contained `Border`. |
| Android | `Google.Android.Material.Button.MaterialButton` (AppCompat) | fbjni / JNI | Material 3 ripple, elevation honored. |
| Linux | `GtkButton` | GTK4 | Image-plus-text via `GtkBox` child. |
| macOS | `NSButton` (push-button style) | AppKit | Catalyst path falls back to `UIButton`. |
| iOS | `UIButton` (system style) | UIKit | Padding mapped to `contentEdgeInsets`. |

## Side-by-side Examples

### MAUI

```xml
<Button Text="Save"
        TextColor="White"
        BackgroundColor="DodgerBlue"
        Clicked="OnSaveClicked"/>
```

### MPAPP (XAML)

```xml
<Button Text="Save"
        TextColor="White"
        Background="DodgerBlue"
        Clicked="{Binding OnSaveClicked}"/>
```

### MPAPP (C++) — T-0003 spike-realised surface

The end-to-end flow validated by [[../50_Tasks/T-0003-winui3-button-spike/T-0003-winui3-button-spike]]:

```cpp
#include <mpapp/button.hpp>
#include <mpapp/label.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/handlers/windows/button_handler.hpp>
#include <mpapp/handlers/windows/label_handler.hpp>

mpapp::button                                    btn;
mpapp::label                                     lbl;
mpapp::button_handler<mpapp::platform::windows>  btn_handler;
mpapp::label_handler<mpapp::platform::windows>   lbl_handler;
mpapp::Observable<int>                           count{0};

btn.set_handler(btn_handler);
lbl.set_handler(lbl_handler);
btn.text = "Click me";
lbl.text = "Count: 0";

btn_handler.map_text(btn);     // pushes text into native + wires .changed
lbl_handler.map_text(lbl);     // ditto for the TextBlock
btn_handler.map_clicked(btn);  // native Click -> btn.clicked signal

mpapp::signal_slot<> click_slot;
auto click_cb = [&](){ count.set(count.get() + 1); };
btn.clicked.subscribe(click_slot, click_cb);

mpapp::signal_slot<const int&> count_slot;
auto count_cb = [&](int n){ lbl.text.set("Count: " + std::to_string(n)); };
count.changed.subscribe(count_slot, count_cb);

// Add `btn_handler.native()` and `lbl_handler.native()` to a WinUI
// StackPanel — see `examples/windows_button_spike/main.cpp`.
```

### MPAPP (C++) — full surface (planned for M-03)

```cpp
auto b = std::make_shared<mpapp::button>();
b->text = "Save";
b->text_color = colors::white;
b->background = brush{colors::dodger_blue};
b->clicked.subscribe([] { save(); });
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/button/mock_test.cpp` (planned)
- Windows handler: `tests/components/button/windows_test.cpp` (planned)
- Android handler: `tests/components/button/android_test.cpp` (planned)
- Linux handler: `tests/components/button/linux_test.cpp` (planned)
- macOS handler: `tests/components/button/macos_test.cpp` (planned)
- iOS handler: `tests/components/button/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
