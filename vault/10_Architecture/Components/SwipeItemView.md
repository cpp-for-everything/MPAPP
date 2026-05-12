---
type: component
mauiHandler: "SwipeItemView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/swipeview"
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

# SwipeItemView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`SwipeItemView` is a [[SwipeView]] action that hosts arbitrary content — any [[View]] — instead of the fixed icon+text shape of [[SwipeItemMenuItem]]. It lets you build fully custom swipe actions (e.g. a tinted panel with a progress ring, or a multi-button cluster) while still participating in SwipeView's gesture, invocation, and `swipe_behavior_on_invoked` lifecycle. It implements both `IContentView` and `ISwipeItem`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\SwipeItemView\SwipeItemViewHandler.cs`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\SwipeView\SwipeItemView.cs`
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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/swipeitemview/mock_test.cpp` (planned)
- Windows handler: `tests/components/swipeitemview/windows_test.cpp` (planned)
- Android handler: `tests/components/swipeitemview/android_test.cpp` (planned)
- Linux handler: `tests/components/swipeitemview/linux_test.cpp` (planned)
- macOS handler: `tests/components/swipeitemview/macos_test.cpp` (planned)
- iOS handler: `tests/components/swipeitemview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Inheritance | `ContentView, ISwipeItem` | `content_view<swipe_item_view>` implementing the swipe-item contract | C++ has no multiple-interface inheritance via attributes | — |
| Windows hosting | WinUI `SwipeItem` only supports icon/text → MAUI hosts custom content under the hood | Same approach, documented explicitly | Matches MAUI runtime | — |
| Command type | `ICommand` | `Command<>` template | [[ADR-0009-public-api-template-wrappers-only]] | — |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[SwipeView]]
- [[SwipeItemMenuItem]]
