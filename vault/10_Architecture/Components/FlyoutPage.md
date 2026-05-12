---
type: component
mauiHandler: "FlyoutPage"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/flyoutpage"
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

# FlyoutPage

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`FlyoutPage` is a two-pane [[Page]] that pairs a navigation menu (the *flyout*) with the currently selected content (the *detail*). On phones the flyout slides over the detail and is dismissed by tap or swipe; on tablets and desktop it can sit beside the detail in a permanent split layout. The `FlyoutLayoutBehavior` enum (`Default`, `Split`, `Popover`, `SplitOnLandscape`, `SplitOnPortrait`) governs which mode applies, and `IsPresented` toggles flyout visibility. For routed apps with deeper hierarchies, [[Shell]] generalizes this pattern and is usually a better fit.

## MAUI Reference

- **Handler:** No dedicated handler — `FlyoutPage` uses `PageHandler` plus the `IFlyoutView` cross-platform contract consumed by `FlyoutViewHandler`. See `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\FlyoutView\`.
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\FlyoutPage\FlyoutPage.cs` (+ `FlyoutLayoutBehavior.cs`, `FlyoutPage.Mapper.cs`)
- **Docs:** [Microsoft .NET MAUI — FlyoutPage](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/flyoutpage)

## MPAPP C++ API

```cpp
namespace mpapp {

enum class flyout_layout_behavior {
    default_, popover, split, split_on_landscape, split_on_portrait
};

class flyout_page : public page {
public:
    // The two child pages. Both required before the page is attached to a window.
    Observable<page*>                   flyout;  // menu pane; must have Title.
    Observable<page*>                   detail;  // primary content pane.

    // Visible (true) or hidden (false). On split layouts, locked open.
    Observable<bool>                    is_presented { false };
    // Whether edge-swipe gestures can open the flyout.
    Observable<bool>                    is_gesture_enabled { true };
    // How the two panes coexist.
    Observable<flyout_layout_behavior>  flyout_layout_behavior { flyout_layout_behavior::default_ };

    flyout_page() = default;

    // Raised whenever is_presented flips.
    Event<>                             is_presented_changed;

    // True when the platform toolbar should show a hamburger / flyout button.
    bool should_show_toolbar_button() const;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<FlyoutPage>
    <FlyoutPage.Flyout>
        <ContentPage Title="Menu">
            <CollectionView ItemsSource="{Binding MenuItems}"/>
        </ContentPage>
    </FlyoutPage.Flyout>
    <FlyoutPage.Detail>
        <NavigationPage>
            <x:Arguments><local:HomePage/></x:Arguments>
        </NavigationPage>
    </FlyoutPage.Detail>
</FlyoutPage>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.NavigationView` in `Left` pane mode | C++/WinRT | Behaves as a two-pane split; the hamburger button toggles `IsPaneOpen`. |
| Android | `androidx.drawerlayout.widget.DrawerLayout` wrapping the detail | fbjni / JNI | Edge-swipe gesture enabled via `setDrawerLockMode`. |
| Linux | `AdwOverlaySplitView` (Adwaita) or `GtkOverlay` + `GtkRevealer` | GTK4 | `Split` mode pins both panes; `Popover` overlays the flyout on top. |
| macOS | `NSSplitViewController` with sidebar style | AppKit | Sidebar toggle button auto-installed in the window toolbar. |
| iOS | `UISplitViewController` (`.doubleColumn` style) | UIKit | Phone idiom collapses to overlay; iPad shows the split. |

## Side-by-side Examples

### MAUI

```xml
<FlyoutPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
            FlyoutLayoutBehavior="SplitOnLandscape">
    <FlyoutPage.Flyout>
        <ContentPage Title="Menu"><Label Text="Items"/></ContentPage>
    </FlyoutPage.Flyout>
    <FlyoutPage.Detail>
        <ContentPage><Label Text="Detail"/></ContentPage>
    </FlyoutPage.Detail>
</FlyoutPage>
```

### MPAPP (XAML)

```xml
<FlyoutPage FlyoutLayoutBehavior="SplitOnLandscape">
    <FlyoutPage.Flyout>
        <ContentPage Title="Menu"><Label Text="Items"/></ContentPage>
    </FlyoutPage.Flyout>
    <FlyoutPage.Detail>
        <ContentPage><Label Text="Detail"/></ContentPage>
    </FlyoutPage.Detail>
</FlyoutPage>
```

### MPAPP (C++)

```cpp
auto menu = new content_page();
menu->title = u8"Menu";
menu->content = new label{.text = u8"Items"};

auto detail = new content_page();
detail->content = new label{.text = u8"Detail"};

auto fp = new flyout_page();
fp->flyout                = menu;
fp->detail                = detail;
fp->flyout_layout_behavior = flyout_layout_behavior::split_on_landscape;
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/flyoutpage/mock_test.cpp` (planned)
- Windows handler: `tests/components/flyoutpage/windows_test.cpp` (planned)
- Android handler: `tests/components/flyoutpage/android_test.cpp` (planned)
- Linux handler: `tests/components/flyoutpage/linux_test.cpp` (planned)
- macOS handler: `tests/components/flyoutpage/macos_test.cpp` (planned)
- iOS handler: `tests/components/flyoutpage/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Flyout title requirement | Throws if `Flyout.Title` is null/empty. | Same — enforced at observable assignment. | Necessary for menu rendering. | n/a |
| `Detail` non-nullable after set | Throws if reassigned to `nullptr`. | Same. | Prevents undefined platform state. | n/a |
| `IsPresented` on split layout | Throws if cleared while split-locked. | Same. | UX consistency. | n/a |
| Default `IsPresented` | `true` on macOS, `false` elsewhere. | Same. | Matches platform conventions (sidebar visible). | n/a |
| Phone idiom | Forces popover even if `Split` requested. | Same. | Real-estate. | n/a |
| Linux | No native two-pane; uses Adwaita `OverlaySplitView`. | Same. | Best GTK4 analog. | RFC TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Page]]
- [[ContentPage]]
- [[NavigationPage]]
- [[Shell]]
- [[FlyoutView]]
