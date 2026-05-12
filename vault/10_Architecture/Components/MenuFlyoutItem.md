---
type: component
mauiHandler: "MenuFlyoutItem"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menuflyoutitem"
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

# MenuFlyoutItem

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/menuflyoutitem/mock_test.cpp` (planned)
- Windows handler: `tests/components/menuflyoutitem/windows_test.cpp` (planned)
- Android handler: `tests/components/menuflyoutitem/android_test.cpp` (planned)
- Linux handler: `tests/components/menuflyoutitem/linux_test.cpp` (planned)
- macOS handler: `tests/components/menuflyoutitem/macos_test.cpp` (planned)
- iOS handler: `tests/components/menuflyoutitem/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `Command` / `Clicked` order | `Clicked` fires before `Command` executes | Same: `clicked` event first, then `command` | Parity | n/a |
| Multiple accelerators | All registered on Windows; only first on Android/macOS/Linux/iOS | Same — surplus accelerators documented as Windows-only | OS limit | RFC TBD |
| `CommandParameter` type | `object` (boxed) | `std::any` | C++ idiom — typed wrappers possible per-binding | n/a |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[MenuFlyout]]
- [[MenuFlyoutSubItem]]
- [[MenuFlyoutSeparator]]
- [[Observable Properties]]
