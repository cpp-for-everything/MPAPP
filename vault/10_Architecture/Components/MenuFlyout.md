---
type: component
mauiHandler: "MenuFlyout"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menuflyout"
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

# MenuFlyout

> [!info] Status
> **android-real** — Windows `mux::Controls::MenuFlyout` + Linux `GtkPopover` w/ vertical `GtkBox` + Android vertical `LinearLayout`; items + is_open via ADR-0013 dispatch. See [[Controls Inventory]] for the full porting matrix.

## Overview

`MenuFlyout` is a context / pop-up menu attached to any [[Element]] via the `FlyoutBase.ContextFlyout` attached property. It is a collection of `IMenuElement` children that appears in response to a right-click, long-press, or programmatic `show()` call. Unlike [[MenuBar]], `MenuFlyout` is not bound to window chrome — it can decorate any control.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_menu_flyout` | [`include/mpapp/internal/basic_menu_flyout.hpp`](../../../include/mpapp/internal/basic_menu_flyout.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::menu_flyout` | [`include/mpapp/menu_flyout.hpp`](../../../include/mpapp/menu_flyout.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/menu_flyout.hpp>

mpapp::menu_flyout w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/menu_flyout.hpp>
#include <mpapp/handlers/mock/menu_flyout_handler.hpp>

mpapp::internal::basic_menu_flyout w;
mpapp::menu_flyout_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::menu_flyout_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::menu_flyout_handler<>` and `mpapp::menu_flyout_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\MenuFlyoutHandler\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Menu\MenuFlyout.cs`
- **Docs:** [Microsoft .NET MAUI — MenuFlyout](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menuflyout)

`MenuFlyout : FlyoutBase, IMenuFlyout` keeps children in a `List<IMenuElement>` and dispatches `ContextFlyoutItemHandlerUpdate(index, item)` to the handler on mutation. The flyout itself owns no display-state properties — show/hide is platform-driven.

## MPAPP C++ API

```cpp
namespace mpapp {

class menu_flyout : public flyout_base<menu_flyout> {
public:
    Observable<observable_list<menu_element>> items;

    void add(menu_element item);
    void insert(std::size_t index, menu_element item);
    void remove_at(std::size_t index);
    void clear();

    // Imperative API for cases where a gesture cannot trigger it natively.
    Command<point /*anchor*/> show;
    Command<>                 hide;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Button Text="Right-click me">
    <FlyoutBase.ContextFlyout>
        <MenuFlyout>
            <MenuFlyoutItem Text="Cut"/>
            <MenuFlyoutItem Text="Copy"/>
            <MenuFlyoutSeparator/>
            <MenuFlyoutSubItem Text="Open with…">
                <MenuFlyoutItem Text="Notepad"/>
            </MenuFlyoutSubItem>
        </MenuFlyout>
    </FlyoutBase.ContextFlyout>
</Button>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `MenuFlyout` (WinUI 3) | C++/WinRT | Triggered by `ContextRequested` event; anchor element drives placement. |
| Android | `PopupMenu` | fbjni / JNI | Anchored to the view that owns the flyout; supports nested submenus via `MenuItem.subMenu`. |
| Linux | `GtkPopoverMenu` | GTK4 | Built from a `GMenuModel`; popped via `gtk_popover_popup()` on right-click / long-press. |
| macOS | `NSMenu.popUpContextMenu` | AppKit | Anchored to the owning `NSView`. |
| iOS | `UIContextMenuInteraction` | UIKit | Requires `UIContextMenuInteractionDelegate` wired by the handler; long-press driven. |

## Side-by-side Examples

### MAUI

```xml
<Label Text="Right-click me">
    <FlyoutBase.ContextFlyout>
        <MenuFlyout>
            <MenuFlyoutItem Text="Inspect" Command="{Binding InspectCommand}"/>
        </MenuFlyout>
    </FlyoutBase.ContextFlyout>
</Label>
```

### MPAPP (XAML)

```xml
<Label Text="Right-click me">
    <FlyoutBase.ContextFlyout>
        <MenuFlyout>
            <MenuFlyoutItem Text="Inspect" Command="{Binding inspect_command}"/>
        </MenuFlyout>
    </FlyoutBase.ContextFlyout>
</Label>
```

### MPAPP (C++)

```cpp
auto flyout = mpapp::menu_flyout{};
flyout.add(mpapp::menu_flyout_item{
    .text = "Inspect",
    .command = vm.inspect_command,
});

label.context_flyout = std::move(flyout);
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Programmatic `Show` | No public API; relies on native gestures | `show(point)` / `hide()` commands | Headless testability + scriptable demos | n/a |
| Trigger gesture | Right-click (desktop), long-press (mobile) | Same per platform | OS convention | n/a |
| iOS context style | `UIContextMenuInteraction` modal | Same; submenus flatten when more than 2 levels deep | UIKit limitation | RFC TBD |

## Implementation

- Surface: [`include/mpapp/menu_flyout.hpp`](../../../include/mpapp/menu_flyout.hpp)
- Mock handler: [`include/mpapp/handlers/mock/menu_flyout_handler.hpp`](../../../include/mpapp/handlers/mock/menu_flyout_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/menu_flyout_handler.hpp`](../../../include/mpapp/handlers/windows/menu_flyout_handler.hpp) + [`src/handlers/windows/menu_flyout_handler.cpp`](../../../src/handlers/windows/menu_flyout_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/menu_flyout_handler.hpp`](../../../include/mpapp/handlers/linux/menu_flyout_handler.hpp) + [`src/handlers/linux/menu_flyout_handler.cpp`](../../../src/handlers/linux/menu_flyout_handler.cpp)
  - Android: [`include/mpapp/handlers/android/menu_flyout_handler.hpp`](../../../include/mpapp/handlers/android/menu_flyout_handler.hpp) + [`src/handlers/android/menu_flyout_handler.cpp`](../../../src/handlers/android/menu_flyout_handler.cpp)
- Tests: [`tests/mock_handlers/menu_flyout_test.cpp`](../../../tests/mock_handlers/menu_flyout_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[MenuFlyoutItem]]
- [[MenuFlyoutSubItem]]
- [[MenuFlyoutSeparator]]
