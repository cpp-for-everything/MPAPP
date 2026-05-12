---
type: component
mauiHandler: "IndicatorView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/indicatorview"
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

# IndicatorView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`IndicatorView` is a small page-indicator strip — a row of dots (or squares, or templated visuals) that shows how many items are in a paged collection and which one is currently visible. It is most commonly paired with [[CarouselView]] via the `IndicatorView` attached property on a CarouselView so positions stay in sync automatically. The control is purely presentational: it does not own the items, only their count and the current `Position`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\IndicatorView\IndicatorViewHandler.cs`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\IndicatorView\` (no dedicated folder — type lives next to CarouselView under `Microsoft.Maui.Controls.IndicatorView`)
- **Docs:** [Microsoft .NET MAUI — IndicatorView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/indicatorview)

## MPAPP C++ API

```cpp
namespace mpapp {

enum class indicator_shape { circle, square };

class indicatorview : public view<indicatorview> {
public:
    // Count is normally bound to ItemsSource.Count of a paired CarouselView.
    Observable<int>            count;
    Observable<int>            position;          // currently selected index
    Observable<double>         indicator_size;    // in DIPs
    Observable<int>            maximum_visible;   // -1 = unbounded
    Observable<bool>           hide_single;       // hide when count <= 1
    Observable<color>          indicator_color;
    Observable<color>          selected_indicator_color;
    Observable<indicator_shape> indicators_shape;

    // Optional: replace the dot with a custom template.
    Observable<data_template>  indicator_template;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<VerticalStackLayout>
    <CarouselView ItemsSource="{Binding Slides}"
                  IndicatorView="indicators"/>
    <IndicatorView x:Name="indicators"
                   IndicatorColor="LightGray"
                   SelectedIndicatorColor="DarkSlateGray"
                   IndicatorsShape="Circle"
                   HorizontalOptions="Center"/>
</VerticalStackLayout>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.Maui.Platform.MauiPageControl` (custom WinUI panel) | C++/WinRT | No native page-indicator; MPAPP draws ellipses/rectangles in a horizontal stack. |
| Android | `Microsoft.Maui.Platform.MauiPageControl` (custom `LinearLayout`) | fbjni / JNI | Drawn as a row of `View`s with rounded backgrounds. |
| Linux | `GtkBox` with `GtkDrawingArea` children | GTK4 | Custom drawn via Cairo to match the same dot/square shapes. |
| macOS | `NSPageControl` (macOS 14+) with custom fallback | AppKit | On older macOS, falls back to MPAPP-drawn dots. |
| iOS | `UIPageControl` | UIKit | Matches MAUI's `MauiPageControl` which subclasses `UIPageControl`. |

## Side-by-side Examples

### MAUI

```xml
<IndicatorView Count="{Binding Slides.Count}"
               Position="{Binding CurrentIndex}"
               IndicatorColor="Gray"
               SelectedIndicatorColor="Black"/>
```

### MPAPP (XAML)

```xml
<IndicatorView Count="{Binding Slides.Count}"
               Position="{Binding CurrentIndex}"
               IndicatorColor="Gray"
               SelectedIndicatorColor="Black"/>
```

### MPAPP (C++)

```cpp
auto iv = std::make_shared<mpapp::indicatorview>();
iv->count = 5;
iv->position = 0;
iv->indicators_shape = mpapp::indicator_shape::circle;
iv->hide_single = true;

// Sync with a CarouselView.
carousel->position.subscribe([iv](int p) { iv->position = p; });
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/indicatorview/mock_test.cpp` (planned)
- Windows handler: `tests/components/indicatorview/windows_test.cpp` (planned)
- Android handler: `tests/components/indicatorview/android_test.cpp` (planned)
- Linux handler: `tests/components/indicatorview/linux_test.cpp` (planned)
- macOS handler: `tests/components/indicatorview/macos_test.cpp` (planned)
- iOS handler: `tests/components/indicatorview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Color type | `Paint?` (allows gradients in theory) | `color` only | Native page controls only honor solid colors; matches MAUI's runtime behavior | — |
| `IndicatorTemplate` | Switches to `ITemplatedIndicatorView` path with a separate layout | Single `indicator_template` property — handler picks the templated path when non-null | Simpler observable model | Possible RFC if behavior diverges |
| Position binding | Two-way `BindableProperty` | Two-way `Observable<int>` — natural | Native to MPAPP | — |
| macOS native control | Uses iOS-style `UIPageControl` via Catalyst | Uses `NSPageControl` where available, falls back to MPAPP drawing | [[ADR-0005-ios-macos-separate-interop]] | — |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[CarouselView]]
