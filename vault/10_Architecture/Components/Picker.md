---
type: component
mauiHandler: "Picker"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/picker"
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

# Picker

> [!info] Status — promoted 2026-05-20
> **3-of-5 platforms real (compile-verified)** — `mpapp::picker` with `items: Observable<vector<string>>` + `selected_index: Observable<int>` (-1 = nothing selected) + `title: Observable<string>`. Windows wraps `mux::Controls::ComboBox` (Items.Clear + Append for items; SelectedIndex; PlaceholderText for title). Linux wraps `GtkDropDown` + a `GtkStringList` model (gtk_string_list_splice for items; gtk_drop_down_set_selected for index; title deferred — `GtkDropDown` has no first-class title slot). Android wraps `android.widget.Spinner` + a freshly-constructed `ArrayAdapter<String>` per items() update (`android.R.layout.simple_spinner_item = 0x01090008`); `AbsSpinner.setSelection` for the index; title deferred. macOS / iOS planned in M-06.

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Picker` is a single-selection drop-down list. The user taps it and the platform shows a native chooser (a popup ComboBox on Windows, a wheel on iOS, a dialog on Android) listing the items in `ItemsSource`. The picked entry is exposed through `SelectedIndex` and `SelectedItem`, both two-way bindable. A static `Title` string acts as the unselected-state caption.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Picker\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Picker\`
- **Docs:** [Microsoft .NET MAUI — Picker](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/picker)

## MPAPP C++ API

```cpp
namespace mpapp {

class picker : public control<picker> {
public:
    // Items
    Observable<std::vector<std::string>> items_source;
    Observable<int>                      selected_index{-1};
    Observable<std::string>              selected_item;

    // Presentation
    Observable<std::string> title;
    Observable<color>       title_color;
    Observable<color>       text_color;
    Observable<font>        font;
    Observable<double>      character_spacing{0.0};
    Observable<text_alignment> horizontal_text_alignment{text_alignment::start};
    Observable<text_alignment> vertical_text_alignment{text_alignment::center};

    // State
    Observable<bool> is_open{false};

    // Commands
    Command<>                focus;
    Command<>                unfocus;
    Command<int /*index*/>   selection_changed;
};

} // namespace mpapp
```

See [[Observable Properties]] and [[Handlers]] for the underlying mechanics.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Picker Title="Choose a fruit"
        ItemsSource="{Binding Fruits}"
        SelectedItem="{Binding ChosenFruit, Mode=TwoWay}"/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.ComboBox` | C++/WinRT | Drop-down list; `IsOpen` is two-way. |
| Android | `MauiPicker` (subclass of `EditText`) | fbjni / JNI | Tap opens an `AlertDialog` with a single-choice list. |
| Linux | `GtkDropDown` | GTK4 | Closest GTK equivalent; popover semantics. |
| macOS | `NSPopUpButton` | AppKit | Pull-down menu of items. |
| iOS | `MauiPicker` over `UIPickerView` | UIKit | Wheel picker inside an input accessory. |

## Side-by-side Examples

### MAUI

```xml
<Picker Title="Select a colour"
        ItemsSource="{Binding Colours}"
        SelectedIndex="{Binding ChosenIndex, Mode=TwoWay}"/>
```

### MPAPP (XAML)

```xml
<Picker Title="Select a colour"
        ItemsSource="{Binding Colours}"
        SelectedIndex="{Binding ChosenIndex, Mode=TwoWay}"/>
```

### MPAPP (C++)

```cpp
auto p = std::make_shared<mpapp::picker>();
p->title = "Select a colour";
p->items_source = { "Red", "Green", "Blue" };
p->selection_changed.subscribe([](int i){ std::cout << "picked " << i << "\n"; });
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `ItemsSource` type | `IList` (any boxed type) | `std::vector<std::string>` for v1; templated overload planned | C++ type safety per [[Type System]] | RFC TBD |

## Implementation

- Surface: [`include/mpapp/picker.hpp`](../../../include/mpapp/picker.hpp)
- Mock handler: [`include/mpapp/handlers/mock/picker_handler.hpp`](../../../include/mpapp/handlers/mock/picker_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/picker_handler.hpp`](../../../include/mpapp/handlers/windows/picker_handler.hpp) + [`src/handlers/windows/picker_handler.cpp`](../../../src/handlers/windows/picker_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/picker_handler.hpp`](../../../include/mpapp/handlers/linux/picker_handler.hpp) + [`src/handlers/linux/picker_handler.cpp`](../../../src/handlers/linux/picker_handler.cpp)
  - Android: [`include/mpapp/handlers/android/picker_handler.hpp`](../../../include/mpapp/handlers/android/picker_handler.hpp) + [`src/handlers/android/picker_handler.cpp`](../../../src/handlers/android/picker_handler.cpp)
- Tests: [`tests/mock_handlers/picker_test.cpp`](../../../tests/mock_handlers/picker_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
