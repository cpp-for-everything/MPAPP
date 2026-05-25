---
type: component
mauiHandler: "Editor"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/editor"
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

# Editor

> [!info] Status
> **3-of-5 platforms real** — `text` + `placeholder` (WinUI 3 + Android only — GTK4 `GtkTextView` has no placeholder concept) + `is_read_only` surface real on WinUI 3 (`mux::Controls::TextBox` with `AcceptsReturn=true, TextWrapping=Wrap`), GTK4 (`GtkTextView` + `GtkTextBuffer`), and Android (`android.widget.EditText` with `InputType = TYPE_CLASS_TEXT | TYPE_TEXT_FLAG_MULTI_LINE`, min 3 lines). Shares the bidirectional text-binding pipeline with Entry — the same `MppTextWatcher` Java listener routes Entry (kind=1) and Editor (kind=2) through a new shared `text_watcher_dispatch.cpp` JNI trampoline. Entry's reverse-binding chain is live-verified on Android; Editor uses the identical code path, just attached to a multi-line-configured EditText. macOS / iOS handlers code-complete pending an Apple host.

> [!info] Original status
> **mock** — full mock surface and handler land in `include/mpapp/handlers/mock/editor_handler.hpp` with tests in `tests/mock_handlers/editor_test.cpp`. Real platform handlers follow in P3.

## Overview

`Editor` is the multi-line text-input control. It accepts free-form text, wraps lines at the available width, supports vertical scrolling, and raises `Completed` when input is finalized (platform-specific — typically loss of focus or pressing return on hardware keyboards). It shares almost all of `Entry`'s `ITextInput` surface (placeholder, max-length, keyboard, read-only) but omits password masking and the single-line return-type / clear-button affordances.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_editor` | [`include/mpapp/internal/basic_editor.hpp`](../../../include/mpapp/internal/basic_editor.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::editor` | [`include/mpapp/editor.hpp`](../../../include/mpapp/editor.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/editor.hpp>

mpapp::editor w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/editor.hpp>
#include <mpapp/handlers/mock/editor_handler.hpp>

mpapp::internal::basic_editor w;
mpapp::editor_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::editor_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::editor_handler<>` and `mpapp::editor_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Editor\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Editor\`
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

## Mock implementation

- Handler: [`include/mpapp/handlers/mock/editor_handler.hpp`](../../../include/mpapp/handlers/mock/editor_handler.hpp)
- Tests: [`tests/mock_handlers/editor_test.cpp`](../../../tests/mock_handlers/editor_test.cpp)

`editor_handler<platform::mock>` records changes to the primitive Observable slots (`text`, `placeholder`, `is_read_only`, `max_length`). The mock surface omits password masking (per the Editor MAUI spec) and exercises multi-line text propagation.

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Implementation

- Surface: [`include/mpapp/editor.hpp`](../../../include/mpapp/editor.hpp)
- Mock handler: [`include/mpapp/handlers/mock/editor_handler.hpp`](../../../include/mpapp/handlers/mock/editor_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/editor_handler.hpp`](../../../include/mpapp/handlers/windows/editor_handler.hpp) + [`src/handlers/windows/editor_handler.cpp`](../../../src/handlers/windows/editor_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/editor_handler.hpp`](../../../include/mpapp/handlers/linux/editor_handler.hpp) + [`src/handlers/linux/editor_handler.cpp`](../../../src/handlers/linux/editor_handler.cpp)
  - Android: [`include/mpapp/handlers/android/editor_handler.hpp`](../../../include/mpapp/handlers/android/editor_handler.hpp) + [`src/handlers/android/editor_handler.cpp`](../../../src/handlers/android/editor_handler.cpp)
- Tests: [`tests/mock_handlers/editor_test.cpp`](../../../tests/mock_handlers/editor_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
