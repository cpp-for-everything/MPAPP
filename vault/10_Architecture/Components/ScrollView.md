---
type: component
mauiHandler: "ScrollView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/scrollview"
mpappStatus: mock
platformWindows: false
platformAndroid: false
platformLinux: false
platformMacos: false
platformIos: false
tags:
  - type/component
  - status/mock
---

# ScrollView

> [!info] Status
> **mock** — cross-platform header at `include/mpapp/scroll_view.hpp`; mock handler records orientation, scrollbar-visibility, content presence, and the `scroll_to` command request. See [[Controls Inventory]].

## Overview

`ScrollView` is a single-child container that lets the user pan its content when the content is larger than the available viewport. It supports vertical, horizontal, both, or neutral orientations and exposes per-axis scrollbar-visibility policies. The control raises `Scrolled` events as the offset changes and offers an awaitable `ScrollToAsync` API that scrolls to a coordinate, an element, or a named position (Start, Center, End, MakeVisible). MAUI's `ScrollViewHandler` maps directly to a `UIScrollView` on iOS, a `MauiScrollView` on Android, and a `Microsoft.UI.Xaml.Controls.ScrollViewer` on Windows.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\ScrollView\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\ScrollView\`
- **Docs:** [Microsoft .NET MAUI — ScrollView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/scrollview)

`ScrollViewHandler.Mapper` adds `Content`, `HorizontalScrollBarVisibility`, `VerticalScrollBarVisibility`, and `Orientation` to the inherited `ViewMapper`. The `CommandMapper` adds `RequestScrollTo`. The control exposes read-only `ScrollX` / `ScrollY` and the `Scrolled` event.

## MPAPP C++ API

```cpp
namespace mpapp {

class scroll_view : public view {
public:
    // Content
    Observable<std::shared_ptr<view>>      content;

    // Behavior
    Observable<scroll_orientation>         orientation;                   // vertical (default) | horizontal | both | neither
    Observable<scroll_bar_visibility>      horizontal_scroll_bar_visibility; // default | always | never
    Observable<scroll_bar_visibility>      vertical_scroll_bar_visibility;

    // Read-only current offsets (set by the handler).
    Observable<double>                     scroll_x;
    Observable<double>                     scroll_y;

    // Commands
    Command<scroll_to_request>             scroll_to;     // x/y or element + position + animated

    // Events
    Event<scrolled_args>                   scrolled;      // fires on offset change
};

} // namespace mpapp
```

## XAML Usage

```xml
<ScrollView Orientation="Vertical"
            VerticalScrollBarVisibility="Always">
    <VerticalStackLayout Padding="16" Spacing="8">
        <Label Text="Item 1" />
        <Label Text="Item 2" />
        <!-- ... -->
    </VerticalStackLayout>
</ScrollView>
```

## Platform Notes

| Platform | Native control                                | Header / source            | Notes |
|----------|-----------------------------------------------|----------------------------|-------|
| Windows  | `Microsoft.UI.Xaml.Controls.ScrollViewer`     | C++/WinRT                  | `Orientation` maps to `HorizontalScrollMode` / `VerticalScrollMode`. |
| Android  | `android.widget.HorizontalScrollView` + `android.widget.ScrollView` (custom `MauiScrollView`) | fbjni / JNI | MAUI wraps both axes in a single composite to support `Both`. |
| Linux    | `GtkScrolledWindow`                           | gtk4-rs                    | Hosts a single child via `gtk_scrolled_window_set_child`. |
| macOS    | `NSScrollView` (AppKit) / `UIScrollView` (Catalyst) | AppKit / UIKit interop | Catalyst path uses `UIScrollView`. |
| iOS      | `UIKit.UIScrollView`                          | UIKit                      | `ContentSize` is set from the cross-platform measure result. |

## Side-by-side Examples

### MAUI

```xml
<ScrollView>
    <VerticalStackLayout>
        <Label Text="Top" />
        <Label Text="Bottom" />
    </VerticalStackLayout>
</ScrollView>
```

### MPAPP (XAML)

```xml
<ScrollView>
    <VerticalStackLayout>
        <Label Text="Top" />
        <Label Text="Bottom" />
    </VerticalStackLayout>
</ScrollView>
```

### MPAPP (C++)

```cpp
auto stack = mpapp::make<mpapp::vertical_stack_layout>();
stack->children.add(mpapp::make<mpapp::label>("Top"));
stack->children.add(mpapp::make<mpapp::label>("Bottom"));

auto sv = mpapp::make<mpapp::scroll_view>();
sv->content     = stack;
sv->orientation = mpapp::scroll_orientation::vertical;
sv->scrolled.subscribe([](const auto& args) {
    // args.scroll_x, args.scroll_y
});
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/scrollview/mock_test.cpp` (planned)
- Windows handler: `tests/components/scrollview/windows_test.cpp` (planned)
- Android handler: `tests/components/scrollview/android_test.cpp` (planned)
- Linux handler: `tests/components/scrollview/linux_test.cpp` (planned)
- macOS handler: `tests/components/scrollview/macos_test.cpp` (planned)
- iOS handler: `tests/components/scrollview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Mock implementation

The P2 mock surface (ADR-0008) lands in this repository:

- **Cross-platform header:** `include/mpapp/scroll_view.hpp` — `mpapp::scroll_view : view` with `Observable<std::shared_ptr<view>> content`, the orientation / scrollbar-visibility enums, read-only `scroll_x`/`scroll_y` observables, and a `scroll_to(scroll_to_request, Command<scroll_to_request>)` method.
- **Mock handler:** `include/mpapp/handlers/mock/scroll_view_handler.hpp` — `scroll_view_handler<platform::mock>` records property mappers and the scroll-to command (x, y, animated).
- **Mock tests:** `tests/mock_handlers/scroll_view_test.cpp`.

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[View]]
- [[Layout]]
