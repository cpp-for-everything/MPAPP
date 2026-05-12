---
type: component
mauiHandler: "Picker"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/picker"
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

# Picker

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Picker` is a single-selection drop-down list. The user taps it and the platform shows a native chooser (a popup ComboBox on Windows, a wheel on iOS, a dialog on Android) listing the items in `ItemsSource`. The picked entry is exposed through `SelectedIndex` and `SelectedItem`, both two-way bindable. A static `Title` string acts as the unselected-state caption.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\Picker\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\Picker\`
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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/picker/mock_test.cpp` (planned)
- Windows handler: `tests/components/picker/windows_test.cpp` (planned)
- Android handler: `tests/components/picker/android_test.cpp` (planned)
- Linux handler: `tests/components/picker/linux_test.cpp` (planned)
- macOS handler: `tests/components/picker/macos_test.cpp` (planned)
- iOS handler: `tests/components/picker/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `ItemsSource` type | `IList` (any boxed type) | `std::vector<std::string>` for v1; templated overload planned | C++ type safety per [[Type System]] | RFC TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
