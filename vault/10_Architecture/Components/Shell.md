---
type: component
mauiHandler: "Shell"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/fundamentals/shell/"
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

# Shell

> [!info] Status
> **android-real** — Windows wraps `mux::Controls::SplitView`: Pane = flyout host (toggled by is_flyout_open), Content = a vertical Grid with a horizontal StackPanel tab strip (Buttons) on top of a `ContentControl` bound to `current_content`. Linux uses a horizontal `GtkPaned`: start child = flyout host (visibility toggled), end child = vertical GtkBox with horizontal tab GtkBox + content GtkBox. Android uses a horizontal LinearLayout with a left FrameLayout (flyout, visibility toggled) + a right vertical LinearLayout containing the tab strip + a FrameLayout content host. Tab buttons set `current_tab_index` on click (Win/Linux; Android click router deferred to M-05). `current_content` swap routes through the ADR-0013 dispatch registry. The full compile-time route table per [[ADR-0016-shell-compile-time-routes]] is deferred; the mock's string-based `go_to()` URI parser still drives `current_route` + `current_tab_index`.

## Overview

`Shell` is MAUI's high-level navigation primitive — a single [[Page]] that combines a flyout, tabs (top + bottom), a navigation stack, search, and URI-based routing into one declarative tree. The hierarchy is `Shell → ShellItem → ShellSection → ShellContent → Page`: `ShellItem`s appear as flyout entries; their `ShellSection`s appear as bottom tabs of the selected item; each section's `ShellContent`s appear as top tabs of the selected section; the leaf `ShellContent` produces the visible `Page`. Apps register routes (`Routing.RegisterRoute("orders/details", typeof(OrderDetailsPage))`) and call `GoToAsync("//main/orders")` or `GoToAsync("details?id=5")` to navigate — the path resolves against the shell tree, and parameters bind into the destination page via `IQueryAttributable` / `[QueryProperty]`. Shell is the largest single control in MAUI (~3,000 LOC of `Shell.cs` plus the `ShellNavigationManager`, `ShellUriHandler`, `ShellAppearance`, `SearchHandler`, and platform renderers); MPAPP's port will follow the same surface to keep MAUI XAML portable, while compile-time-checked route generation is on the table (see Known Differences).


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_shell` | [`include/mpapp/internal/basic_shell.hpp`](../../../include/mpapp/internal/basic_shell.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::shell` | [`include/mpapp/shell.hpp`](../../../include/mpapp/shell.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/shell.hpp>

mpapp::shell w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/shell.hpp>
#include <mpapp/handlers/mock/shell_handler.hpp>

mpapp::internal::basic_shell w;
mpapp::shell_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::shell_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::shell_handler<>` and `mpapp::shell_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** No top-level `ShellHandler` in `Core/src/Handlers`; rendering is done by per-platform compatibility renderers in `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Shell\` plus extension hooks (`IShellController`, `IShellAppearanceElement`).
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Shell\Shell.cs` (~3000 lines) and 30+ siblings:
  - Structure: `ShellItem.cs`, `ShellSection.cs`, `ShellContent.cs`, `BaseShellItem.cs`, `MenuShellItem.cs`, `MenuItemCollection.cs`
  - Navigation: `ShellNavigationManager.cs`, `ShellUriHandler.cs`, `ShellNavigationState.cs`, `ShellNavigationParameters.cs`, `ShellNavigationQueryParameters.cs`, `ShellNavigatingEventArgs.cs`, `ShellNavigatedEventArgs.cs`, `RouteRequestBuilder.cs`, `RequestDefinition.cs`
  - Appearance: `ShellAppearance.cs`, `FlyoutHeaderBehavior.cs`, `FlyoutDisplayOptions.cs`
  - Search: `SearchHandler.cs`, `ISearchHandlerController.cs`
  - Back nav: `BackButtonBehavior.cs`
- **Docs:** [Microsoft .NET MAUI — Shell](https://learn.microsoft.com/en-us/dotnet/maui/fundamentals/shell/)
- **Deep dive:** [[60_Research/dotnet-maui-deep-dive]]

## MPAPP C++ API

```cpp
namespace mpapp {

// --- Hierarchy --------------------------------------------------------------
// shell -> shell_item (flyout entry) -> shell_section (bottom tab)
//       -> shell_content (top tab) -> page (leaf)

class shell_content : public element {
public:
    Observable<std::u8string>   route;     // identifier used by go_to_async()
    Observable<std::u8string>   title;
    Observable<image_source>    icon;
    Observable<bool>            is_visible { true };
    Observable<bool>            is_enabled { true };
    Observable<data_template>   content_template; // creates the page on first navigation
    Observable<page*>           content;          // direct alternative to template
};

class shell_section : public element {
public:
    Observable<std::u8string>           route;
    Observable<std::u8string>           title;
    Observable<image_source>            icon;
    Observable<list<shell_content*>>    items;       // top tabs
    Observable<shell_content*>          current_item;
};

class shell_item : public element {
public:
    Observable<std::u8string>           route;
    Observable<std::u8string>           title;
    Observable<image_source>            icon;
    Observable<image_source>            flyout_icon;
    Observable<list<shell_section*>>    items;       // bottom tabs
    Observable<shell_section*>          current_item;
    Observable<flyout_display_options>  flyout_display_options;
};

enum class flyout_behavior          { disabled, flyout, locked };
enum class flyout_header_behavior   { default_, fixed, scroll, collapse_on_scroll };
enum class flyout_display_options   { as_multiple_items, as_single_item };
enum class presentation_mode        { not_animated, animated, modal_not_animated, modal_animated };

class shell : public page {
public:
    // Tree.
    Observable<list<shell_item*>>       items;          // root collection
    Observable<shell_item*>             current_item;
    Observable<page*>                   current_page;   // leaf (read-only)
    Observable<shell_navigation_state>  current_state;  // current URI

    // Flyout layout & visuals.
    Observable<flyout_behavior>         flyout_behavior { flyout_behavior::flyout };
    Observable<bool>                    flyout_is_presented { false };
    Observable<image_source>            flyout_icon;
    Observable<color>                   flyout_background_color;
    Observable<brush>                   flyout_background;
    Observable<image_source>            flyout_background_image;
    Observable<aspect>                  flyout_background_image_aspect;
    Observable<brush>                   flyout_backdrop;
    Observable<double>                  flyout_width  { -1.0 };
    Observable<double>                  flyout_height { -1.0 };
    Observable<flyout_header_behavior>  flyout_header_behavior;
    Observable<view*>                   flyout_header;
    Observable<view*>                   flyout_footer;
    Observable<view*>                   flyout_content;
    Observable<data_template>           flyout_header_template;
    Observable<data_template>           flyout_footer_template;
    Observable<data_template>           flyout_content_template;
    Observable<data_template>           item_template;
    Observable<data_template>           menu_item_template;
    Observable<scroll_mode>             flyout_vertical_scroll_mode;

    // Tab & navigation bar visuals.
    Observable<color>  tab_bar_background_color;
    Observable<color>  tab_bar_foreground_color;
    Observable<color>  tab_bar_title_color;
    Observable<color>  tab_bar_unselected_color;
    Observable<color>  tab_bar_disabled_color;
    Observable<color>  title_color;
    Observable<color>  unselected_color;
    Observable<color>  disabled_color;
    Observable<color>  foreground_color;

    // Search.
    Observable<search_handler*>  search_handler;

    // --- Navigation API ---
    Command<task<void>(shell_navigation_state)>                                go_to_async;
    Command<task<void>(shell_navigation_state, bool /*animate*/)>              go_to_async_animated;
    Command<task<void>(shell_navigation_state, dict<u8string, value>)>         go_to_async_params;
    Command<task<void>(shell_navigation_state, bool, dict<u8string, value>)>   go_to_async_full;

    // Navigation events (deferrable).
    Event<shell_navigating_args>  navigating; // can be cancelled / deferred
    Event<shell_navigated_args>   navigated;

    // Route registry (static, populated at app start or by source-gen).
    static void register_route(std::u8string_view route, std::function<page*()> factory);

    // Per-page attached properties (set on the leaf page).
    static void set_flyout_behavior(page&, flyout_behavior);
    static void set_nav_bar_is_visible(page&, bool);
    static void set_nav_bar_has_shadow(page&, bool);
    static void set_tab_bar_is_visible(page&, bool);
    static void set_presentation_mode(page&, presentation_mode);
    static void set_back_button_behavior(page&, back_button_behavior);
    static void set_search_handler(page&, search_handler*);
    static void set_title_view(page&, view*);
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Shell xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
       FlyoutBehavior="Flyout">
    <ShellItem Title="Home" Icon="home.png">
        <ShellSection>
            <ShellContent Route="home"     ContentTemplate="{DataTemplate local:HomePage}"/>
            <ShellContent Route="trending" ContentTemplate="{DataTemplate local:TrendingPage}"/>
        </ShellSection>
    </ShellItem>
    <ShellItem Title="Account" Icon="account.png">
        <ShellContent Route="profile" ContentTemplate="{DataTemplate local:ProfilePage}"/>
    </ShellItem>
</Shell>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.NavigationView` (flyout) + `Frame` per section | C++/WinRT | Flyout pane is `NavigationView.PaneDisplayMode = LeftCompact` / `Top` based on `flyout_behavior`. Bottom tabs render as `NavigationViewItem` children when there are >1 sections. |
| Android | `androidx.drawerlayout.widget.DrawerLayout` + `BottomNavigationView` + `ViewPager2` for top tabs | fbjni / JNI | Hardware back button consults the `back_button_behavior` attached property. Search bar uses `androidx.appcompat.widget.SearchView`. |
| Linux | `AdwOverlaySplitView` (flyout) + `AdwViewSwitcher` (top tabs) + `GtkBox` for bottom nav | GTK4 | Bottom-tab style is non-native; emulated with a `GtkBox` of toggle buttons styled like a navigation rail. |
| macOS | `NSSplitViewController` (sidebar) + `NSTabViewController` per section | AppKit | Sidebar item icons sourced from `ImageSource`. Tab bar maps to a segmented control in the toolbar. |
| iOS | `UISplitViewController` + `UITabBarController` + `UINavigationController` | UIKit | Phone idiom uses overlay flyout; iPad shows split. Search bar uses `UISearchController`. |

## Side-by-side Examples

### MAUI

```csharp
// AppShell.xaml.cs
public AppShell()
{
    InitializeComponent();
    Routing.RegisterRoute("orders/details", typeof(OrderDetailsPage));
}

// somewhere in the app:
await Shell.Current.GoToAsync("//main/orders/details?id=42");
```

```xml
<Shell xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
       xmlns:local="clr-namespace:Sample"
       x:Class="Sample.AppShell"
       FlyoutBehavior="Flyout">
    <FlyoutItem Title="Main" Icon="main.png" Route="main">
        <Tab Title="Home"   Icon="home.png"   Route="home">
            <ShellContent ContentTemplate="{DataTemplate local:HomePage}"/>
        </Tab>
        <Tab Title="Orders" Icon="orders.png" Route="orders">
            <ShellContent ContentTemplate="{DataTemplate local:OrdersPage}"/>
        </Tab>
    </FlyoutItem>
</Shell>
```

### MPAPP (XAML)

```xml
<Shell FlyoutBehavior="Flyout">
    <FlyoutItem Title="Main" Icon="main.png" Route="main">
        <Tab Title="Home"   Icon="home.png"   Route="home">
            <ShellContent ContentTemplate="{DataTemplate local:HomePage}"/>
        </Tab>
        <Tab Title="Orders" Icon="orders.png" Route="orders">
            <ShellContent ContentTemplate="{DataTemplate local:OrdersPage}"/>
        </Tab>
    </FlyoutItem>
</Shell>
```

### MPAPP (C++)

```cpp
class app_shell : public shell {
public:
    app_shell() {
        flyout_behavior = flyout_behavior::flyout;

        auto home   = new shell_content();
        home->route = u8"home";
        home->content_template = data_template_for<home_page>();

        auto orders   = new shell_content();
        orders->route = u8"orders";
        orders->content_template = data_template_for<orders_page>();

        auto main_section = new shell_section();
        main_section->items.mutate([&](auto& v){ v.push_back(home); v.push_back(orders); });

        auto main = new shell_item();
        main->route = u8"main";
        main->title = u8"Main";
        main->icon  = u8"main.png";
        main->items.mutate([&](auto& v){ v.push_back(main_section); });

        items.mutate([&](auto& v){ v.push_back(main); });

        shell::register_route(u8"orders/details", []{ return new order_details_page(); });
    }
};

// Navigation:
co_await shell::current().go_to_async(u8"//main/orders/details?id=42");
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Route registration | Runtime via `Routing.RegisterRoute(string, Type)`. | Runtime `register_route(u8string_view, factory)` + source-generated typed routes (`routes::orders_details(id)`). | Compile-time checked navigation. | RFC TBD |
| Query parameters | `[QueryProperty]` attribute or `IQueryAttributable.ApplyQueryAttributes`. | Typed parameter dictionary passed to the page constructor / `apply_query_attributes`. | Stronger typing. | RFC TBD |
| `Shell.Current` | Static accessor reaching into `Application.Current`. | `shell::current()` — same shape; throws if no shell. | Parity. | n/a |
| Search handler | `SearchHandler` is a `BindableObject` with templates. | Same shape; templates use compile-time-typed `data_template<T>`. | Parity. | n/a |
| Modal presentation | `PresentationMode.Modal*` pushes modally. | Same enum, same behavior. | Parity. | n/a |
| Deferred navigation | `ShellNavigatingDeferral` lets handlers await cancellation. | `shell_navigating_args::get_deferral()` returns an RAII object. | Parity. | n/a |
| Compatibility renderers | iOS/Mac/Android use legacy renderers; Windows uses Maui handler. | Single handler per platform. | New codebase; no legacy debt. | n/a |
| `FlyoutItem` / `Tab` XAML shorthand | XAML conveniences that desugar to `ShellItem`/`ShellSection`. | Same — preserved by XAML compiler. | XAML parity (ADR-0004). | n/a |

## Implementation

- Surface: [`include/mpapp/shell.hpp`](../../../include/mpapp/shell.hpp)
- Mock handler: [`include/mpapp/handlers/mock/shell_handler.hpp`](../../../include/mpapp/handlers/mock/shell_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/shell_handler.hpp`](../../../include/mpapp/handlers/windows/shell_handler.hpp) + [`src/handlers/windows/shell_handler.cpp`](../../../src/handlers/windows/shell_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/shell_handler.hpp`](../../../include/mpapp/handlers/linux/shell_handler.hpp) + [`src/handlers/linux/shell_handler.cpp`](../../../src/handlers/linux/shell_handler.cpp)
  - Android: [`include/mpapp/handlers/android/shell_handler.hpp`](../../../include/mpapp/handlers/android/shell_handler.hpp) + [`src/handlers/android/shell_handler.cpp`](../../../src/handlers/android/shell_handler.cpp)
- Tests: [`tests/mock_handlers/shell_test.cpp`](../../../tests/mock_handlers/shell_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Page]]
- [[ContentPage]]
- [[NavigationPage]]
- [[FlyoutPage]]
- [[TabbedPage]]
- [[60_Research/dotnet-maui-deep-dive]]
