---
type: component
mauiHandler: "SwipeItemMenuItem"
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

# SwipeItemMenuItem

> [!info] Status
> **android-real** — Windows `mux::Controls::Button` (Click → `invoked`) + Linux `GtkButton` (`"clicked"` → `invoked`) + Android `android.widget.Button` (text-only; OnClickListener routing deferred until MppClickRouter generalises beyond `button`). `text` + `icon_uri` are live; richer `background` / `is_destructive` surface deferred. See [[Controls Inventory]] for the full porting matrix.

## Overview

`SwipeItemMenuItem` is the standard icon-plus-text action used inside a [[SwipeView]] — the "Delete", "Archive", "Favourite" pills you typically see when you swipe a list row. It derives from `MenuItem` (so it inherits `Text`, `IconImageSource`, `Command`, `CommandParameter`, `IsDestructive`) and implements `ISwipeItem`, which means it also surfaces a `Background` paint and a `Visibility` value so SwipeView can size and tint the action pill consistently across platforms.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_swipe_item_menu_item` | [`include/mpapp/internal/basic_swipe_item_menu_item.hpp`](../../../include/mpapp/internal/basic_swipe_item_menu_item.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::swipe_item_menu_item` | [`include/mpapp/swipe_item_menu_item.hpp`](../../../include/mpapp/swipe_item_menu_item.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/swipe_item_menu_item.hpp>

mpapp::swipe_item_menu_item w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/swipe_item_menu_item.hpp>
#include <mpapp/handlers/mock/swipe_item_menu_item_handler.hpp>

mpapp::internal::basic_swipe_item_menu_item w;
mpapp::swipe_item_menu_item_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::swipe_item_menu_item_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::swipe_item_menu_item_handler<>` and `mpapp::swipe_item_menu_item_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\SwipeItemMenuItem\SwipeItemMenuItemHandler.cs`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\SwipeView\SwipeItem.cs` (type is `Microsoft.Maui.Controls.SwipeItem` — `SwipeItemMenuItem` is the handler-name alias)
- **Docs:** [Microsoft .NET MAUI — SwipeView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/swipeview)

## MPAPP C++ API

```cpp
namespace mpapp {

enum class visibility { visible, hidden, collapsed };

class swipe_item_menu_item : public menu_item /* and ISwipeItem */ {
public:
    // From menu_item (MAUI's MenuItem):
    //   Observable<std::string>           text;
    //   Observable<image_source>          icon;          // IconImageSource
    //   Observable<color>                 text_color;
    //   Command<>                         command;
    //   Observable<std::any>              command_parameter;
    //   Observable<bool>                  is_destructive;
    //   Observable<bool>                  is_enabled;
    //   Observable<std::string>           automation_id;

    // SwipeItemMenuItem-specific additions:
    Observable<color>       background;     // pill fill — Paint? in MAUI
    Observable<visibility>  visible;

    // Fired when the user releases the swipe over this item.
    event<>                 invoked;

    // Programmatically trigger the action.
    void on_invoked();
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<SwipeView>
    <SwipeView.RightItems>
        <SwipeItems Mode="Reveal">
            <SwipeItem Text="Delete"
                       IconImageSource="trash.png"
                       BackgroundColor="Crimson"
                       IsDestructive="true"
                       Command="{Binding DeleteCommand}"/>
        </SwipeItems>
    </SwipeView.RightItems>

    <Label Text="{Binding Title}"/>
</SwipeView>
```

> XAML uses `<SwipeItem>` (the MAUI public name); the underlying handler is `SwipeItemMenuItem` and matches one-for-one.

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.SwipeItem` | C++/WinRT | Native WinUI swipe-item; honours `Text`, `IconSource`, `Background`, `BehaviorOnInvoked`. |
| Android | `android.views.View` rendered as an icon+text pill (`MauiSwipeItem`) | fbjni / JNI | Custom-drawn inside the parent `MauiSwipeView`; no system widget. |
| Linux | `GtkButton` styled as a pill (label + icon) | GTK4 | Hosted inside the SwipeView overlay container. |
| macOS | `NSButton` with custom drawing | AppKit | Pill background painted in `drawRect:`. |
| iOS | `UIKit.UIButton` (`MauiSwipeItem`) | UIKit | Title + image; tint set from `background`. |

## Side-by-side Examples

### MAUI

```xml
<SwipeItem Text="Archive"
           IconImageSource="archive.png"
           BackgroundColor="DarkSlateBlue"
           Command="{Binding ArchiveCommand}"/>
```

### MPAPP (XAML)

```xml
<SwipeItem Text="Archive"
           IconImageSource="archive.png"
           BackgroundColor="DarkSlateBlue"
           Command="{Binding ArchiveCommand}"/>
```

### MPAPP (C++)

```cpp
auto archive = std::make_shared<mpapp::swipe_item_menu_item>();
archive->text       = "Archive";
archive->icon       = mpapp::file_image_source{"archive.png"};
archive->background = mpapp::colors::dark_slate_blue;
archive->command    = mpapp::Command<>([]{ archive_selected(); });

archive->invoked.connect([] { mpapp::log::info("archived"); });
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Background type | `Paint?` (allows brushes) | `color` only | Native swipe-item widgets honour solid colors only | — |
| XAML element name | `<SwipeItem>` maps to `SwipeItemMenuItem` handler | Same XAML element, same handler name | Mirror MAUI naming asymmetry | — |
| `Command` type | `ICommand` | `Command<>` template | [[ADR-0009-public-api-template-wrappers-only]] | — |
| `IsDestructive` | Hints platforms to use destructive styling (red on iOS) | Same — iOS/macOS apply destructive role; other platforms ignore | Platform-native styling | — |

## Implementation

- Surface: [`include/mpapp/swipe_item_menu_item.hpp`](../../../include/mpapp/swipe_item_menu_item.hpp)
- Mock handler: [`include/mpapp/handlers/mock/swipe_item_menu_item_handler.hpp`](../../../include/mpapp/handlers/mock/swipe_item_menu_item_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/swipe_item_menu_item_handler.hpp`](../../../include/mpapp/handlers/windows/swipe_item_menu_item_handler.hpp) + [`src/handlers/windows/swipe_item_menu_item_handler.cpp`](../../../src/handlers/windows/swipe_item_menu_item_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/swipe_item_menu_item_handler.hpp`](../../../include/mpapp/handlers/linux/swipe_item_menu_item_handler.hpp) + [`src/handlers/linux/swipe_item_menu_item_handler.cpp`](../../../src/handlers/linux/swipe_item_menu_item_handler.cpp)
  - Android: [`include/mpapp/handlers/android/swipe_item_menu_item_handler.hpp`](../../../include/mpapp/handlers/android/swipe_item_menu_item_handler.hpp) + [`src/handlers/android/swipe_item_menu_item_handler.cpp`](../../../src/handlers/android/swipe_item_menu_item_handler.cpp)
- Tests: [`tests/mock_handlers/swipe_item_menu_item_test.cpp`](../../../tests/mock_handlers/swipe_item_menu_item_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[SwipeView]]
- [[SwipeItemView]]
- [[MenuItem]]
