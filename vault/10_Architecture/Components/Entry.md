---
type: component
mauiHandler: "Entry"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/entry"
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

# Entry

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/entry/mock_test.cpp` (planned)
- Windows handler: `tests/components/entry/windows_test.cpp` (planned)
- Android handler: `tests/components/entry/android_test.cpp` (planned)
- Linux handler: `tests/components/entry/linux_test.cpp` (planned)
- macOS handler: `tests/components/entry/macos_test.cpp` (planned)
- iOS handler: `tests/components/entry/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
