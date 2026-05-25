---
type: component
mauiHandler: "Frame"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/frame"
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

# Frame

> [!info] Status
> **mock** (deprecated) — cross-platform header at `include/mpapp/frame.hpp` carries the C++ `[[deprecated]]` attribute, mirroring MAUI .NET 9's deprecation. New code should use [[Border]]; the mock exists for one-to-one XAML migration parity. See [[Controls Inventory]].

## Overview

`Frame` is a single-child container with a colored border, a corner radius, and an optional drop shadow. It is the original Xamarin.Forms decorator and is now **obsolete as of .NET 9** — Microsoft's guidance is to use [[Border]] for all new code. MPAPP ports `Frame` for one-to-one XAML compatibility and migration from Forms / early MAUI codebases, but marks it deprecated in the same way: the MPAPP control compiles, behaves identically, and emits a deprecation diagnostic suggesting `border`. `Frame` derives from `ContentView`, so it inherits a `Content` property and a default `Padding` of 20.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_frame` | [`include/mpapp/internal/basic_frame.hpp`](../../../include/mpapp/internal/basic_frame.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::frame` | [`include/mpapp/frame.hpp`](../../../include/mpapp/frame.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/frame.hpp>

mpapp::frame w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/frame.hpp>
#include <mpapp/handlers/mock/frame_handler.hpp>

mpapp::internal::basic_frame w;
mpapp::frame_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::frame_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::frame_handler<>` and `mpapp::frame_handler<platform::mock>` valid spellings without naming `internal::`.

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

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Mock implementation

The P2 mock surface (ADR-0008) lands in this repository:

- **Cross-platform header:** `include/mpapp/frame.hpp` — `mpapp::frame : view` with `[[deprecated("mpapp::frame is deprecated; use mpapp::border instead.")]]`. Properties match the vault spec (`border_color`, `has_shadow`, `corner_radius`, `padding`, `content`) with MAUI defaults (`has_shadow=true`, `corner_radius=-1`, `padding=thickness{20}`).
- **Mock handler:** `include/mpapp/handlers/mock/frame_handler.hpp` — `frame_handler<platform::mock>` records every property mapper. Local pragmas suppress the deprecation diagnostic inside the handler since it IS the legacy path. The umbrella header `<mpapp/mpapp.hpp>` deliberately does NOT include `<frame.hpp>` — callers opt in explicitly.
- **Mock tests:** `tests/mock_handlers/frame_test.cpp`.

## Implementation

- Surface: [`include/mpapp/frame.hpp`](../../../include/mpapp/frame.hpp)
- Mock handler: [`include/mpapp/handlers/mock/frame_handler.hpp`](../../../include/mpapp/handlers/mock/frame_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/frame_handler.hpp`](../../../include/mpapp/handlers/windows/frame_handler.hpp) + [`src/handlers/windows/frame_handler.cpp`](../../../src/handlers/windows/frame_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/frame_handler.hpp`](../../../include/mpapp/handlers/linux/frame_handler.hpp) + [`src/handlers/linux/frame_handler.cpp`](../../../src/handlers/linux/frame_handler.cpp)
  - Android: [`include/mpapp/handlers/android/frame_handler.hpp`](../../../include/mpapp/handlers/android/frame_handler.hpp) + [`src/handlers/android/frame_handler.cpp`](../../../src/handlers/android/frame_handler.cpp)
- Tests: [`tests/mock_handlers/frame_test.cpp`](../../../tests/mock_handlers/frame_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Border]]
- [[ContentView]]
