---
type: component
mauiHandler: "MenuBar"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menubar"
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

# MenuBar

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`MenuBar` is the top-level, horizontal application menu attached to a window or [[Page]]. It is a collection of [[MenuBarItem]] children (File, Edit, View, …) and has a single observable boolean `is_enabled` flag that disables the whole bar at once. On platforms with a native window-chrome menu (Windows, macOS desktop) the bar maps onto the OS menu surface; on mobile platforms the bar collapses into an overflow affordance or is not rendered at all.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\MenuBar\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Menu\MenuBar.cs`
- **Docs:** [Microsoft .NET MAUI — MenuBar](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menubar)

MAUI exposes one `BindableProperty` (`IsEnabled`) and treats the bar as an `IList<IMenuBarItem>` via `Add` / `Insert` / `Remove` / `RemoveAt`. Mutations call `Handler.Invoke("Add" | "Insert" | "Remove", MenuBarHandlerUpdate)` so handlers patch the native menu without a full re-mount.

## MPAPP C++ API

```cpp
namespace mpapp {

class menu_bar : public control<menu_bar> {
public:
    Observable<bool> is_enabled{ true };

    // Children: MenuBarItem collection (incremental — never a full rebuild).
    Observable<observable_list<menu_bar_item>> items;

    void add(menu_bar_item item);
    void insert(std::size_t index, menu_bar_item item);
    void remove_at(std::size_t index);
    void clear();
};

} // namespace mpapp
```

The collection raises granular `(index, item)` change events so the [[Handler]] can patch native menu surfaces in place — this mirrors MAUI's `MenuBarHandlerUpdate` (see `MenuBarHandlerUpdate.cs`).

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<ContentPage.MenuBarItems>
    <MenuBar>
        <MenuBarItem Text="File">
            <MenuFlyoutItem Text="Open"/>
            <MenuFlyoutItem Text="Save"/>
        </MenuBarItem>
        <MenuBarItem Text="Edit">
            <MenuFlyoutItem Text="Copy"/>
        </MenuBarItem>
    </MenuBar>
</ContentPage.MenuBarItems>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `MenuBar` (WinUI 3) | C++/WinRT | Hosted in the [[TitleBar]] region. Each `MenuBarItem` maps to a WinUI `MenuBarItem` with a child `MenuFlyout`. |
| Android | `Toolbar` overflow menu | fbjni / JNI | Android has no menu-bar surface; entries collapse into the action-bar overflow (`PopupMenu`). |
| Linux | `GtkPopoverMenuBar` (GTK4) | GTK4 | Bound to a `GMenu` model; mirrors `IsEnabled` via the GTK action group. |
| macOS | `NSMenu` (app menu) | AppKit | The bar is the *application* menu in the OS menu strip — there is only ever one per app, so the last activated window's `MenuBar` wins. |
| iOS | `UIMenuSystem.main` builder | UIKit | iPadOS hardware-keyboard menu only; iPhone renders nothing (input is via long-press / context menus). |

## Side-by-side Examples

### MAUI

```xml
<ContentPage.MenuBarItems>
    <MenuBarItem Text="File" IsEnabled="True">
        <MenuFlyoutItem Text="Exit" Command="{Binding ExitCommand}"/>
    </MenuBarItem>
</ContentPage.MenuBarItems>
```

### MPAPP (XAML)

```xml
<ContentPage.MenuBarItems>
    <MenuBar IsEnabled="True">
        <MenuBarItem Text="File">
            <MenuFlyoutItem Text="Exit" Command="{Binding exit_command}"/>
        </MenuBarItem>
    </MenuBar>
</ContentPage.MenuBarItems>
```

### MPAPP (C++)

```cpp
auto bar = mpapp::menu_bar{};
auto file = mpapp::menu_bar_item{ .text = "File" };
file.add(mpapp::menu_flyout_item{ .text = "Exit", .command = vm.exit_command });
bar.add(std::move(file));
page.menu_bar_items = std::move(bar);
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/menubar/mock_test.cpp` (planned)
- Windows handler: `tests/components/menubar/windows_test.cpp` (planned)
- Android handler: `tests/components/menubar/android_test.cpp` (planned)
- Linux handler: `tests/components/menubar/linux_test.cpp` (planned)
- macOS handler: `tests/components/menubar/macos_test.cpp` (planned)
- iOS handler: `tests/components/menubar/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Collection mutation events | C# `ObservableCollection` + handler `Invoke(string, args)` | Typed `observable_list<menu_bar_item>` change events | Removes string-keyed dispatch per [[ADR-0009-public-api-template-wrappers-only]] | n/a |
| macOS menu identity | One `NSMenu` per `MenuBar` instance | App menu is process-wide; last-focused window wins | OS constraint | RFC TBD |
| iPhone rendering | No menu bar | Same | OS constraint | n/a |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[MenuBarItem]]
- [[MenuFlyoutItem]]
- [[TitleBar]]
