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


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_menu_bar_item` | [`include/mpapp/internal/basic_menu_bar_item.hpp`](../../../include/mpapp/internal/basic_menu_bar_item.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::menu_bar_item` | [`include/mpapp/menu_bar_item.hpp`](../../../include/mpapp/menu_bar_item.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/menu_bar_item.hpp>

mpapp::menu_bar_item w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/menu_bar_item.hpp>
#include <mpapp/handlers/mock/menu_bar_item_handler.hpp>

mpapp::internal::basic_menu_bar_item w;
mpapp::menu_bar_item_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::menu_bar_item_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::menu_bar_item_handler<>` and `mpapp::menu_bar_item_handler<platform::mock>` valid spellings without naming `internal::`.

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

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Children type | `List<IMenuElement>` with runtime-cast `Element` | Typed `observable_list<menu_element>` variant | Compile-time exhaustiveness ([[ADR-0009-public-api-template-wrappers-only]]) | n/a |
| `Priority` semantics | Honoured only on Android | Ignored on Windows/macOS/Linux/iOS; documented as Android-only hint | OS menu surfaces order strictly by insertion | RFC TBD |

## Implementation

- Surface: [`include/mpapp/menu_bar_item.hpp`](../../../include/mpapp/menu_bar_item.hpp)
- Mock handler: [`include/mpapp/handlers/mock/menu_bar_item_handler.hpp`](../../../include/mpapp/handlers/mock/menu_bar_item_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/menu_bar_item_handler.hpp`](../../../include/mpapp/handlers/windows/menu_bar_item_handler.hpp) + [`src/handlers/windows/menu_bar_item_handler.cpp`](../../../src/handlers/windows/menu_bar_item_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/menu_bar_item_handler.hpp`](../../../include/mpapp/handlers/linux/menu_bar_item_handler.hpp) + [`src/handlers/linux/menu_bar_item_handler.cpp`](../../../src/handlers/linux/menu_bar_item_handler.cpp)
  - Android: [`include/mpapp/handlers/android/menu_bar_item_handler.hpp`](../../../include/mpapp/handlers/android/menu_bar_item_handler.hpp) + [`src/handlers/android/menu_bar_item_handler.cpp`](../../../src/handlers/android/menu_bar_item_handler.cpp)
- Tests: [`tests/mock_handlers/menu_bar_item_test.cpp`](../../../tests/mock_handlers/menu_bar_item_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[MenuBar]]
- [[MenuFlyoutItem]]
- [[MenuFlyoutSubItem]]
