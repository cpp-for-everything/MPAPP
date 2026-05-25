---
type: component
mauiHandler: "MenuFlyoutSeparator"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menuflyoutseparator"
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

# MenuFlyoutSeparator

> [!info] Status
> **android-real** — Windows `mux::Controls::MenuFlyoutSeparator` + Linux horizontal `GtkSeparator` + Android `android.view.View` w/ 1px minimum height; pure marker type, no observable properties. See [[Controls Inventory]] for the full porting matrix.

## Overview

`MenuFlyoutSeparator` is a non-interactive divider inserted between groups of [[MenuFlyoutItem]] entries inside a [[MenuFlyout]], [[MenuBarItem]], or [[MenuFlyoutSubItem]]. It carries no public properties — it is a positional marker that the handler translates into a platform-native separator row. In MAUI, `MenuFlyoutSeparator` derives from `MenuFlyoutItem` but ignores its properties.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_menu_flyout_separator` | [`include/mpapp/internal/basic_menu_flyout_separator.hpp`](../../../include/mpapp/internal/basic_menu_flyout_separator.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::menu_flyout_separator` | [`include/mpapp/menu_flyout_separator.hpp`](../../../include/mpapp/menu_flyout_separator.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/menu_flyout_separator.hpp>

mpapp::menu_flyout_separator w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/menu_flyout_separator.hpp>
#include <mpapp/handlers/mock/menu_flyout_separator_handler.hpp>

mpapp::internal::basic_menu_flyout_separator w;
mpapp::menu_flyout_separator_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::menu_flyout_separator_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::menu_flyout_separator_handler<>` and `mpapp::menu_flyout_separator_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\MenuFlyoutSeparator\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Menu\MenuFlyoutSeparator.cs`
- **Docs:** [Microsoft .NET MAUI — MenuFlyoutSeparator](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/menuflyoutseparator)

The whole class body is a single line: `public class MenuFlyoutSeparator : MenuFlyoutItem, IMenuFlyoutSeparator { }`. Property inheritance is intentional but unused; the handler keys off the *type*, not the data.

## MPAPP C++ API

```cpp
namespace mpapp {

class menu_flyout_separator : public control<menu_flyout_separator> {
public:
    // No observable properties — separators are a structural marker.
};

} // namespace mpapp
```

The marker type avoids dragging in the unused `Text`/`Command`/etc. surface that MAUI inherits.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<MenuFlyoutSeparator/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `MenuFlyoutSeparator` (WinUI 3) | C++/WinRT | 1:1 mapping. |
| Android | `Menu` with a divider style | fbjni / JNI | Android's `Menu` API does not render dividers by default; handler inserts a styled spacer view. |
| Linux | `GMenuModel` section break | GTK4 | A new section in the `GMenuModel` produces a separator. |
| macOS | `NSMenuItem.separatorItem` | AppKit | Static factory produces the standard separator row. |
| iOS | `UIMenu` inline submenu boundary | UIKit | Separators are implicit between `UIMenuElement` groups created with `UIMenu.Options.displayInline`. |

## Side-by-side Examples

### MAUI

```xml
<MenuFlyout>
    <MenuFlyoutItem Text="Cut"/>
    <MenuFlyoutItem Text="Copy"/>
    <MenuFlyoutSeparator/>
    <MenuFlyoutItem Text="Paste"/>
</MenuFlyout>
```

### MPAPP (XAML)

```xml
<MenuFlyout>
    <MenuFlyoutItem Text="Cut"/>
    <MenuFlyoutItem Text="Copy"/>
    <MenuFlyoutSeparator/>
    <MenuFlyoutItem Text="Paste"/>
</MenuFlyout>
```

### MPAPP (C++)

```cpp
auto flyout = mpapp::menu_flyout{};
flyout.add(mpapp::menu_flyout_item{ .text = "Cut" });
flyout.add(mpapp::menu_flyout_item{ .text = "Copy" });
flyout.add(mpapp::menu_flyout_separator{});
flyout.add(mpapp::menu_flyout_item{ .text = "Paste" });
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Inheritance | `MenuFlyoutSeparator : MenuFlyoutItem` (inherits unused props) | Independent type with no properties | Tighter API ([[ADR-0009-public-api-template-wrappers-only]]) | n/a |
| iOS rendering | Implicit between inline sub-`UIMenu`s | Same; consecutive separators collapse to a single visual divider | UIKit behavior | n/a |
| Android rendering | None (separators dropped) | Handler injects a 1-px divider view to match desktop | Visual parity per [[ADR-0006-interop-parity]] | n/a |

## Implementation

- Surface: [`include/mpapp/menu_flyout_separator.hpp`](../../../include/mpapp/menu_flyout_separator.hpp)
- Mock handler: [`include/mpapp/handlers/mock/menu_flyout_separator_handler.hpp`](../../../include/mpapp/handlers/mock/menu_flyout_separator_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/menu_flyout_separator_handler.hpp`](../../../include/mpapp/handlers/windows/menu_flyout_separator_handler.hpp) + [`src/handlers/windows/menu_flyout_separator_handler.cpp`](../../../src/handlers/windows/menu_flyout_separator_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/menu_flyout_separator_handler.hpp`](../../../include/mpapp/handlers/linux/menu_flyout_separator_handler.hpp) + [`src/handlers/linux/menu_flyout_separator_handler.cpp`](../../../src/handlers/linux/menu_flyout_separator_handler.cpp)
  - Android: [`include/mpapp/handlers/android/menu_flyout_separator_handler.hpp`](../../../include/mpapp/handlers/android/menu_flyout_separator_handler.hpp) + [`src/handlers/android/menu_flyout_separator_handler.cpp`](../../../src/handlers/android/menu_flyout_separator_handler.cpp)
- Tests: [`tests/mock_handlers/menu_flyout_separator_test.cpp`](../../../tests/mock_handlers/menu_flyout_separator_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[MenuFlyout]]
- [[MenuFlyoutItem]]
- [[MenuFlyoutSubItem]]
