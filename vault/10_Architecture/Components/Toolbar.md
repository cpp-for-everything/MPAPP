---
type: component
mauiHandler: "Toolbar"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/toolbar"
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

# Toolbar

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Toolbar` is the top horizontal bar attached to a [[Page]] or [[NavigationPage]]. It owns a title, optional title icon, optional center `title_view`, a back-button affordance, and a collection of `ToolbarItem` actions. It is *the* surface for page-scoped navigation chrome — distinct from [[TitleBar]] (window chrome) and [[MenuBar]] (menus).

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\Toolbar\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\Toolbar\Toolbar.cs`
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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/toolbar/mock_test.cpp` (planned)
- Windows handler: `tests/components/toolbar/windows_test.cpp` (planned)
- Android handler: `tests/components/toolbar/android_test.cpp` (planned)
- Linux handler: `tests/components/toolbar/linux_test.cpp` (planned)
- macOS handler: `tests/components/toolbar/macos_test.cpp` (planned)
- iOS handler: `tests/components/toolbar/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Property model | Plain CLR + `INotifyPropertyChanged` (no `BindableProperty`) | `Observable<T>` per field | Uniform binding surface ([[ADR-0009-public-api-template-wrappers-only]]) | n/a |
| `BarHeight` | Nullable `double?`; null = platform default | `Observable<std::optional<double>>` | Faithful port | n/a |
| iOS `Secondary` placement | Bottom toolbar (not navigation bar) | Same | OS convention | n/a |
| Drawer toggle | Surfaced via `DrawerToggleVisible` | Same; ignored on non-drawer hosts | Parity | n/a |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[TitleBar]]
- [[MenuBar]]
- [[NavigationPage]]
- [[Page]]
