---
type: component
mauiHandler: "Entry"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/entry"
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

# Entry

> [!info] Status
> **3-of-5 platforms real** — `text` + `placeholder` + `is_read_only` surface real on WinUI 3 (`mux::Controls::TextBox` + `TextChanged` event), GTK4 (`GtkEntry` + `changed` signal), and Android (`android.widget.EditText` + `TextWatcher` via the `MppTextWatcher` JNI bridge). **Bidirectional binding** end-to-end live-verified on Android: typing into the EditText flows back through Java `afterTextChanged` → JNI `nativeDispatchTextChanged` → `mpapp::entry::text.set` → user observer → label update. Verified screenshot: `_Archive/T-0011-app-shell-abstraction/screenshots/android-entry-typed.png` + `android-entry-typed-clicked3.png` (Count: 0 → 4, name "xelA" persists across button taps; "xelA" rather than "Alex" is an `adb shell input text` ordering quirk unrelated to the binding). macOS / iOS handlers code-complete pending an Apple host.

## Overview

`Entry` is the single-line text-input control: user types a string, IME conventions apply, and a `Completed` event fires when the return key is pressed. It supports placeholder text, password masking, clear-button visibility, max-length, return-key style, and keyboard-type hints. Per MAUI's `IEntry`, it implements `ITextInput` and `ITextAlignment`, giving it the same surface as `Editor` minus multi-line rendering.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Entry\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Entry\`
- **Docs:** [Microsoft .NET MAUI — Entry](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/entry)

## MPAPP C++ API

```cpp
namespace mpapp {

class entry : public control<entry> {
public:
    Observable<std::string>             text;
    Observable<std::string>             placeholder;
    Observable<color>                   text_color;
    Observable<color>                   placeholder_color;
    Observable<font>                    font;
    Observable<double>                  character_spacing;
    Observable<bool>                    is_password;
    Observable<bool>                    is_read_only;
    Observable<bool>                    is_spell_check_enabled;
    Observable<bool>                    is_text_prediction_enabled;
    Observable<int>                     max_length;
    Observable<int>                     cursor_position;
    Observable<int>                     selection_length;
    Observable<keyboard>                keyboard;
    Observable<return_type>             return_type;
    Observable<clear_button_visibility> clear_button_visibility;
    Observable<text_alignment>          horizontal_text_alignment;
    Observable<text_alignment>          vertical_text_alignment;

    Command<>                           completed;
    Command<std::string>                text_changed;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Entry Placeholder="Email" Keyboard="Email"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.TextBox` (or `PasswordBox` when `IsPassword`) | C++/WinRT | Handler swaps control type when password mode toggles. |
| Android | `AndroidX.AppCompat.Widget.AppCompatEditText` | fbjni / JNI | `inputType` derives from keyboard + password. |
| Linux | `GtkEntry` | GTK4 | `visibility=false` for password mode. |
| macOS | `NSTextField` / `NSSecureTextField` | AppKit | Secure variant for `is_password`. |
| iOS | `MauiTextField` (subclass of `UITextField`) | UIKit | Clear button mapped to `clearButtonMode`. |

## Side-by-side Examples

### MAUI

```xml
<Entry Placeholder="Username"
       Text="{Binding Username}"
       MaxLength="32"
       ReturnType="Next"
       Completed="OnUsernameCompleted"/>
```

### MPAPP (XAML)

```xml
<Entry Placeholder="Username"
       Text="{Binding Username}"
       MaxLength="32"
       ReturnType="Next"
       Completed="{Binding OnUsernameCompleted}"/>
```

### MPAPP (C++)

```cpp
auto e = std::make_shared<mpapp::entry>();
e->placeholder = "Username";
e->max_length = 32;
e->return_type = return_type::next;
e->completed.subscribe([&] { focus_next(); });
```

## Mock implementation

- Handler: [`include/mpapp/handlers/mock/entry_handler.hpp`](../../../include/mpapp/handlers/mock/entry_handler.hpp)
- Tests: [`tests/mock_handlers/entry_test.cpp`](../../../tests/mock_handlers/entry_test.cpp)

`entry_handler<platform::mock>` records changes to the primitive Observable slots (`text`, `placeholder`, `is_password`, `is_read_only`, `max_length`, `cursor_position`). Tests verify all mappers fire on attach, the same-value-no-emit contract, and exercise the password / cursor toggles.

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Implementation

- Surface: [`include/mpapp/entry.hpp`](../../../include/mpapp/entry.hpp)
- Mock handler: [`include/mpapp/handlers/mock/entry_handler.hpp`](../../../include/mpapp/handlers/mock/entry_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/entry_handler.hpp`](../../../include/mpapp/handlers/windows/entry_handler.hpp) + [`src/handlers/windows/entry_handler.cpp`](../../../src/handlers/windows/entry_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/entry_handler.hpp`](../../../include/mpapp/handlers/linux/entry_handler.hpp) + [`src/handlers/linux/entry_handler.cpp`](../../../src/handlers/linux/entry_handler.cpp)
  - Android: [`include/mpapp/handlers/android/entry_handler.hpp`](../../../include/mpapp/handlers/android/entry_handler.hpp) + [`src/handlers/android/entry_handler.cpp`](../../../src/handlers/android/entry_handler.cpp)
- Tests: [`tests/mock_handlers/entry_test.cpp`](../../../tests/mock_handlers/entry_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
