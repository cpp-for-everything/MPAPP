---
type: component
mauiHandler: "CarouselView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/carouselview"
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

# CarouselView

> [!info] Status — mock
> The last MAUI control with no MPAPP counterpart, filled mock-first. The `mpapp::internal::basic_carousel_view` surface + mock handler + tests are in; per-platform real handlers are the standard follow-up (mock → platform-real).

## Overview

`CarouselView` is MAUI's swipeable, paged item host — the sibling of [[CollectionView]]. It shows one (or a peeked few) item(s) at a time and lets the user swipe between them, with an optional looping wrap-around.

## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]], a concrete control has a `basic_<name>` surface + a `mpapp::<name>` wrapper. At `mock` status only the **surface** (`internal::basic_carousel_view`) + the **mock handler** exist; the wrapper lands with the per-platform real handlers.

```cpp
namespace mpapp::internal {
class basic_carousel_view : public view {
public:
    Observable<std::vector<std::string>> items_source{};  // flat-string mock stand-in
    Observable<int>  position{0};            // current page (MAUI Position, two-way)
    Observable<bool> loop{true};             // wrap past the ends
    Observable<bool> is_swipe_enabled{true};
    Observable<int>  peek_count{0};          // peeked neighbours
    mpapp::signal<int> position_changed{};   // MAUI PositionChanged
    void scroll_to(int index);               // wraps (loop) / clamps
    std::size_t item_count() const;
};
}
```

`scroll_to` wraps the target index modulo `item_count()` when `loop` is set, else clamps to `[0, count-1]`, and fires `position_changed` only on a real change.

## Per-platform plan (follow-ups)

| Platform | Native widget |
|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.FlipView` |
| Linux | `Adw.Carousel` (libadwaita) or a `GtkStack` + swipe gesture |
| Android | `androidx.viewpager2.widget.ViewPager2` |
| macOS / iOS | `NSPageController` / `UIPageViewController` (blind, Apple-host pending) |

## MAUI Reference

- **Control:** `references/maui/src/Controls/src/Core/Items/CarouselView.cs`
- **Docs:** [Microsoft .NET MAUI — CarouselView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/carouselview)

## Tests

`tests/mock_handlers/carousel_view_test.cpp` — 4 cases / 16 assertions: defaults, scroll_to clamp (loop off), scroll_to wrap (loop on), mock-handler mapper + simulate_swipe recording.
