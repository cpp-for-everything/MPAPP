---
type: component
mauiHandler: "TitleBar"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/titlebar"
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

# TitleBar

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`TitleBar` is a custom-content replacement for the OS window title bar (Windows + macOS Catalyst). It exposes six content regions — `icon`, `leading_content`, `title`, `subtitle`, `content`, `trailing_content` — plus a foreground color and a `passthrough_elements` list of inner views that handle their own input instead of being treated as drag targets. The default 32 px height can be overridden, and hiding the title bar extends window content into the chrome region.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_title_bar` | [`include/mpapp/internal/basic_title_bar.hpp`](../../../include/mpapp/internal/basic_title_bar.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::title_bar` | [`include/mpapp/title_bar.hpp`](../../../include/mpapp/title_bar.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/title_bar.hpp>

mpapp::title_bar w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/title_bar.hpp>
#include <mpapp/handlers/mock/title_bar_handler.hpp>

mpapp::internal::basic_title_bar w;
mpapp::title_bar_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::title_bar_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::title_bar_handler<>` and `mpapp::title_bar_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** MAUI ships no dedicated `TitleBarHandler`; the control is hosted directly by `Window` and rendered via templated views.
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\TitleBar\TitleBar.cs`
- **Docs:** [Microsoft .NET MAUI — TitleBar](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/titlebar)

`TitleBar : TemplatedView, ITitleBar, ISafeAreaView` has bindable properties for `Icon`, `LeadingContent`, `Content`, `TrailingContent`, `Title`, `Subtitle`, and `ForegroundColor`. Visual-state groups (`TitleBarTitleActive` / `Inactive`, `TitleBarLeftToRight` / `RightToLeft`, plus per-slot visible/collapsed groups) animate the bar.

## MPAPP C++ API

```cpp
namespace mpapp {

class title_bar : public templated_view<title_bar> {
public:
    Observable<image_source> icon;

    Observable<view>         leading_content;
    Observable<view>         content;
    Observable<view>         trailing_content;

    Observable<std::string>  title;
    Observable<std::string>  subtitle;

    Observable<color>        foreground_color;

    // Read-only: views that should bypass title-bar drag and handle input directly.
    Observable<observable_list<view>> passthrough_elements;
};

} // namespace mpapp
```

Visibility is driven by [[Observable Properties]] — setting `title` to empty triggers the `TitleHidden` visual state automatically, matching MAUI's `OnTitleChanged` behaviour.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Window.TitleBar>
    <TitleBar Title="My App" Subtitle="Untitled"
              Icon="appicon.png"
              ForegroundColor="White">
        <TitleBar.LeadingContent>
            <Button Text="Back"/>
        </TitleBar.LeadingContent>
        <TitleBar.TrailingContent>
            <Button Text="Profile"/>
        </TitleBar.TrailingContent>
    </TitleBar>
</Window.TitleBar>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Window.SetTitleBar` + custom drag region | C++/WinRT | MAUI's reference implementation; reserves a 150-px right margin for system buttons. |
| Android | Not applicable (no window chrome) | fbjni / JNI | `TitleBar` collapses into a no-op; consider [[Toolbar]] for action-bar surfaces. |
| Linux | `GtkHeaderBar` set as `gtk_window_set_titlebar` | GTK4 | Six regions map to start / center / end slots; system buttons handled by the WM. |
| macOS | `NSWindow.titlebarAccessoryViewControllers` + Mac Catalyst extension | AppKit | Reserves an 80-px (90-px on macOS 26+ Liquid Glass) leading margin for traffic-light buttons. |
| iOS | Not applicable | UIKit | Hidden; navigation chrome lives in [[Toolbar]]. |

## Side-by-side Examples

### MAUI

```xml
<Window>
    <Window.TitleBar>
        <TitleBar Title="Notes" Icon="logo.png" ForegroundColor="White">
            <TitleBar.Content>
                <SearchBar Placeholder="Search"/>
            </TitleBar.Content>
        </TitleBar>
    </Window.TitleBar>
</Window>
```

### MPAPP (XAML)

```xml
<Window>
    <Window.TitleBar>
        <TitleBar Title="Notes" Icon="logo.png" ForegroundColor="White">
            <TitleBar.Content>
                <SearchBar Placeholder="Search"/>
            </TitleBar.Content>
        </TitleBar>
    </Window.TitleBar>
</Window>
```

### MPAPP (C++)

```cpp
auto tb = mpapp::title_bar{
    .icon = mpapp::image_source::from_file("logo.png"),
    .title = "Notes",
    .foreground_color = mpapp::colors::white,
};
tb.content = mpapp::search_bar{ .placeholder = "Search" };
window.title_bar = std::move(tb);
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Visual states | `VisualStateManager` groups (`Active`/`Inactive`, LTR/RTL, per-slot visibility) | Drived from observable property values; handlers translate to native window states | No `VisualStateManager` runtime in MPAPP MVP | RFC TBD |
| Default template | Built imperatively in `BuildDefaultTemplate()` | Built declaratively in a XAML template stub | Reuses MPAPP-XAML markup | n/a |
| Android / iOS support | No-op (TBD upstream) | Same; documented as desktop-only | OS constraints | n/a |
| System button margin | Hard-coded constants (150 px Win, 80–90 px macOS) | Same constants; exposed as compile-time platform traits | Faithful port | n/a |

## Implementation

- Surface: [`include/mpapp/title_bar.hpp`](../../../include/mpapp/title_bar.hpp)
- Mock handler: [`include/mpapp/handlers/mock/title_bar_handler.hpp`](../../../include/mpapp/handlers/mock/title_bar_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/title_bar_handler.hpp`](../../../include/mpapp/handlers/windows/title_bar_handler.hpp) + [`src/handlers/windows/title_bar_handler.cpp`](../../../src/handlers/windows/title_bar_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/title_bar_handler.hpp`](../../../include/mpapp/handlers/linux/title_bar_handler.hpp) + [`src/handlers/linux/title_bar_handler.cpp`](../../../src/handlers/linux/title_bar_handler.cpp)
  - Android: [`include/mpapp/handlers/android/title_bar_handler.hpp`](../../../include/mpapp/handlers/android/title_bar_handler.hpp) + [`src/handlers/android/title_bar_handler.cpp`](../../../src/handlers/android/title_bar_handler.cpp)
- Tests: [`tests/mock_handlers/title_bar_test.cpp`](../../../tests/mock_handlers/title_bar_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Toolbar]]
- [[MenuBar]]
