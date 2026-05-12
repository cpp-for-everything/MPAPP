---
type: component
mauiHandler: "Editor"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/editor"
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

# Editor

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Editor` is the multi-line text-input control. It accepts free-form text, wraps lines at the available width, supports vertical scrolling, and raises `Completed` when input is finalized (platform-specific — typically loss of focus or pressing return on hardware keyboards). It shares almost all of `Entry`'s `ITextInput` surface (placeholder, max-length, keyboard, read-only) but omits password masking and the single-line return-type / clear-button affordances.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\Editor\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\Editor\`
- **Docs:** [Microsoft .NET MAUI — Editor](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/editor)

## MPAPP C++ API

```cpp
namespace mpapp {

class editor : public control<editor> {
public:
    Observable<std::string>     text;
    Observable<std::string>     placeholder;
    Observable<color>           text_color;
    Observable<color>           placeholder_color;
    Observable<font>            font;
    Observable<double>          character_spacing;
    Observable<bool>            is_read_only;
    Observable<bool>            is_spell_check_enabled;
    Observable<bool>            is_text_prediction_enabled;
    Observable<int>             max_length;
    Observable<int>             cursor_position;
    Observable<int>             selection_length;
    Observable<keyboard>        keyboard;
    Observable<text_alignment>  horizontal_text_alignment;
    Observable<text_alignment>  vertical_text_alignment;

    Command<>                   completed;
    Command<std::string>        text_changed;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Editor Placeholder="Notes" AutoSize="TextChanges"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.TextBox` (with `AcceptsReturn=true`) | C++/WinRT | Vertical scroll auto-enabled. |
| Android | `AndroidX.AppCompat.Widget.AppCompatEditText` (with `inputType` multiline) | fbjni / JNI | `singleLine=false`. |
| Linux | `GtkTextView` inside `GtkScrolledWindow` | GTK4 | `GtkTextBuffer` exposes text. |
| macOS | `NSTextView` inside `NSScrollView` | AppKit | Catalyst uses `MauiTextView`. |
| iOS | `MauiTextView` (subclass of `UITextView`) | UIKit | Adds placeholder support. |

## Side-by-side Examples

### MAUI

```xml
<Editor Placeholder="Write your review..."
        Text="{Binding Review}"
        MaxLength="500"
        AutoSize="TextChanges"/>
```

### MPAPP (XAML)

```xml
<Editor Placeholder="Write your review..."
        Text="{Binding Review}"
        MaxLength="500"
        AutoSize="TextChanges"/>
```

### MPAPP (C++)

```cpp
auto ed = std::make_shared<mpapp::editor>();
ed->placeholder = "Write your review...";
ed->max_length = 500;
ed->text_changed.subscribe([](auto const& s) { word_count(s); });
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/editor/mock_test.cpp` (planned)
- Windows handler: `tests/components/editor/windows_test.cpp` (planned)
- Android handler: `tests/components/editor/android_test.cpp` (planned)
- Linux handler: `tests/components/editor/linux_test.cpp` (planned)
- macOS handler: `tests/components/editor/macos_test.cpp` (planned)
- iOS handler: `tests/components/editor/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
