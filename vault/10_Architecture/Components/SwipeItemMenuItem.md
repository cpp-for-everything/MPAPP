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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/swipeitemmenuitem/mock_test.cpp` (planned)
- Windows handler: `tests/components/swipeitemmenuitem/windows_test.cpp` (planned)
- Android handler: `tests/components/swipeitemmenuitem/android_test.cpp` (planned)
- Linux handler: `tests/components/swipeitemmenuitem/linux_test.cpp` (planned)
- macOS handler: `tests/components/swipeitemmenuitem/macos_test.cpp` (planned)
- iOS handler: `tests/components/swipeitemmenuitem/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Background type | `Paint?` (allows brushes) | `color` only | Native swipe-item widgets honour solid colors only | — |
| XAML element name | `<SwipeItem>` maps to `SwipeItemMenuItem` handler | Same XAML element, same handler name | Mirror MAUI naming asymmetry | — |
| `Command` type | `ICommand` | `Command<>` template | [[ADR-0009-public-api-template-wrappers-only]] | — |
| `IsDestructive` | Hints platforms to use destructive styling (red on iOS) | Same — iOS/macOS apply destructive role; other platforms ignore | Platform-native styling | — |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[SwipeView]]
- [[SwipeItemView]]
- [[MenuItem]]
