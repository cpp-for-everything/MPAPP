---
type: component
mauiHandler: "Toolbar"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/toolbar"
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

# Toolbar

> [!info] Status
> **android-real** — Windows `mux::Controls::CommandBar` + `AppBarButton` per item, Linux `GtkActionBar` + `GtkButton` per item, Android `android.widget.Toolbar` with Menu items. Items rebuilt on collection change. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Toolbar` is the top horizontal bar attached to a [[Page]] or [[NavigationPage]]. It owns a title, optional title icon, optional center `title_view`, a back-button affordance, and a collection of `ToolbarItem` actions. It is *the* surface for page-scoped navigation chrome — distinct from [[TitleBar]] (window chrome) and [[MenuBar]] (menus).


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_toolbar` | [`include/mpapp/internal/basic_toolbar.hpp`](../../../include/mpapp/internal/basic_toolbar.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::toolbar` | [`include/mpapp/toolbar.hpp`](../../../include/mpapp/toolbar.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/toolbar.hpp>

mpapp::toolbar w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/toolbar.hpp>
#include <mpapp/handlers/mock/toolbar_handler.hpp>

mpapp::internal::basic_toolbar w;
mpapp::toolbar_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::toolbar_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::toolbar_handler<>` and `mpapp::toolbar_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Toolbar\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Toolbar\Toolbar.cs`
- **Docs:** [Microsoft .NET MAUI — Toolbar](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/toolbar)

MAUI's `Toolbar : IToolbar, INotifyPropertyChanged` exposes plain CLR properties (no `BindableProperty`): `Title`, `TitleIcon`, `TitleView`, `BarBackground`, `BarTextColor`, `IconColor`, `BarHeight`, `BackButtonTitle`, `BackButtonVisible`, `BackButtonEnabled`, `DrawerToggleVisible`, `DynamicOverflowEnabled`, `IsVisible`, and the `ToolbarItems` enumerable.

Each `ToolbarItem : MenuItem` adds `Order` (`Default | Primary | Secondary`) and a `Priority` integer.

## MPAPP C++ API

```cpp
namespace mpapp {

class toolbar : public control<toolbar> {
public:
    Observable<std::string>     title;
    Observable<image_source>    title_icon;
    Observable<view>            title_view;

    Observable<brush>           bar_background;
    Observable<color>           bar_text_color;
    Observable<color>           icon_color;
    Observable<std::optional<double>> bar_height;

    Observable<std::string>     back_button_title;
    Observable<bool>            back_button_visible{ false };
    Observable<bool>            back_button_enabled{ true };
    Observable<bool>            drawer_toggle_visible{ false };

    Observable<bool>            dynamic_overflow_enabled{ true };
    Observable<bool>            is_visible{ true };

    Observable<observable_list<toolbar_item>> items;
};

class toolbar_item : public control<toolbar_item> {
public:
    Observable<std::string>    text;
    Observable<image_source>   icon;
    Observable<bool>           is_enabled{ true };

    enum class order { default_, primary, secondary };
    Observable<order>          order{ order::default_ };
    Observable<int>            priority{ 0 };

    Command<>                  command;
    Event<>                    clicked;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<ContentPage Title="Inbox">
    <ContentPage.ToolbarItems>
        <ToolbarItem Text="Compose" IconImageSource="compose.png" Order="Primary"
                     Command="{Binding compose_command}"/>
        <ToolbarItem Text="Settings" Order="Secondary"
                     Command="{Binding settings_command}"/>
    </ContentPage.ToolbarItems>
</ContentPage>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `CommandBar` (WinUI 3) | C++/WinRT | `Primary` items become `AppBarButton`s; `Secondary` items fill the overflow `…` menu. |
| Android | `androidx.appcompat.widget.Toolbar` + `PopupMenu` | fbjni / JNI | Mirrors MAUI's `ToolbarExtensions`; `Secondary` items go to the action-bar overflow. |
| Linux | `GtkHeaderBar` + `GtkPopoverMenu` | GTK4 | `title_view` slots into the headerbar's custom-title area. |
| macOS | `NSToolbar` (on `NSWindow`) | AppKit | `ToolbarItem`s become `NSToolbarItem`s; secondary items collapse into the customisable overflow. |
| iOS | `UINavigationBar` + `UIBarButtonItem`s | UIKit | The bar is the navigation controller's; `Primary` items align right, `Secondary` items align in the bottom toolbar. |

## Side-by-side Examples

### MAUI

```xml
<ContentPage Title="Notes">
    <ContentPage.ToolbarItems>
        <ToolbarItem Text="Add" IconImageSource="add.png" Order="Primary"
                     Command="{Binding AddCommand}"/>
    </ContentPage.ToolbarItems>
</ContentPage>
```

### MPAPP (XAML)

```xml
<ContentPage Title="Notes">
    <ContentPage.ToolbarItems>
        <ToolbarItem Text="Add" IconImageSource="add.png" Order="Primary"
                     Command="{Binding add_command}"/>
    </ContentPage.ToolbarItems>
</ContentPage>
```

### MPAPP (C++)

```cpp
auto page = mpapp::content_page{ .title = "Notes" };
page.toolbar_items.value().push_back(mpapp::toolbar_item{
    .text = "Add",
    .icon = mpapp::image_source::from_file("add.png"),
    .order = mpapp::toolbar_item::order::primary,
    .command = vm.add_command,
});
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Property model | Plain CLR + `INotifyPropertyChanged` (no `BindableProperty`) | `Observable<T>` per field | Uniform binding surface ([[ADR-0009-public-api-template-wrappers-only]]) | n/a |
| `BarHeight` | Nullable `double?`; null = platform default | `Observable<std::optional<double>>` | Faithful port | n/a |
| iOS `Secondary` placement | Bottom toolbar (not navigation bar) | Same | OS convention | n/a |
| Drawer toggle | Surfaced via `DrawerToggleVisible` | Same; ignored on non-drawer hosts | Parity | n/a |

## Implementation

- Surface: [`include/mpapp/toolbar.hpp`](../../../include/mpapp/toolbar.hpp)
- Mock handler: [`include/mpapp/handlers/mock/toolbar_handler.hpp`](../../../include/mpapp/handlers/mock/toolbar_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/toolbar_handler.hpp`](../../../include/mpapp/handlers/windows/toolbar_handler.hpp) + [`src/handlers/windows/toolbar_handler.cpp`](../../../src/handlers/windows/toolbar_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/toolbar_handler.hpp`](../../../include/mpapp/handlers/linux/toolbar_handler.hpp) + [`src/handlers/linux/toolbar_handler.cpp`](../../../src/handlers/linux/toolbar_handler.cpp)
  - Android: [`include/mpapp/handlers/android/toolbar_handler.hpp`](../../../include/mpapp/handlers/android/toolbar_handler.hpp) + [`src/handlers/android/toolbar_handler.cpp`](../../../src/handlers/android/toolbar_handler.cpp)
- Tests: [`tests/mock_handlers/toolbar_test.cpp`](../../../tests/mock_handlers/toolbar_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[TitleBar]]
- [[MenuBar]]
- [[NavigationPage]]
- [[Page]]
