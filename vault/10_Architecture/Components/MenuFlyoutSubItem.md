---
type: component
mauiHandler: "MenuFlyoutSubItem"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menuflyoutsubitem"
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

# MenuFlyoutSubItem

> [!info] Status
> **android-real** — Windows `mux::Controls::MenuFlyoutSubItem` + Linux `GtkMenuButton` wrapping a nested `GtkPopover` + Android vertical `LinearLayout` w/ TextView header + inner items host; text + items via ADR-0013 dispatch. See [[Controls Inventory]] for the full porting matrix.

## Overview

`MenuFlyoutSubItem` is a [[MenuFlyoutItem]] that, instead of invoking a command, expands into a nested sub-menu. It carries the same label / icon / `is_enabled` surface as a normal item plus an inner collection of `IMenuElement` children — meaning sub-menus can be nested arbitrarily deep (subject to platform caps). On most platforms the entry renders with a trailing chevron indicating expansion.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_menu_flyout_sub_item` | [`include/mpapp/internal/basic_menu_flyout_sub_item.hpp`](../../../include/mpapp/internal/basic_menu_flyout_sub_item.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::menu_flyout_sub_item` | [`include/mpapp/menu_flyout_sub_item.hpp`](../../../include/mpapp/menu_flyout_sub_item.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/menu_flyout_sub_item.hpp>

mpapp::menu_flyout_sub_item w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/menu_flyout_sub_item.hpp>
#include <mpapp/handlers/mock/menu_flyout_sub_item_handler.hpp>

mpapp::internal::basic_menu_flyout_sub_item w;
mpapp::menu_flyout_sub_item_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::menu_flyout_sub_item_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::menu_flyout_sub_item_handler<>` and `mpapp::menu_flyout_sub_item_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\MenuFlyoutSubItem\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Menu\MenuFlyoutSubItem.cs`
- **Docs:** [Microsoft .NET MAUI — MenuFlyoutSubItem](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menuflyoutsubitem)

`MenuFlyoutSubItem : MenuFlyoutItem, IMenuFlyoutSubItem` keeps children in a `List<IMenuElement>` with `Add` / `Insert` / `Remove` / `RemoveAt` that notify `MenuFlyoutSubItemHandlerUpdate(index, item)`.

## MPAPP C++ API

```cpp
namespace mpapp {

class menu_flyout_sub_item : public control<menu_flyout_sub_item> {
public:
    Observable<std::string>  text;
    Observable<image_source> icon;
    Observable<bool>         is_enabled{ true };

    Observable<observable_list<menu_element>> items;

    void add(menu_element item);
    void insert(std::size_t index, menu_element item);
    void remove_at(std::size_t index);
    void clear();
};

} // namespace mpapp
```

Unlike [[MenuFlyoutItem]], there is no `command` — activation only expands the sub-menu.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<MenuFlyoutSubItem Text="Recent files">
    <MenuFlyoutItem Text="report.docx"/>
    <MenuFlyoutItem Text="budget.xlsx"/>
    <MenuFlyoutSeparator/>
    <MenuFlyoutItem Text="Clear list"/>
</MenuFlyoutSubItem>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `MenuFlyoutSubItem` (WinUI 3) | C++/WinRT | Direct 1:1 mapping, includes the trailing chevron. |
| Android | `SubMenu` on `MenuItem` | fbjni / JNI | Android `PopupMenu` permits exactly one level of nesting; deeper trees are flattened. |
| Linux | `GMenu` submenu attribute | GTK4 | Built as a nested `GMenuModel`; depth limited only by available screen space. |
| macOS | `NSMenuItem.submenu` | AppKit | Submenu attached to the item; arbitrary nesting. |
| iOS | `UIMenu` nested in a parent `UIMenu` | UIKit | UIKit supports nested menus; on iPhone the chevron triggers a slide-in panel. |

## Side-by-side Examples

### MAUI

```xml
<MenuFlyoutSubItem Text="Open with…">
    <MenuFlyoutItem Text="Notepad" Command="{Binding OpenNotepad}"/>
    <MenuFlyoutItem Text="VS Code" Command="{Binding OpenVsCode}"/>
</MenuFlyoutSubItem>
```

### MPAPP (XAML)

```xml
<MenuFlyoutSubItem Text="Open with…">
    <MenuFlyoutItem Text="Notepad" Command="{Binding open_notepad}"/>
    <MenuFlyoutItem Text="VS Code" Command="{Binding open_vs_code}"/>
</MenuFlyoutSubItem>
```

### MPAPP (C++)

```cpp
auto open_with = mpapp::menu_flyout_sub_item{ .text = "Open with…" };
open_with.add(mpapp::menu_flyout_item{ .text = "Notepad", .command = vm.open_notepad });
open_with.add(mpapp::menu_flyout_item{ .text = "VS Code", .command = vm.open_vs_code });
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Activation invokes `Command` | Inherited from `MenuFlyoutItem` but unused | `command` is *not* on the type | Removes an inherited dead surface | n/a |
| Nesting depth on Android | Flattened to one level | Same; deeper levels rendered as text-prefixed entries (e.g. `"Recent > report.docx"`) | OS cap | RFC TBD |
| Trailing chevron | Drawn automatically by host | Drawn by handler on each platform; matches platform style | Consistency | n/a |

## Implementation

- Surface: [`include/mpapp/menu_flyout_sub_item.hpp`](../../../include/mpapp/menu_flyout_sub_item.hpp)
- Mock handler: [`include/mpapp/handlers/mock/menu_flyout_sub_item_handler.hpp`](../../../include/mpapp/handlers/mock/menu_flyout_sub_item_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/menu_flyout_sub_item_handler.hpp`](../../../include/mpapp/handlers/windows/menu_flyout_sub_item_handler.hpp) + [`src/handlers/windows/menu_flyout_sub_item_handler.cpp`](../../../src/handlers/windows/menu_flyout_sub_item_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/menu_flyout_sub_item_handler.hpp`](../../../include/mpapp/handlers/linux/menu_flyout_sub_item_handler.hpp) + [`src/handlers/linux/menu_flyout_sub_item_handler.cpp`](../../../src/handlers/linux/menu_flyout_sub_item_handler.cpp)
  - Android: [`include/mpapp/handlers/android/menu_flyout_sub_item_handler.hpp`](../../../include/mpapp/handlers/android/menu_flyout_sub_item_handler.hpp) + [`src/handlers/android/menu_flyout_sub_item_handler.cpp`](../../../src/handlers/android/menu_flyout_sub_item_handler.cpp)
- Tests: [`tests/mock_handlers/menu_flyout_sub_item_test.cpp`](../../../tests/mock_handlers/menu_flyout_sub_item_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[MenuFlyout]]
- [[MenuFlyoutItem]]
- [[MenuFlyoutSeparator]]
- [[MenuBarItem]]
