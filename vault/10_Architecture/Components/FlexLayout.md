---
type: component
mauiHandler: "FlexLayout"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/layouts/flexlayout"
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

# FlexLayout

> [!info] Status
> **mock + linux-real + android-real (compile-verified, v1)** — surface + mock handler + Catch2 tests verified (1476-test suite green under g++ 14.2). **Real handlers implemented + compile-verified:** Linux `GtkBox` v1 handler links into `mpapp-handlers-linux` (WSL GTK4); Android `LinearLayout` v1 handler cross-compiles arm64+x86_64 (NDK 27.2). The v1 handlers map flex properties onto GtkBox/LinearLayout semantics — a **faithful CSS-flexbox solver** (e.g. Yoga / `com.google.android.flexbox`) is the substantive follow-up, alongside the Windows handler (MSVC+WinUI), macOS/iOS (Apple host), and on-device runtime verification. See [[2026-W23-Weekly]].

## Overview

`flex_layout` arranges children using CSS-flexbox semantics. Mirrors .NET
MAUI's `FlexLayout` (itself a port of Facebook's CSS flexbox). Container
properties control the main/cross axis flow; per-child attached properties
(`order`, `grow`, `shrink`, `basis`, `align_self`) tune individual items.

Inherits the child-list API from [[Layout]].

## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]]:

| Layer | Class | Header |
|---|---|---|
| Surface | `mpapp::internal::basic_flex_layout` | [`include/mpapp/internal/basic_flex_layout.hpp`](../../../include/mpapp/internal/basic_flex_layout.hpp) |
| Wrapper | `mpapp::flex_layout` | [`include/mpapp/flex_layout.hpp`](../../../include/mpapp/flex_layout.hpp) |

## MAUI Reference

- **Control:** `references\maui\src\Controls\src\Core\Layout\FlexLayout.cs`
- **Docs:** [Microsoft .NET MAUI — FlexLayout](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/layouts/flexlayout)

## MPAPP C++ API

```cpp
namespace mpapp {

enum class flex_direction     : std::uint8_t { row, row_reverse, column, column_reverse };
enum class flex_wrap          : std::uint8_t { no_wrap, wrap, wrap_reverse };
enum class flex_justify       : std::uint8_t { start, center, end, space_between, space_around, space_evenly };
enum class flex_align_items   : std::uint8_t { stretch, center, start, end };
enum class flex_align_content : std::uint8_t { stretch, center, start, end, space_between, space_around };
enum class flex_align_self    : std::uint8_t { auto_, stretch, center, start, end };
enum class flex_position      : std::uint8_t { relative, absolute };

class flex_layout : public layout {
public:
    Observable<flex_direction>     direction{flex_direction::row};
    Observable<flex_wrap>          wrap{flex_wrap::no_wrap};
    Observable<flex_justify>       justify_content{flex_justify::start};
    Observable<flex_align_items>   align_items{flex_align_items::stretch};
    Observable<flex_align_content> align_content{flex_align_content::stretch};
    Observable<flex_position>      position{flex_position::relative};

    // Per-child attached props (side map keyed on the child view*):
    void set_order(view&, int);        // default 0
    void set_grow(view&, double);      // default 0
    void set_shrink(view&, double);    // default 1
    void set_align_self(view&, flex_align_self);
    void set_basis(view&, double);     // -1 == auto
    child_props get_child_props(const view&) const;
};

} // namespace mpapp
```

## XAML Usage

```xml
<FlexLayout Direction="Row" Wrap="Wrap" JustifyContent="SpaceBetween" AlignItems="Center">
    <Label Text="A" FlexLayout.Grow="1" />
    <Label Text="B" FlexLayout.Order="-1" FlexLayout.AlignSelf="Start" />
</FlexLayout>
```

## Platform Notes

| Platform | Native approach | Notes |
|---|---|---|
| Windows | custom arrange / `Canvas` | Handler written, unverified (no host). |
| Linux | custom arrange on `GtkFixed`/`GtkBox` | Handler written, unverified. |
| Android | custom layout via JNI | Handler written, unverified. |
| macOS / iOS | custom arrange (planned) | Not written. |

> [!note] Real-handler depth
> MAUI vendors a full flexbox solver. The MPAPP real handlers currently map
> the container + child props to the native container; a faithful flexbox
> arrange engine is the substantive follow-up before `<platform>-real`.

## Implementation

- Surface: [`include/mpapp/internal/basic_flex_layout.hpp`](../../../include/mpapp/internal/basic_flex_layout.hpp)
- Wrapper: [`include/mpapp/flex_layout.hpp`](../../../include/mpapp/flex_layout.hpp)
- Mock handler: [`include/mpapp/handlers/mock/flex_layout_handler.hpp`](../../../include/mpapp/handlers/mock/flex_layout_handler.hpp)
- Real handlers (blind, unverified): `handlers/{windows,linux,android}/flex_layout_handler.hpp`
- Tests: [`tests/mock_handlers/flex_layout_test.cpp`](../../../tests/mock_handlers/flex_layout_test.cpp) — 11 cases / 76 assertions.

## Known Differences

| Aspect | MAUI | MPAPP |
|---|---|---|
| `Basis` type | `FlexBasis` (auto / % / px struct) | `double` with `-1` == auto (parity simplification for M-04 mock) |
| flexbox solver | full Facebook Yoga-style solver | native-container mapping; full solver is a follow-up |

## See also

- [[Controls Inventory]] · [[Layout]] · [[Grid]] · [[AbsoluteLayout]] · [[StackLayout]]
- [[ADR-0008-mock-first-implementation]] · [[ADR-0024-wrapper-component-pattern]]
