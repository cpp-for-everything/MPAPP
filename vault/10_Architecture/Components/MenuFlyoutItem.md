---
type: component
mauiHandler: "MenuFlyoutItem"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menuflyoutitem"
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

# MenuFlyoutItem

> [!info] Status
> **android-real** — Windows `mux::Controls::MenuFlyoutItem` (Click → clicked) + Linux `GtkButton` (flat-styled, "clicked" signal) + Android `android.widget.Button` (Click router deferred to M-05); text + is_enabled + clicked. See [[Controls Inventory]] for the full porting matrix.

## Overview

`MenuFlyoutItem` is a single, invokable leaf inside a [[MenuFlyout]], [[MenuBarItem]], or [[MenuFlyoutSubItem]]. It carries a label, optional icon, an `is_enabled` flag, a list of keyboard accelerators, and a `Command<>` fired when the user activates it. It is the most common menu element — every clickable entry in a menu tree is a `MenuFlyoutItem`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\MenuFlyoutItem\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Menu\MenuFlyoutItem.cs`
- **Docs:** [Microsoft .NET MAUI — MenuFlyoutItem](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menuflyoutitem)

`MenuFlyoutItem : MenuItem, IMenuFlyoutItem` inherits `Text`, `IconImageSource`, `IsEnabled`, `Command`, and `CommandParameter` from `MenuItem`, and adds an `IList<KeyboardAccelerator>` collection.

## MPAPP C++ API

```cpp
namespace mpapp {

class menu_flyout_item : public control<menu_flyout_item> {
public:
    Observable<std::string>    text;
    Observable<image_source>   icon;
    Observable<bool>           is_enabled{ true };

    Command<>                  command;
    Observable<std::any>       command_parameter;

    Observable<observable_list<keyboard_accelerator>> keyboard_accelerators;

    // Fired after the platform invokes the item, before `command` runs.
    Event<>                    clicked;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<MenuFlyoutItem Text="Save" IconImageSource="save.png"
                Command="{Binding save_command}">
    <MenuFlyoutItem.KeyboardAccelerators>
        <KeyboardAccelerator Modifiers="Ctrl" Key="S"/>
    </MenuFlyoutItem.KeyboardAccelerators>
</MenuFlyoutItem>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `MenuFlyoutItem` (WinUI 3) | C++/WinRT | `KeyboardAccelerator` maps 1:1 to WinUI's type; icon set via `IconSource`. |
| Android | `MenuItem` (in a `PopupMenu`) | fbjni / JNI | Accelerators surface only on hardware-keyboard configurations. |
| Linux | `GMenuItem` | GTK4 | Action handler invokes `command`; accelerator registered on the parent window's `GtkApplication`. |
| macOS | `NSMenuItem` | AppKit | `keyEquivalent` + `keyEquivalentModifierMask` set from the first accelerator; icon set via `image`. |
| iOS | `UIAction` inside a `UIMenu` | UIKit | Accelerator on iPadOS only; icon set via `UIImage`. |

## Side-by-side Examples

### MAUI

```xml
<MenuFlyoutItem Text="Copy" Command="{Binding CopyCommand}">
    <MenuFlyoutItem.KeyboardAccelerators>
        <KeyboardAccelerator Modifiers="Ctrl" Key="C"/>
    </MenuFlyoutItem.KeyboardAccelerators>
</MenuFlyoutItem>
```

### MPAPP (XAML)

```xml
<MenuFlyoutItem Text="Copy" Command="{Binding copy_command}">
    <MenuFlyoutItem.KeyboardAccelerators>
        <KeyboardAccelerator Modifiers="Ctrl" Key="C"/>
    </MenuFlyoutItem.KeyboardAccelerators>
</MenuFlyoutItem>
```

### MPAPP (C++)

```cpp
auto copy = mpapp::menu_flyout_item{
    .text = "Copy",
    .command = vm.copy_command,
};
copy.keyboard_accelerators.value().push_back({
    .modifiers = mpapp::key_modifier::ctrl,
    .key = mpapp::key::c,
});
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `Command` / `Clicked` order | `Clicked` fires before `Command` executes | Same: `clicked` event first, then `command` | Parity | n/a |
| Multiple accelerators | All registered on Windows; only first on Android/macOS/Linux/iOS | Same — surplus accelerators documented as Windows-only | OS limit | RFC TBD |
| `CommandParameter` type | `object` (boxed) | `std::any` | C++ idiom — typed wrappers possible per-binding | n/a |

## Implementation

- Surface: [`include/mpapp/menu_flyout_item.hpp`](../../../include/mpapp/menu_flyout_item.hpp)
- Mock handler: [`include/mpapp/handlers/mock/menu_flyout_item_handler.hpp`](../../../include/mpapp/handlers/mock/menu_flyout_item_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/menu_flyout_item_handler.hpp`](../../../include/mpapp/handlers/windows/menu_flyout_item_handler.hpp) + [`src/handlers/windows/menu_flyout_item_handler.cpp`](../../../src/handlers/windows/menu_flyout_item_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/menu_flyout_item_handler.hpp`](../../../include/mpapp/handlers/linux/menu_flyout_item_handler.hpp) + [`src/handlers/linux/menu_flyout_item_handler.cpp`](../../../src/handlers/linux/menu_flyout_item_handler.cpp)
  - Android: [`include/mpapp/handlers/android/menu_flyout_item_handler.hpp`](../../../include/mpapp/handlers/android/menu_flyout_item_handler.hpp) + [`src/handlers/android/menu_flyout_item_handler.cpp`](../../../src/handlers/android/menu_flyout_item_handler.cpp)
- Tests: [`tests/mock_handlers/menu_flyout_item_test.cpp`](../../../tests/mock_handlers/menu_flyout_item_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[MenuFlyout]]
- [[MenuFlyoutSubItem]]
- [[MenuFlyoutSeparator]]
- [[Observable Properties]]
