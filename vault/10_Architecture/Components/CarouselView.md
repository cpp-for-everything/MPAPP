---
type: component
mauiHandler: "CarouselView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/carouselview"
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

# CarouselView

> [!info] Status — android-real (real on Win + Linux + Android)
> Real per-platform handlers are in: WinUI `FlipView`, GTK4 `GtkStack` + swipe gesture, Android `ViewFlipper`. The `mpapp::carousel_view` wrapper (ADR-0024) + the `mpapp::internal::basic_carousel_view` surface + mock handler + tests all exist. macOS/iOS handlers are **blind-written** (compiled+run on a Mac: PENDING). The УИСС reference app exercises it (Информация-page announcements carousel, paged by ◀ ▶ buttons).

## Overview

`CarouselView` is MAUI's swipeable, paged item host — the sibling of [[CollectionView]]. It shows one (or a peeked few) item(s) at a time and lets the user swipe between them, with an optional looping wrap-around.

## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]], a concrete control has a `basic_<name>` surface + a `mpapp::<name>` wrapper. The `mpapp::carousel_view` wrapper embeds the platform-current handler by value and auto-binds it in its constructor (`map_items_source` / `map_position` / `map_loop` / `map_is_swipe_enabled` / `map_peek_count` / `map_gestures`), so app code reads `mpapp::carousel_view c; c.items_source = {...};` with no separate handler variable.

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

## Per-platform handlers (implemented)

| Platform | Native widget | Status | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.FlipView` | **real** | `SelectedIndex` ↔ `position`; `SelectionChanged` drives `position_changed`. Swipe is always touch-enabled (no public `IsSwipeEnabled`). |
| Linux | `GtkStack` + `GtkGestureSwipe` | **real** | One named page per item; horizontal fling → `scroll_to(±1)`. `Adw.Carousel` (peek/native-swipe) is a libadwaita follow-up. |
| Android | `android.widget.ViewFlipper` | **real** | Framework built-in (no androidx); `setDisplayedChild(position)`. Programmatic + tap-driven; `ViewPager2` fling paging is a follow-up. |
| macOS / iOS | `NSView` / `UIView` one-visible-page | **blind** | Compiled+run on a Mac: PENDING. `NSPageController` / `UIPageViewController` swipe is a follow-up. |

Cross-platform notes: `loop`/clamp is applied in `basic_carousel_view::scroll_to` (platform-neutral); `peek_count` shows a single page on every desktop/mobile widget in v1 (real peek-area insets are a per-platform follow-up).

## MAUI Reference

- **Control:** `references/maui/src/Controls/src/Core/Items/CarouselView.cs`
- **Docs:** [Microsoft .NET MAUI — CarouselView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/carouselview)

## Tests

`tests/mock_handlers/carousel_view_test.cpp` — 4 cases / 16 assertions: defaults, scroll_to clamp (loop off), scroll_to wrap (loop on), mock-handler mapper + simulate_swipe recording.
