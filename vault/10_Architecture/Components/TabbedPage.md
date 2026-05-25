---
type: component
mauiHandler: "TabbedPage"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/tabbedpage"
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

# TabbedPage

> [!info] Status
> **android-real** — Windows wraps `mux::Controls::Pivot` with one PivotItem per child page (Header from page.title). Linux uses `GtkNotebook` with one page per child (tab label from page.title). Android uses a vertical LinearLayout with a horizontal tab strip of TextViews + a FrameLayout content host that swaps to the selected page's native. Children are resolved via the ADR-0013 dispatch registry on every collection change. Bar styling Observables still mock-only; real styling lands in M-05 polish.

## Overview

`TabbedPage` is a multi-page container that shows its `Children` as a row of tabs and switches the visible content as the user selects them. It derives from `MultiPage<Page>`, so children can be added directly or generated from an `ItemsSource` + `ItemTemplate`. Each tab's label and icon come from the child page's `Title` and `IconImageSource`. The bar appearance (background, text color, selected/unselected tab colors) is configurable on the parent. For nested tabs inside flyout-based routing, [[Shell]] offers a richer alternative; for a single-pane tab UI inside another page, consider `TabbedView` instead.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_tabbed_page` | [`include/mpapp/internal/basic_tabbed_page.hpp`](../../../include/mpapp/internal/basic_tabbed_page.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::tabbed_page` | [`include/mpapp/tabbed_page.hpp`](../../../include/mpapp/tabbed_page.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/tabbed_page.hpp>

mpapp::tabbed_page w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/tabbed_page.hpp>
#include <mpapp/handlers/mock/tabbed_page_handler.hpp>

mpapp::internal::basic_tabbed_page w;
mpapp::tabbed_page_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::tabbed_page_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::tabbed_page_handler<>` and `mpapp::tabbed_page_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** No dedicated handler — `TabbedPage` uses the base `PageHandler` plus per-platform partial classes in the control project (`TabbedPage.Windows.cs`, `.Android.cs`, `.iOS.cs`).
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\TabbedPage\TabbedPage.cs` + `TabbedPage.Mapper.cs`
- **Docs:** [Microsoft .NET MAUI — TabbedPage](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/tabbedpage)

## MPAPP C++ API

```cpp
namespace mpapp {

class tabbed_page : public multi_page<page> {
public:
    // Tab bar appearance.
    Observable<color>  bar_background_color;
    Observable<brush>  bar_background;
    Observable<color>  bar_text_color;
    Observable<color>  selected_tab_color;
    Observable<color>  unselected_tab_color;

    // Inherited from multi_page<page>:
    // Observable<list<page*>>      children;
    // Observable<page*>            current_page;
    // Observable<observable_list>  items_source;
    // Observable<data_template>    item_template;
    // Event<>                      current_page_changed;

    tabbed_page() = default;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<TabbedPage>
    <ContentPage Title="Home" IconImageSource="home.png">
        <Label Text="Home"/>
    </ContentPage>
    <ContentPage Title="Profile" IconImageSource="profile.png">
        <Label Text="Profile"/>
    </ContentPage>
</TabbedPage>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.NavigationView` in `Top` pane mode | C++/WinRT | Each tab becomes a `NavigationViewItem`; content swapped via `Frame.Navigate`. |
| Android | `androidx.viewpager2.widget.ViewPager2` + `com.google.android.material.tabs.TabLayout` | fbjni / JNI | Fragment-per-tab; swipe between tabs enabled by default. |
| Linux | `GtkNotebook` (top tabs) or `AdwTabBar` + `AdwTabView` for Libadwaita apps | GTK4 | `selected_tab_color` maps to CSS `:checked` styling on the tab labels. |
| macOS | `NSTabViewController` (segmented style) | AppKit | Tabs render in a segmented control; child pages are `NSViewController` instances. |
| iOS | `UITabBarController` | UIKit | Bottom tab bar; max 5 visible tabs before "More" overflow (platform limit). |

## Side-by-side Examples

### MAUI

```xml
<TabbedPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
            SelectedTabColor="Blue" UnselectedTabColor="Gray">
    <ContentPage Title="Home"  IconImageSource="home.png"/>
    <ContentPage Title="Cart"  IconImageSource="cart.png"/>
</TabbedPage>
```

### MPAPP (XAML)

```xml
<TabbedPage SelectedTabColor="Blue" UnselectedTabColor="Gray">
    <ContentPage Title="Home" IconImageSource="home.png"/>
    <ContentPage Title="Cart" IconImageSource="cart.png"/>
</TabbedPage>
```

### MPAPP (C++)

```cpp
auto home = new content_page();  home->title = u8"Home";  home->icon_image_source = u8"home.png";
auto cart = new content_page();  cart->title = u8"Cart";  cart->icon_image_source = u8"cart.png";

auto tabs = new tabbed_page();
tabs->selected_tab_color   = colors::blue;
tabs->unselected_tab_color = colors::gray;
tabs->children.mutate([&](auto& v){ v.push_back(home); v.push_back(cart); });
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Tab position | iOS bottom; Android/Windows top; configurable on Android via `TabbedPage.SetToolbarPlacement`. | Same defaults; placement attached prop preserved. | Platform conventions. | RFC TBD |
| "More" overflow | iOS only (5-tab limit). | Same — UIKit constraint. | Platform limit. | n/a |
| `ItemsSource` data binding | Live binding to `IEnumerable` with `ItemTemplate`. | Same via `observable_list`. | Parity. | n/a |
| Tab swipe gestures | Android only by default. | Same. | Platform conventions. | RFC TBD |

## Implementation

- Surface: [`include/mpapp/tabbed_page.hpp`](../../../include/mpapp/tabbed_page.hpp)
- Mock handler: [`include/mpapp/handlers/mock/tabbed_page_handler.hpp`](../../../include/mpapp/handlers/mock/tabbed_page_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/tabbed_page_handler.hpp`](../../../include/mpapp/handlers/windows/tabbed_page_handler.hpp) + [`src/handlers/windows/tabbed_page_handler.cpp`](../../../src/handlers/windows/tabbed_page_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/tabbed_page_handler.hpp`](../../../include/mpapp/handlers/linux/tabbed_page_handler.hpp) + [`src/handlers/linux/tabbed_page_handler.cpp`](../../../src/handlers/linux/tabbed_page_handler.cpp)
  - Android: [`include/mpapp/handlers/android/tabbed_page_handler.hpp`](../../../include/mpapp/handlers/android/tabbed_page_handler.hpp) + [`src/handlers/android/tabbed_page_handler.cpp`](../../../src/handlers/android/tabbed_page_handler.cpp)
- Tests: [`tests/mock_handlers/tabbed_page_test.cpp`](../../../tests/mock_handlers/tabbed_page_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Page]]
- [[ContentPage]]
- [[TabbedView]]
- [[Shell]]
