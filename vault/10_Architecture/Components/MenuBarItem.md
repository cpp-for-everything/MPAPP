---
type: component
mauiHandler: "MenuBarItem"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menubaritem"
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

# MenuBarItem

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`MenuBarItem` is a single top-level entry inside a [[MenuBar]] — typically "File", "Edit", "View". It exposes a `text` label, an `is_enabled` flag, a `priority` ordering hint, and an inner collection of `IMenuElement` children ([[MenuFlyoutItem]], [[MenuFlyoutSeparator]], [[MenuFlyoutSubItem]]) that render as a drop-down when activated.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\MenuBarItem\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Menu\MenuBarItem.cs`
- **Docs:** [Microsoft .NET MAUI — MenuBarItem](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menubaritem)

MAUI's `MenuBarItem : BaseMenuItem, IMenuBarItem` exposes `BindableProperty` entries for `Text`, `IsEnabled`, and `Priority`. Children are kept in a `List<IMenuElement>` and mutations notify `MenuBarItemHandlerUpdate(index, item)`.

## MPAPP C++ API

```cpp
namespace mpapp {

class menu_bar_item : public control<menu_bar_item> {
public:
    Observable<std::string> text;
    Observable<bool>        is_enabled{ true };
    Observable<int>         priority{ 0 };

    Observable<observable_list<menu_element>> items;

    void add(menu_element item);
    void insert(std::size_t index, menu_element item);
    void remove_at(std::size_t index);
    void clear();
};

} // namespace mpapp
```

`menu_element` is the variant base of [[MenuFlyoutItem]], [[MenuFlyoutSeparator]], and [[MenuFlyoutSubItem]].

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<MenuBarItem Text="File" Priority="0">
    <MenuFlyoutItem Text="New"/>
    <MenuFlyoutSeparator/>
    <MenuFlyoutItem Text="Exit"/>
</MenuBarItem>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `MenuBarItem` (WinUI 3) | C++/WinRT | Hosts a `MenuFlyout` populated from children. |
| Android | `MenuItem` (action-bar overflow) | fbjni / JNI | No nested rendering — children promote into a single `PopupMenu` opened by tapping the entry. |
| Linux | `GMenu` submenu | GTK4 | Each `menu_bar_item` is a `GMenuModel` section bound into the parent `GtkPopoverMenuBar`. |
| macOS | `NSMenuItem` (with submenu) | AppKit | `text` maps to `title`; `is_enabled` to `enabled`. |
| iOS | `UIMenu` | UIKit | Children become a `UIMenu` published through the `UIMenuBuilder` on iPadOS. |

## Side-by-side Examples

### MAUI

```xml
<MenuBarItem Text="Edit" IsEnabled="True">
    <MenuFlyoutItem Text="Copy" Command="{Binding CopyCommand}"/>
    <MenuFlyoutItem Text="Paste" Command="{Binding PasteCommand}"/>
</MenuBarItem>
```

### MPAPP (XAML)

```xml
<MenuBarItem Text="Edit" IsEnabled="True">
    <MenuFlyoutItem Text="Copy" Command="{Binding copy_command}"/>
    <MenuFlyoutItem Text="Paste" Command="{Binding paste_command}"/>
</MenuBarItem>
```

### MPAPP (C++)

```cpp
auto edit = mpapp::menu_bar_item{ .text = "Edit" };
edit.add(mpapp::menu_flyout_item{ .text = "Copy",  .command = vm.copy_command });
edit.add(mpapp::menu_flyout_item{ .text = "Paste", .command = vm.paste_command });
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/menubaritem/mock_test.cpp` (planned)
- Windows handler: `tests/components/menubaritem/windows_test.cpp` (planned)
- Android handler: `tests/components/menubaritem/android_test.cpp` (planned)
- Linux handler: `tests/components/menubaritem/linux_test.cpp` (planned)
- macOS handler: `tests/components/menubaritem/macos_test.cpp` (planned)
- iOS handler: `tests/components/menubaritem/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Children type | `List<IMenuElement>` with runtime-cast `Element` | Typed `observable_list<menu_element>` variant | Compile-time exhaustiveness ([[ADR-0009-public-api-template-wrappers-only]]) | n/a |
| `Priority` semantics | Honoured only on Android | Ignored on Windows/macOS/Linux/iOS; documented as Android-only hint | OS menu surfaces order strictly by insertion | RFC TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[MenuBar]]
- [[MenuFlyoutItem]]
- [[MenuFlyoutSubItem]]
