---
type: component
mauiHandler: "RefreshView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/refreshview"
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

# RefreshView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`RefreshView` is a container that wraps a scrollable child (typically a [[ScrollView]], [[ListView]], or [[CollectionView]]) and adds the standard "pull down to refresh" gesture seen across mobile platforms. When the user pulls past a threshold, `is_refreshing` flips to `true`, a spinner animates, and the bound `refresh_command` executes — the consumer is responsible for setting `is_refreshing` back to `false` when the refresh work completes. It is a single-content container and shares MAUI's `IRefreshView` interface verbatim.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_refresh_view` | [`include/mpapp/internal/basic_refresh_view.hpp`](../../../include/mpapp/internal/basic_refresh_view.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::refresh_view` | [`include/mpapp/refresh_view.hpp`](../../../include/mpapp/refresh_view.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/refresh_view.hpp>

mpapp::refresh_view w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/refresh_view.hpp>
#include <mpapp/handlers/mock/refresh_view_handler.hpp>

mpapp::internal::basic_refresh_view w;
mpapp::refresh_view_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::refresh_view_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::refresh_view_handler<>` and `mpapp::refresh_view_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\RefreshView\RefreshViewHandler.cs`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\RefreshView\` (type lives at `Microsoft.Maui.Controls.RefreshView`)
- **Docs:** [Microsoft .NET MAUI — RefreshView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/refreshview)

## MPAPP C++ API

```cpp
namespace mpapp {

class refreshview : public content_view<refreshview> {
public:
    // Two-way: set true to programmatically show the spinner, watch for
    // false-to-true transitions to detect a user-initiated pull.
    Observable<bool>      is_refreshing;

    // When false, the pull gesture is suppressed but content is interactive.
    Observable<bool>      is_refresh_enabled;

    // Spinner tint.
    Observable<color>     refresh_color;

    // Fired (and command executed) when the user releases the pull past the
    // platform threshold. The consumer must flip is_refreshing back to false.
    Command<>             refresh_command;
    event<>               refreshing;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<RefreshView IsRefreshing="{Binding IsBusy}"
             Command="{Binding LoadCommand}"
             RefreshColor="DodgerBlue">
    <CollectionView ItemsSource="{Binding Items}"/>
</RefreshView>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.RefreshContainer` + `RefreshVisualizer` | C++/WinRT | Pull direction defaults to top; tint via `RefreshVisualizer.Foreground`. |
| Android | `androidx.swiperefreshlayout.widget.SwipeRefreshLayout` (`MauiSwipeRefreshLayout`) | fbjni / JNI | Wraps any scrollable child; tint via `setColorSchemeColors`. |
| Linux | Custom `GtkOverlay` over the content with a `GtkSpinner`; gesture via `GtkGestureDrag` | GTK4 | No native pull-to-refresh widget in GTK; behavior is implemented in MPAPP. |
| macOS | Custom `NSView` overlay with `NSProgressIndicator`; gesture via `NSPanGestureRecognizer` on the inner scroll view | AppKit | Native macOS apps rarely use pull-to-refresh; MPAPP implements it for parity. |
| iOS | `UIRefreshControl` attached to the wrapped scroll view | UIKit | Standard system spinner; `tintColor` set from `refresh_color`. |

## Side-by-side Examples

### MAUI

```xml
<RefreshView IsRefreshing="{Binding IsBusy}" Command="{Binding RefreshCommand}">
    <ScrollView><Label Text="{Binding Status}"/></ScrollView>
</RefreshView>
```

### MPAPP (XAML)

```xml
<RefreshView IsRefreshing="{Binding IsBusy}" Command="{Binding RefreshCommand}">
    <ScrollView><Label Text="{Binding Status}"/></ScrollView>
</RefreshView>
```

### MPAPP (C++)

```cpp
auto rv = std::make_shared<mpapp::refreshview>();
rv->refresh_color = mpapp::colors::dodger_blue;
rv->content       = make_collection_view();

rv->refresh_command = mpapp::Command<>([rv]() {
    rv->is_refreshing = true;
    fetch_async().then([rv](auto) { rv->is_refreshing = false; });
});
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `RefreshColor` type | `Paint?` | `color` — only solid colors honored by native | Matches native capability | — |
| Linux native widget | Not supported (no MAUI Linux head) | Custom MPAPP overlay | GTK has no equivalent | — |
| macOS gesture | Catalyst inherits `UIRefreshControl` | AppKit-native gesture + spinner | [[ADR-0005-ios-macos-separate-interop]] | — |
| `IsRefreshEnabled` default | `true` (default-implemented) | `true` | — | — |

## Implementation

- Surface: [`include/mpapp/refresh_view.hpp`](../../../include/mpapp/refresh_view.hpp)
- Mock handler: [`include/mpapp/handlers/mock/refresh_view_handler.hpp`](../../../include/mpapp/handlers/mock/refresh_view_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/refresh_view_handler.hpp`](../../../include/mpapp/handlers/windows/refresh_view_handler.hpp) + [`src/handlers/windows/refresh_view_handler.cpp`](../../../src/handlers/windows/refresh_view_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/refresh_view_handler.hpp`](../../../include/mpapp/handlers/linux/refresh_view_handler.hpp) + [`src/handlers/linux/refresh_view_handler.cpp`](../../../src/handlers/linux/refresh_view_handler.cpp)
  - Android: [`include/mpapp/handlers/android/refresh_view_handler.hpp`](../../../include/mpapp/handlers/android/refresh_view_handler.hpp) + [`src/handlers/android/refresh_view_handler.cpp`](../../../src/handlers/android/refresh_view_handler.cpp)
- Tests: [`tests/mock_handlers/refresh_view_test.cpp`](../../../tests/mock_handlers/refresh_view_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ListView]]
- [[CollectionView]]
- [[ScrollView]]
