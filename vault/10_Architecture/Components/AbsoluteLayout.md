---
type: component
mauiHandler: "AbsoluteLayout"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/layouts/absolutelayout"
mpappStatus: mock
platformWindows: false
platformAndroid: false
platformLinux: false
platformMacos: false
platformIos: false
tags:
  - type/component
  - status/mock
  - area/widgets
---

# AbsoluteLayout

> [!info] Status
> **mock + linux-real + android-real (compile-verified)** — surface + mock handler + Catch2 tests verified (1476-test suite green under g++ 14.2). **Real handlers implemented + compile-verified:** Linux `GtkFixed` handler compiles + links into `mpapp-handlers-linux` (WSL GTK4 4.14.5); Android `FrameLayout`+LayoutParams handler cross-compiles arm64+x86_64 (NDK 27.2). The earlier headers were declaration-only; the `.cpp` implementations landed this session ([[2026-W23-Weekly]]). **Remaining:** Windows (`mux::Canvas`, needs MSVC+WinUI), macOS/iOS (Apple host), and on-device runtime verification.

## Overview

`absolute_layout` positions each child explicitly via an attached
`layout_bounds` rectangle plus a `layout_flags` bitmask that selects which
rectangle components are interpreted **proportionally** (a 0..1 fraction of
the container) versus as **absolute** device-independent pixels. Mirrors
.NET MAUI's `AbsoluteLayout` and its `AbsoluteLayoutFlags`.

Inherits the child-list API from [[Layout]] — `add()`, `insert()`,
`remove()`, `clear()`. Placement is per-child attached state, mirroring
[[Grid]]'s cell-placement store ([[ADR-0017-grid-track-definitions]] /
[[ADR-0013-data-driven-widget-dispatch]]).

## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]]:

| Layer | Class | Header |
|---|---|---|
| Surface | `mpapp::internal::basic_absolute_layout` | [`include/mpapp/internal/basic_absolute_layout.hpp`](../../../include/mpapp/internal/basic_absolute_layout.hpp) |
| Wrapper | `mpapp::absolute_layout` | [`include/mpapp/absolute_layout.hpp`](../../../include/mpapp/absolute_layout.hpp) |

App code uses the wrapper; mock-handler tests stay on the surface +
`mpapp::absolute_layout_handler<mpapp::platform::mock>`.

## MAUI Reference

- **Control:** `references\maui\src\Controls\src\Core\Layout\AbsoluteLayout.cs`
- **Flags:** `references\maui\src\Controls\src\Core\Layout\AbsoluteLayoutFlags.cs`
- **Docs:** [Microsoft .NET MAUI — AbsoluteLayout](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/layouts/absolutelayout)

## MPAPP C++ API

```cpp
namespace mpapp {

struct rect { double x = 0, y = 0, width = 0, height = 0; };

enum class absolute_layout_flags : std::uint8_t {
    none = 0, x_proportional = 1, y_proportional = 2,
    width_proportional = 4, height_proportional = 8,
    position_proportional = 3,  // x | y
    size_proportional     = 12, // width | height
    all                   = 15,
};

class absolute_layout : public layout {
public:
    // Per-child attached placement (side map keyed on the child view*):
    void set_layout_bounds(view&, rect);
    void set_layout_flags(view&, absolute_layout_flags);
    rect get_layout_bounds(const view&) const;
    absolute_layout_flags get_layout_flags(const view&) const;
    // Inherited from layout: add/insert/remove/clear, Observable<thickness> padding.
};

} // namespace mpapp
```

## XAML Usage

```xml
<AbsoluteLayout>
    <BoxView AbsoluteLayout.LayoutBounds="0,0,0.5,0.25"
             AbsoluteLayout.LayoutFlags="All" />
    <Label AbsoluteLayout.LayoutBounds="20,20,200,40"
           AbsoluteLayout.LayoutFlags="None" Text="Pinned" />
</AbsoluteLayout>
```

## Platform Notes

| Platform | Native control | Notes |
|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.Canvas` | Handler written, unverified (no host). |
| Linux | `GtkFixed` | Handler written, unverified. |
| Android | absolute placement via JNI | Handler written, unverified. |
| macOS | `NSView` + constraints (planned) | Not written. |
| iOS | `UIView` + frames (planned) | Not written. |

## Implementation

- Surface: [`include/mpapp/internal/basic_absolute_layout.hpp`](../../../include/mpapp/internal/basic_absolute_layout.hpp)
- Wrapper: [`include/mpapp/absolute_layout.hpp`](../../../include/mpapp/absolute_layout.hpp)
- Mock handler: [`include/mpapp/handlers/mock/absolute_layout_handler.hpp`](../../../include/mpapp/handlers/mock/absolute_layout_handler.hpp)
- Dispatch: [`include/mpapp/handlers/absolute_layout_handler.hpp`](../../../include/mpapp/handlers/absolute_layout_handler.hpp)
- Real handlers (blind, unverified): `src`/`include` `handlers/{windows,linux,android}/absolute_layout_handler.hpp`
- Tests: [`tests/mock_handlers/absolute_layout_test.cpp`](../../../tests/mock_handlers/absolute_layout_test.cpp) — 13 cases / 44 assertions.

## Known Differences

| Aspect | MAUI | MPAPP |
|---|---|---|
| `LayoutBounds` type | `Rect` | `rect` (framework-owned POD) |
| `AutoSize` sentinel | `AbsoluteLayout.AutoSize` constant | not yet modeled (M-04 parity decision) |

## See also

- [[Controls Inventory]] · [[Layout]] · [[Grid]] · [[FlexLayout]] · [[StackLayout]]
- [[ADR-0008-mock-first-implementation]] · [[ADR-0024-wrapper-component-pattern]]
