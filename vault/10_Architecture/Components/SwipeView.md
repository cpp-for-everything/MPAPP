---
type: component
mauiHandler: "SwipeView"
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

# SwipeView

> [!info] Status
> **android-real** — Windows `mux::Controls::SwipeControl` + Linux `GtkBox` content-only + Android `FrameLayout` content-only; `left_items` / `right_items` registered through ADR-0013 dispatch, gestures deferred on Linux/Android. See [[Controls Inventory]] for the full porting matrix.

## Overview

`SwipeView` is a container that wraps a single content view and reveals contextual command panels — left, right, top, or bottom — when the user swipes the content in the corresponding direction. Each side is an `ISwipeItems` collection of [[SwipeItemMenuItem]] or [[SwipeItemView]] entries. SwipeView is most often used inside [[ListView]] / [[CollectionView]] rows to expose "delete" / "archive" / "favourite" actions without taking up persistent screen space.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_swipe_view` | [`include/mpapp/internal/basic_swipe_view.hpp`](../../../include/mpapp/internal/basic_swipe_view.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::swipe_view` | [`include/mpapp/swipe_view.hpp`](../../../include/mpapp/swipe_view.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/swipe_view.hpp>

mpapp::swipe_view w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/swipe_view.hpp>
#include <mpapp/handlers/mock/swipe_view_handler.hpp>

mpapp::internal::basic_swipe_view w;
mpapp::swipe_view_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::swipe_view_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::swipe_view_handler<>` and `mpapp::swipe_view_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\SwipeView\SwipeViewHandler.cs`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\SwipeView\` (lives next to `SwipeItem.cs`, `SwipeItems.cs`)
- **Docs:** [Microsoft .NET MAUI — SwipeView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/swipeview)

## MPAPP C++ API

```cpp
namespace mpapp {

enum class swipe_behavior_on_invoked { auto_close, remain_open };
enum class swipe_mode                 { reveal, execute };
enum class swipe_transition_mode      { reveal, drag };
enum class swipe_direction            { right, left, up, down };

class swipe_items : public element {
public:
    Observable<observable_vector<std::shared_ptr<swipe_item_base>>> items;
    Observable<swipe_mode>                  mode;            // reveal | execute
    Observable<swipe_behavior_on_invoked>   swipe_behavior;  // auto_close | remain_open
};

struct swipe_started_args  { swipe_direction direction; };
struct swipe_changing_args { swipe_direction direction; double offset; };
struct swipe_ended_args    { swipe_direction direction; bool is_open; };

class swipeview : public content_view<swipeview> {
public:
    Observable<swipe_items>          left_items;
    Observable<swipe_items>          right_items;
    Observable<swipe_items>          top_items;
    Observable<swipe_items>          bottom_items;
    Observable<double>               threshold;             // min pixels to recognize
    Observable<bool>                 is_open;
    Observable<swipe_transition_mode> transition_mode;

    event<swipe_started_args>        swipe_started;
    event<swipe_changing_args>       swipe_changing;
    event<swipe_ended_args>          swipe_ended;

    void request_open(swipe_direction dir, bool animated = true);
    void request_close(bool animated = true);
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<SwipeView>
    <SwipeView.RightItems>
        <SwipeItems Mode="Reveal">
            <SwipeItem Text="Archive"
                       BackgroundColor="DarkSlateBlue"
                       Command="{Binding ArchiveCommand}"/>
            <SwipeItem Text="Delete"
                       BackgroundColor="Crimson"
                       Command="{Binding DeleteCommand}"/>
        </SwipeItems>
    </SwipeView.RightItems>

    <Grid Padding="12">
        <Label Text="{Binding Title}"/>
    </Grid>
</SwipeView>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.SwipeControl` | C++/WinRT | Native WinUI SwipeControl supports the same four sides; tracks `is_open` natively. |
| Android | `Microsoft.Maui.Platform.MauiSwipeView` (custom `ViewGroup`) | fbjni / JNI | Custom drag handler; no native equivalent — items are drawn behind the content. |
| Linux | Custom `GtkOverlay` + `GtkGestureDrag` driving a translation transform | GTK4 | No native widget; implemented as a fixed-position panel underneath the content. |
| macOS | Custom `NSView` with `NSPanGestureRecognizer` (`MauiSwipeView`) | AppKit | macOS has no system swipe-actions widget outside `NSTableView`'s row actions; MPAPP implements its own. |
| iOS | `Microsoft.Maui.Platform.MauiSwipeView` (custom `UIView` with `UIPanGestureRecognizer`) | UIKit | Does not use `UISwipeActionsConfiguration` (which is `UITableView`-only); uses a generic gesture so SwipeView works outside lists. |

## Side-by-side Examples

### MAUI

```xml
<SwipeView>
    <SwipeView.LeftItems>
        <SwipeItems>
            <SwipeItem Text="Star" IconImageSource="star.png"/>
        </SwipeItems>
    </SwipeView.LeftItems>
    <Label Text="Swipe me right"/>
</SwipeView>
```

### MPAPP (XAML)

```xml
<SwipeView>
    <SwipeView.LeftItems>
        <SwipeItems>
            <SwipeItem Text="Star" IconImageSource="star.png"/>
        </SwipeItems>
    </SwipeView.LeftItems>
    <Label Text="Swipe me right"/>
</SwipeView>
```

### MPAPP (C++)

```cpp
auto sv   = std::make_shared<mpapp::swipeview>();
auto left = mpapp::swipe_items{};

auto star = std::make_shared<mpapp::swipe_item_menu_item>();
star->text = "Star";
star->icon = mpapp::file_image_source{"star.png"};
star->invoked.connect([] { mpapp::log::info("starred"); });
left.items.get().push_back(star);

sv->left_items = std::move(left);
sv->content    = make_label("Swipe me right");
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Windows uses native widget; other platforms use a custom view | Same on iOS/Android | Same — explicitly documented per platform | Native parity where it exists | — |
| `RequestOpen` / `RequestClose` | Takes a request struct with direction and animated flag | Same shape, but flattened into method args | C++ idiom | — |
| Item collection types | `ISwipeItems : IList<ISwipeItem>` | `swipe_items` with `observable_vector<shared_ptr<swipe_item_base>>` | Compile-time observable contract | — |

## Implementation

- Surface: [`include/mpapp/swipe_view.hpp`](../../../include/mpapp/swipe_view.hpp)
- Mock handler: [`include/mpapp/handlers/mock/swipe_view_handler.hpp`](../../../include/mpapp/handlers/mock/swipe_view_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/swipe_view_handler.hpp`](../../../include/mpapp/handlers/windows/swipe_view_handler.hpp) + [`src/handlers/windows/swipe_view_handler.cpp`](../../../src/handlers/windows/swipe_view_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/swipe_view_handler.hpp`](../../../include/mpapp/handlers/linux/swipe_view_handler.hpp) + [`src/handlers/linux/swipe_view_handler.cpp`](../../../src/handlers/linux/swipe_view_handler.cpp)
  - Android: [`include/mpapp/handlers/android/swipe_view_handler.hpp`](../../../include/mpapp/handlers/android/swipe_view_handler.hpp) + [`src/handlers/android/swipe_view_handler.cpp`](../../../src/handlers/android/swipe_view_handler.cpp)
- Tests: [`tests/mock_handlers/swipe_view_test.cpp`](../../../tests/mock_handlers/swipe_view_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[SwipeItemView]]
- [[SwipeItemMenuItem]]
- [[ListView]]
- [[CollectionView]]
