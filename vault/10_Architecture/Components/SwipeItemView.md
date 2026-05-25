---
type: component
mauiHandler: "SwipeItemView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/swipeview"
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

# SwipeItemView

> [!info] Status
> **android-real** — Windows `mux::Controls::ContentControl` + Linux `GtkBox` + Android `FrameLayout` content host; renders the custom action content inline (gesture-reveal deferred). See [[Controls Inventory]] for the full porting matrix.

## Overview

`SwipeItemView` is a [[SwipeView]] action that hosts arbitrary content — any [[View]] — instead of the fixed icon+text shape of [[SwipeItemMenuItem]]. It lets you build fully custom swipe actions (e.g. a tinted panel with a progress ring, or a multi-button cluster) while still participating in SwipeView's gesture, invocation, and `swipe_behavior_on_invoked` lifecycle. It implements both `IContentView` and `ISwipeItem`.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_swipe_item_view` | [`include/mpapp/internal/basic_swipe_item_view.hpp`](../../../include/mpapp/internal/basic_swipe_item_view.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::swipe_item_view` | [`include/mpapp/swipe_item_view.hpp`](../../../include/mpapp/swipe_item_view.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/swipe_item_view.hpp>

mpapp::swipe_item_view w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/swipe_item_view.hpp>
#include <mpapp/handlers/mock/swipe_item_view_handler.hpp>

mpapp::internal::basic_swipe_item_view w;
mpapp::swipe_item_view_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::swipe_item_view_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::swipe_item_view_handler<>` and `mpapp::swipe_item_view_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\SwipeItemView\SwipeItemViewHandler.cs`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\SwipeView\SwipeItemView.cs`
- **Docs:** [Microsoft .NET MAUI — SwipeView (custom items)](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/swipeview)

## MPAPP C++ API

```cpp
namespace mpapp {

class swipe_item_view : public content_view<swipe_item_view> {
public:
    // ISwipeItem surface.
    Observable<std::string>     automation_id;
    Command<>                   command;          // executed when invoked
    Observable<std::any>        command_parameter;

    event<>                     invoked;

    // Inherits content_view<>'s `content` Observable<view*>.

    // Programmatically trigger the action (matches MAUI's OnInvoked).
    void on_invoked();
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<SwipeView>
    <SwipeView.RightItems>
        <SwipeItems>
            <SwipeItemView Command="{Binding FavoriteCommand}">
                <Grid BackgroundColor="Gold" Padding="12">
                    <Image Source="heart.png" HeightRequest="24"/>
                </Grid>
            </SwipeItemView>
        </SwipeItems>
    </SwipeView.RightItems>

    <Label Text="Swipe to favorite"/>
</SwipeView>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.FrameworkElement` (hosted inside `SwipeControl`'s `SwipeItem`) | C++/WinRT | WinUI's `SwipeItem` accepts only icon/text; custom content is hosted via a `Border` containing the MPAPP-rendered view tree. |
| Android | `Microsoft.Maui.Platform.ContentViewGroup` | fbjni / JNI | Plain content host inside the custom `MauiSwipeView`. |
| Linux | `GtkBox` content host inside the SwipeView overlay | GTK4 | Renders any child widget directly. |
| macOS | `Microsoft.Maui.Platform.ContentView` (`NSView` subclass) | AppKit | Custom content host. |
| iOS | `Microsoft.Maui.Platform.ContentView` (`UIView` subclass) | UIKit | Custom content host. |

## Side-by-side Examples

### MAUI

```xml
<SwipeItemView Command="{Binding ArchiveCommand}">
    <Grid BackgroundColor="DarkSlateBlue" Padding="12">
        <Label Text="Archive" TextColor="White"/>
    </Grid>
</SwipeItemView>
```

### MPAPP (XAML)

```xml
<SwipeItemView Command="{Binding ArchiveCommand}">
    <Grid BackgroundColor="DarkSlateBlue" Padding="12">
        <Label Text="Archive" TextColor="White"/>
    </Grid>
</SwipeItemView>
```

### MPAPP (C++)

```cpp
auto item = std::make_shared<mpapp::swipe_item_view>();
item->command = mpapp::Command<>([]{ archive(); });

auto grid = std::make_shared<mpapp::grid>();
grid->background_color = mpapp::colors::dark_slate_blue;
grid->padding          = mpapp::thickness{12};
grid->children.get().push_back(make_label("Archive", mpapp::colors::white));

item->content = grid;
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Inheritance | `ContentView, ISwipeItem` | `content_view<swipe_item_view>` implementing the swipe-item contract | C++ has no multiple-interface inheritance via attributes | — |
| Windows hosting | WinUI `SwipeItem` only supports icon/text → MAUI hosts custom content under the hood | Same approach, documented explicitly | Matches MAUI runtime | — |
| Command type | `ICommand` | `Command<>` template | [[ADR-0009-public-api-template-wrappers-only]] | — |

## Implementation

- Surface: [`include/mpapp/swipe_item_view.hpp`](../../../include/mpapp/swipe_item_view.hpp)
- Mock handler: [`include/mpapp/handlers/mock/swipe_item_view_handler.hpp`](../../../include/mpapp/handlers/mock/swipe_item_view_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/swipe_item_view_handler.hpp`](../../../include/mpapp/handlers/windows/swipe_item_view_handler.hpp) + [`src/handlers/windows/swipe_item_view_handler.cpp`](../../../src/handlers/windows/swipe_item_view_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/swipe_item_view_handler.hpp`](../../../include/mpapp/handlers/linux/swipe_item_view_handler.hpp) + [`src/handlers/linux/swipe_item_view_handler.cpp`](../../../src/handlers/linux/swipe_item_view_handler.cpp)
  - Android: [`include/mpapp/handlers/android/swipe_item_view_handler.hpp`](../../../include/mpapp/handlers/android/swipe_item_view_handler.hpp) + [`src/handlers/android/swipe_item_view_handler.cpp`](../../../src/handlers/android/swipe_item_view_handler.cpp)
- Tests: [`tests/mock_handlers/swipe_item_view_test.cpp`](../../../tests/mock_handlers/swipe_item_view_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[SwipeView]]
- [[SwipeItemMenuItem]]
