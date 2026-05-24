---
type: component
mauiHandler: "TabbedView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/tabbedview"
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

# TabbedView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`TabbedView` is the abstract, `IView`-only contract behind [[TabbedPage]] — it describes a container with a horizontal bar of tabs and one child page visible at a time. The contract carries the tab bar's appearance properties (background color/brush, text color, selected/unselected tab colors) so that bar-only handlers can be written without inheriting the full `Page` surface. In MPAPP it surfaces as a lightweight view wrapper distinct from [[TabbedPage]], usable as a tabbed sub-region inside any layout.

## MAUI Reference

- **Handler:** MAUI has no `TabbedViewHandler`; the contract is consumed indirectly through [[TabbedPage]]'s `MultiPage<Page>` handler.
- **Control:** Surface defined by `Microsoft.Maui.Controls.ITabbedView` (implemented by `TabbedPage`).
- **Docs:** [Microsoft .NET MAUI — TabbedPage](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/pages/tabbedpage)

MAUI's `TabbedPage : MultiPage<Page>, IBarElement, ITabbedView` exposes `BarBackgroundColor`, `BarBackground`, `BarTextColor`, `UnselectedTabColor`, `SelectedTabColor`, and inherits `Children`, `CurrentPage`, and `ItemsSource` from `MultiPage<Page>`.

## MPAPP C++ API

```cpp
namespace mpapp {

class tabbed_view : public control<tabbed_view> {
public:
    Observable<observable_list<view>> children;
    Observable<view>                  current;

    Observable<color> bar_background_color;
    Observable<brush> bar_background;
    Observable<color> bar_text_color;
    Observable<color> selected_tab_color;
    Observable<color> unselected_tab_color;

    // Optional data-template bridge — mirrors MultiPage<T>.ItemsSource.
    Observable<observable_list<std::any>> items_source;

    Event<view /*old*/, view /*new*/> selection_changed;
};

} // namespace mpapp
```

[[TabbedPage]] is the `page<page>`-derived sibling for use as a top-level route; `tabbed_view` is the embeddable version.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<TabbedView SelectedTabColor="DodgerBlue" UnselectedTabColor="Gray">
    <ContentPage Title="Inbox"/>
    <ContentPage Title="Drafts"/>
    <ContentPage Title="Sent"/>
</TabbedView>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `NavigationView` with `PaneDisplayMode="Top"` (WinUI 3) | C++/WinRT | Each child page becomes a `NavigationViewItem`; selected color drives the indicator brush. |
| Android | `TabLayout` + `ViewPager2` (Material) | fbjni / JNI | Mirrors MAUI's `TabbedPageManager`; swipe-to-switch enabled by default. |
| Linux | `GtkNotebook` (GTK4) | GTK4 | Tab labels bound to child `title`; reorder disabled by default. |
| macOS | `NSTabView` | AppKit | One `NSTabViewItem` per child; tab style configurable via platform extension. |
| iOS | `UITabBarController` (when used as page) / segmented control (when embedded) | UIKit | At top-level uses `UITabBar`; embedded uses `UISegmentedControl` over a content swap. |

## Side-by-side Examples

### MAUI

```xml
<TabbedPage SelectedTabColor="DodgerBlue"
            UnselectedTabColor="Gray">
    <ContentPage Title="Profile">
        <Label Text="Profile"/>
    </ContentPage>
    <ContentPage Title="Settings">
        <Label Text="Settings"/>
    </ContentPage>
</TabbedPage>
```

### MPAPP (XAML)

```xml
<TabbedView SelectedTabColor="DodgerBlue"
            UnselectedTabColor="Gray">
    <ContentPage Title="Profile">
        <Label Text="Profile"/>
    </ContentPage>
    <ContentPage Title="Settings">
        <Label Text="Settings"/>
    </ContentPage>
</TabbedView>
```

### MPAPP (C++)

```cpp
auto tabs = mpapp::tabbed_view{
    .selected_tab_color   = mpapp::colors::dodger_blue,
    .unselected_tab_color = mpapp::colors::gray,
};
tabs.children.value().push_back(mpapp::content_page{ .title = "Profile",  .content = mpapp::label{ .text = "Profile"  } });
tabs.children.value().push_back(mpapp::content_page{ .title = "Settings", .content = mpapp::label{ .text = "Settings" } });
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Standalone type | `ITabbedView` is an internal contract; only `TabbedPage` realises it | `tabbed_view` is a public, embeddable control | Allows tabs inside layouts, not only top-level pages | n/a |
| iOS tab bar | Always `UITabBarController` | `UITabBar` when used as page; `UISegmentedControl` when embedded | Avoids nesting two `UITabBarController`s | n/a |
| Tab reorder | Off by default on all platforms | Same | OS defaults | n/a |
| `ItemsSource` template | `DataTemplate` | `data_template<T>` (compile-time typed) | [[ADR-0009-public-api-template-wrappers-only]] | n/a |

## Implementation

- Surface: [`include/mpapp/tabbed_view.hpp`](../../../include/mpapp/tabbed_view.hpp)
- Mock handler: [`include/mpapp/handlers/mock/tabbed_view_handler.hpp`](../../../include/mpapp/handlers/mock/tabbed_view_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/tabbed_view_handler.hpp`](../../../include/mpapp/handlers/windows/tabbed_view_handler.hpp) + [`src/handlers/windows/tabbed_view_handler.cpp`](../../../src/handlers/windows/tabbed_view_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/tabbed_view_handler.hpp`](../../../include/mpapp/handlers/linux/tabbed_view_handler.hpp) + [`src/handlers/linux/tabbed_view_handler.cpp`](../../../src/handlers/linux/tabbed_view_handler.cpp)
  - Android: [`include/mpapp/handlers/android/tabbed_view_handler.hpp`](../../../include/mpapp/handlers/android/tabbed_view_handler.hpp) + [`src/handlers/android/tabbed_view_handler.cpp`](../../../src/handlers/android/tabbed_view_handler.cpp)
- Tests: [`tests/mock_handlers/tabbed_view_test.cpp`](../../../tests/mock_handlers/tabbed_view_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[TabbedPage]]
- [[Page]]
- [[NavigationPage]]
