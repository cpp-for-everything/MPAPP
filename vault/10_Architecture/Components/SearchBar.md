---
type: component
mauiHandler: "SearchBar"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/searchbar"
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

# SearchBar

> [!info] Status
> **3-of-5 platforms real (compile-verified)** — `mpapp::search_bar` with `text` + `placeholder`. Windows wraps `mux::Controls::AutoSuggestBox` (with `QueryIcon = SymbolIcon{Find}` for search affordance). Linux wraps `GtkSearchEntry`. Android wraps `android.widget.SearchView` (`setIconified(false)` so the field opens immediately, `setQuery` for text, `setQueryHint` for placeholder). macOS / iOS planned in M-06.

## Overview

`SearchBar` is a specialised single-line text input optimised for queries. It carries a search glyph, a placeholder, and (on most platforms) a clear/cancel button. Submitting (tapping the keyboard's search action) raises `search_command` with the current text. It is conceptually `Entry` + "search" `return_type` + cancel affordance, and uses `AutoSuggestBox`/`UISearchBar`/`SearchView` natively.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_search_bar` | [`include/mpapp/internal/basic_search_bar.hpp`](../../../include/mpapp/internal/basic_search_bar.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::search_bar` | [`include/mpapp/search_bar.hpp`](../../../include/mpapp/search_bar.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/search_bar.hpp>

mpapp::search_bar w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/search_bar.hpp>
#include <mpapp/handlers/mock/search_bar_handler.hpp>

mpapp::internal::basic_search_bar w;
mpapp::search_bar_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::search_bar_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::search_bar_handler<>` and `mpapp::search_bar_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\SearchBar\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\SearchBar\`
- **Docs:** [Microsoft .NET MAUI — SearchBar](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/searchbar)

## MPAPP C++ API

```cpp
namespace mpapp {

class search_bar : public control<search_bar> {
public:
    // Text
    Observable<std::string> text;
    Observable<std::string> placeholder;
    Observable<color>       placeholder_color;
    Observable<color>       text_color;
    Observable<color>       cancel_button_color;
    Observable<color>       search_icon_color;

    // Typing behaviour
    Observable<bool>   is_read_only{false};
    Observable<bool>   is_text_prediction_enabled{true};
    Observable<bool>   is_spell_check_enabled{true};
    Observable<int>    max_length{-1};
    Observable<int>    cursor_position{0};
    Observable<int>    selection_length{0};
    Observable<keyboard_kind> keyboard{keyboard_kind::text};
    Observable<return_kind>   return_type{return_kind::search};

    // Layout
    Observable<font>           font;
    Observable<double>         character_spacing{0.0};
    Observable<text_alignment> horizontal_text_alignment{text_alignment::start};
    Observable<text_alignment> vertical_text_alignment{text_alignment::center};

    // Commands
    Command<std::string /*query*/> search_command;
    Command<std::string /*query*/> text_changed;
    Command<>                      focus;
    Command<>                      unfocus;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<SearchBar Placeholder="Find..."
           Text="{Binding Query, Mode=TwoWay}"
           SearchCommand="{Binding RunSearch}"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.AutoSuggestBox` | C++/WinRT | `QueryEditor` is the box itself. |
| Android | `androidx.appcompat.widget.SearchView` | fbjni / JNI | `QueryEditor` is the inner `EditText`. |
| Linux | `GtkSearchEntry` | GTK4 | Built-in clear button. |
| macOS | `NSSearchField` | AppKit | Round-bezel field, sends `searchAction:`. |
| iOS | `MauiSearchBar` over `UISearchBar` | UIKit | `QueryEditor` is the inner `UITextField`. |

## Side-by-side Examples

### MAUI

```xml
<SearchBar Placeholder="Search products"
           SearchCommand="{Binding RunSearch}"
           SearchCommandParameter="{Binding Filters}"/>
```

### MPAPP (XAML)

```xml
<SearchBar Placeholder="Search products"
           SearchCommand="{Binding RunSearch}"
           SearchCommandParameter="{Binding Filters}"/>
```

### MPAPP (C++)

```cpp
auto sb = std::make_shared<mpapp::search_bar>();
sb->placeholder = "Search products";
sb->search_command.subscribe([](std::string q){
    std::cout << "searching for: " << q << "\n";
});
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Implementation

- Surface: [`include/mpapp/search_bar.hpp`](../../../include/mpapp/search_bar.hpp)
- Mock handler: [`include/mpapp/handlers/mock/search_bar_handler.hpp`](../../../include/mpapp/handlers/mock/search_bar_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/search_bar_handler.hpp`](../../../include/mpapp/handlers/windows/search_bar_handler.hpp) + [`src/handlers/windows/search_bar_handler.cpp`](../../../src/handlers/windows/search_bar_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/search_bar_handler.hpp`](../../../include/mpapp/handlers/linux/search_bar_handler.hpp) + [`src/handlers/linux/search_bar_handler.cpp`](../../../src/handlers/linux/search_bar_handler.cpp)
  - Android: [`include/mpapp/handlers/android/search_bar_handler.hpp`](../../../include/mpapp/handlers/android/search_bar_handler.hpp) + [`src/handlers/android/search_bar_handler.cpp`](../../../src/handlers/android/search_bar_handler.cpp)
- Tests: [`tests/mock_handlers/search_bar_test.cpp`](../../../tests/mock_handlers/search_bar_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Entry]]
