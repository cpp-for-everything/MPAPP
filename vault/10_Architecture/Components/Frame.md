---
type: component
mauiHandler: "Frame"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/frame"
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

# Frame

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Frame` is a single-child container with a colored border, a corner radius, and an optional drop shadow. It is the original Xamarin.Forms decorator and is now **obsolete as of .NET 9** — Microsoft's guidance is to use [[Border]] for all new code. MPAPP ports `Frame` for one-to-one XAML compatibility and migration from Forms / early MAUI codebases, but marks it deprecated in the same way: the MPAPP control compiles, behaves identically, and emits a deprecation diagnostic suggesting `border`. `Frame` derives from `ContentView`, so it inherits a `Content` property and a default `Padding` of 20.

## MAUI Reference

- **Handler:** *(no `FrameHandler` — `Frame` is rendered through the legacy compatibility pipeline; see `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Compatibility\Handlers\Android\FrameRenderer.cs`, `…\iOS\FrameRenderer.cs`, `…\Windows\FrameRenderer.cs`)*
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Frame\Frame.cs`
- **Docs:** [Microsoft .NET MAUI — Frame](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/frame)

`Frame` exposes three bindable properties beyond what `ContentView` gives it: `BorderColor`, `HasShadow` (default `true`), and `CornerRadius` (default `-1.0` meaning "platform default"). On iOS, `Frame` injects a hard-coded `Shadow` (`Radius=5, Opacity=0.8, Brush=Black`) when `HasShadow` is set and no explicit `Shadow` is on the view.

## MPAPP C++ API

```cpp
namespace mpapp {

// Deprecated in MPAPP, mirroring MAUI's .NET 9 deprecation.
// The real header carries [deprecated("Use mpapp::border instead.")] -
// the attribute is omitted here to keep this snippet wikilink-safe.
// Prefer mpapp::border for new code.
class frame : public content_view {
public:
    Observable<color>                      border_color;
    Observable<bool>                       has_shadow;          // default true
    Observable<float>                      corner_radius;       // -1 => platform default
    Observable<thickness>                  padding;             // default 20
};

} // namespace mpapp
```

## XAML Usage

```xml
<Frame BorderColor="Gray"
       CornerRadius="8"
       HasShadow="True"
       Padding="20">
    <Label Text="Legacy framed content" />
</Frame>
```

## Platform Notes

`Frame` reuses MAUI's legacy renderer pipeline. The mapping below reflects how MAUI realises the control today; MPAPP follows the same approach for parity.

| Platform | Native control                                                  | Header / source            | Notes |
|----------|-----------------------------------------------------------------|----------------------------|-------|
| Windows  | `Microsoft.UI.Xaml.Controls.Border` (inside legacy renderer)    | C++/WinRT                  | `HasShadow` is ignored historically; MPAPP matches. |
| Android  | `androidx.cardview.widget.CardView`                             | fbjni / JNI                | `HasShadow` -> `CardElevation`; `CornerRadius` -> `CardCornerRadius`. |
| Linux    | `GtkFrame` with CSS-styled border + drop shadow                 | gtk4-rs                    | Native GTK widget for legacy-frame parity. |
| macOS    | `NSBox` (AppKit) / `UIView` + `CALayer` shadow (Catalyst)       | AppKit / UIKit interop     | Catalyst path mirrors iOS. |
| iOS      | `UIKit.UIView` + `CALayer` shadow + border layer                | UIKit                      | MAUI hard-codes shadow at radius 5, opacity 0.8 when `HasShadow=true`. |

## Side-by-side Examples

### MAUI

```xml
<Frame BorderColor="Black" CornerRadius="10" HasShadow="True">
    <Label Text="Hello" />
</Frame>
```

### MPAPP (XAML)

```xml
<Frame BorderColor="Black" CornerRadius="10" HasShadow="True">
    <Label Text="Hello" />
</Frame>
```

### MPAPP (C++)

```cpp
auto f = mpapp::make<mpapp::frame>();
f->border_color  = mpapp::colors::black;
f->corner_radius = 10.0f;
f->has_shadow    = true;
f->content       = mpapp::make<mpapp::label>("Hello");

// Migration target:
auto b = mpapp::make<mpapp::border>();
b->stroke           = mpapp::colors::black;
b->stroke_thickness = 1.0;
b->stroke_shape     = mpapp::shapes::round_rectangle(10);
b->content          = mpapp::make<mpapp::label>("Hello");
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/frame/mock_test.cpp` (planned)
- Windows handler: `tests/components/frame/windows_test.cpp` (planned)
- Android handler: `tests/components/frame/android_test.cpp` (planned)
- Linux handler: `tests/components/frame/linux_test.cpp` (planned)
- macOS handler: `tests/components/frame/macos_test.cpp` (planned)
- iOS handler: `tests/components/frame/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Border]]
- [[ContentView]]
