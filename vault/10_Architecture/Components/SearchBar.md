---
type: component
mauiHandler: "SearchBar"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/searchbar"
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

# SearchBar

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`SearchBar` is a specialised single-line text input optimised for queries. It carries a search glyph, a placeholder, and (on most platforms) a clear/cancel button. Submitting (tapping the keyboard's search action) raises `search_command` with the current text. It is conceptually `Entry` + "search" `return_type` + cancel affordance, and uses `AutoSuggestBox`/`UISearchBar`/`SearchView` natively.

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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/searchbar/mock_test.cpp` (planned)
- Windows handler: `tests/components/searchbar/windows_test.cpp` (planned)
- Android handler: `tests/components/searchbar/android_test.cpp` (planned)
- Linux handler: `tests/components/searchbar/linux_test.cpp` (planned)
- macOS handler: `tests/components/searchbar/macos_test.cpp` (planned)
- iOS handler: `tests/components/searchbar/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Entry]]
