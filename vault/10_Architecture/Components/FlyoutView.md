---
type: component
mauiHandler: "FlyoutView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/flyoutview"
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

# FlyoutView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`FlyoutView` is the abstract cross-platform [[View]] that hosts a two-pane master/detail layout: a **flyout** (the slide-out side panel) and a **detail** (the main content area). It is the surface every higher-level navigation primitive — [[FlyoutPage]] and [[Shell]] — composes on top of, and it is what the framework's adaptive layouts collapse into on small screens. It is defined in `Microsoft.Maui.Core` (no `Controls` subclass exists on its own; `FlyoutPage` and `Shell` are the user-facing consumers).

`FlyoutBehavior` controls visibility mode (`Flyout`, `Popover`, `Disabled`), and `IsPresented` toggles the panel open/closed; both are intentionally surfaced for parent navigation containers to manipulate.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_flyout_view` | [`include/mpapp/internal/basic_flyout_view.hpp`](../../../include/mpapp/internal/basic_flyout_view.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::flyout_view` | [`include/mpapp/flyout_view.hpp`](../../../include/mpapp/flyout_view.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/flyout_view.hpp>

mpapp::flyout_view w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/flyout_view.hpp>
#include <mpapp/handlers/mock/flyout_view_handler.hpp>

mpapp::internal::basic_flyout_view w;
mpapp::flyout_view_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::flyout_view_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::flyout_view_handler<>` and `mpapp::flyout_view_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\FlyoutView\`
- **Control:** Defined as an abstract `IFlyoutView` interface in `D:\GitHub\MPAPP\references\maui\src\Core\src\Core\IFlyoutView.cs` — consumed by `FlyoutPage` and `Shell` under `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\`.
- **Docs:** [Microsoft .NET MAUI — FlyoutPage](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/pages/flyoutpage)

## MPAPP C++ API

```cpp
namespace mpapp {

class flyout_view : public view<flyout_view> {
public:
    // The two panes. Often supplied by FlyoutPage/Shell rather than user XAML.
    Observable<std::shared_ptr<view_base>> flyout;
    Observable<std::shared_ptr<view_base>> detail;

    // True when the flyout pane is open. Two-way bindable.
    Observable<bool> is_presented { false };

    // Visibility mode: flyout | popover | disabled.
    Observable<flyout_behavior> flyout_behavior { flyout_behavior::flyout };

    // Width of the flyout pane in DIPs. <0 means "platform default".
    Observable<double> flyout_width { -1.0 };

    // Whether edge-swipe gestures can open the flyout (mobile).
    Observable<bool> is_gesture_enabled { true };
};

} // namespace mpapp
```

`flyout_view` has no verbs — opening/closing is driven by toggling `is_presented`.

## XAML Usage

`FlyoutView` is typically not authored directly; consumers use [[FlyoutPage]] or [[Shell]], which compose a `FlyoutView` internally. The XAML form is still valid for low-level scenarios:

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<FlyoutView FlyoutBehavior="Flyout" IsPresented="{Binding ShowMenu}">
    <FlyoutView.Flyout>
        <ContentView>
            <ListView ItemsSource="{Binding MenuItems}"/>
        </ContentView>
    </FlyoutView.Flyout>
    <FlyoutView.Detail>
        <ContentView>
            <Label Text="Body"/>
        </ContentView>
    </FlyoutView.Detail>
</FlyoutView>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.Maui.Platform.RootNavigationView` | C++/WinRT | Built on `Microsoft.UI.Xaml.Controls.NavigationView`. |
| Android | `androidx.drawerlayout.widget.DrawerLayout` | fbjni / JNI | Edge swipe opens via `IsGestureEnabled`. |
| Linux | Custom `GtkBox` + `GtkRevealer` composition | GTK4 | Targets [[GTK4]] adaptive container patterns. |
| macOS | `NSSplitView` with collapsible sidebar | AppKit via [[Objective-Cpp]] | Behavior `Popover` shows a transient overlay instead. |
| iOS | `UISplitViewController` | UIKit via [[Objective-Cpp]] | `Popover` collapses to a hamburger on compact widths. |

## Side-by-side Examples

### MAUI

```xml
<!-- Most users go through FlyoutPage rather than IFlyoutView directly. -->
<FlyoutPage>
    <FlyoutPage.Flyout>
        <ContentPage Title="Menu"/>
    </FlyoutPage.Flyout>
    <FlyoutPage.Detail>
        <NavigationPage>
            <x:Arguments><ContentPage/></x:Arguments>
        </NavigationPage>
    </FlyoutPage.Detail>
</FlyoutPage>
```

### MPAPP (XAML)

```xml
<FlyoutView FlyoutBehavior="Flyout">
    <FlyoutView.Flyout>
        <ContentView><Label Text="Menu"/></ContentView>
    </FlyoutView.Flyout>
    <FlyoutView.Detail>
        <ContentView><Label Text="Body"/></ContentView>
    </FlyoutView.Detail>
</FlyoutView>
```

### MPAPP (C++)

```cpp
auto fv = std::make_shared<mpapp::flyout_view>();
fv->flyout_behavior = mpapp::flyout_behavior::flyout;

auto menu = std::make_shared<mpapp::content_view>();
menu->content = std::make_shared<mpapp::label>("Menu");
fv->flyout = menu;

auto body = std::make_shared<mpapp::content_view>();
body->content = std::make_shared<mpapp::label>("Body");
fv->detail = body;

fv->is_presented = true;
```

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `Flyout`/`Detail` mapping on iOS/macOS | Mapped through the page-level handler (`FlyoutPage`), not the view-level one | Mapped at the view level for symmetry | Lets `flyout_view` be used directly without a `Page` wrapper | TBD |
| `FlyoutWidth` < 0 sentinel | Same | Same | Matches MAUI defaults | N/A |
| `IsGestureEnabled` on desktop | Effectively no-op | Same — explicitly documented as no-op | Aligns with [[Interop Parity]] | TBD |

## Implementation

- Surface: [`include/mpapp/flyout_view.hpp`](../../../include/mpapp/flyout_view.hpp)
- Mock handler: [`include/mpapp/handlers/mock/flyout_view_handler.hpp`](../../../include/mpapp/handlers/mock/flyout_view_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/flyout_view_handler.hpp`](../../../include/mpapp/handlers/windows/flyout_view_handler.hpp) + [`src/handlers/windows/flyout_view_handler.cpp`](../../../src/handlers/windows/flyout_view_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/flyout_view_handler.hpp`](../../../include/mpapp/handlers/linux/flyout_view_handler.hpp) + [`src/handlers/linux/flyout_view_handler.cpp`](../../../src/handlers/linux/flyout_view_handler.cpp)
  - Android: [`include/mpapp/handlers/android/flyout_view_handler.hpp`](../../../include/mpapp/handlers/android/flyout_view_handler.hpp) + [`src/handlers/android/flyout_view_handler.cpp`](../../../src/handlers/android/flyout_view_handler.cpp)
- Tests: [`tests/mock_handlers/flyout_view_test.cpp`](../../../tests/mock_handlers/flyout_view_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[FlyoutPage]]
- [[Shell]]
- [[View]]
- [[Page]]
