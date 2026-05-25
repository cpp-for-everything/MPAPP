---
type: component
mauiHandler: "Window"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/window"
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

# Window

> [!info] Status
> **3-of-5 platforms real** — mock + WinUI 3 + GTK4 + Android (Activity content view via JNI) handlers all landed and **live-verified end-to-end** in [[T-0011-app-shell-abstraction]]. The simplified initial surface (title / content / width / height / is_visible + activated/closed signals) replaces the original spec; the richer geometry / titlebar / mode flags below remain on the M-04 docket. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Window` represents a top-level OS surface — a desktop window on Windows/Linux/macOS, an `Activity` on Android, and a `UIWindow` on iOS. It carries a single root [[Page]] (`Page`), plus geometry (`X`, `Y`, `Width`, `Height` with `Maximum*`/`Minimum*` bounds), title, flow direction, optional [[TitleBar]] (desktop only), [[Toolbar]], and [[MenuBar]]. `Window` derives from `NavigableElement` (a [[Element]] subclass) — it is *not* a `View` and has no visual layout slot of its own; instead the [[Handler]] maps `Window.Page` onto the native window's content surface and forwards platform commands like `RequestDisplayDensity`.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_window` | [`include/mpapp/internal/basic_window.hpp`](../../../include/mpapp/internal/basic_window.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::window` | [`include/mpapp/window.hpp`](../../../include/mpapp/window.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/window.hpp>

mpapp::window w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/window.hpp>
#include <mpapp/handlers/mock/window_handler.hpp>

mpapp::internal::basic_window w;
mpapp::window_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::window_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::window_handler<>` and `mpapp::window_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Window\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Window\`
- **Docs:** [Microsoft .NET MAUI — Window](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/window)

## MPAPP C++ API

Initial surface as shipped in T-0011. The richer geometry / chrome /
title-bar set follows in M-04.

```cpp
namespace mpapp {

class window : public control<window> {
public:
    // The root view hosted by this window. Non-owning pointer; the
    // user owns the lifetime (typically as a sibling field of the
    // mpapp::application subclass). The handler rebinds the native
    // window's content slot on change.
    Observable<view*>       content{nullptr};

    Observable<std::string> title{""};
    Observable<int>         width{0};       // 0 = let the OS choose
    Observable<int>         height{0};
    Observable<bool>        is_visible{false};

    mpapp::signal<>         activated;      // emitted after platform Activate
    mpapp::signal<>         closed;         // emitted on user / programmatic close

    void show();                            // sets is_visible = true
    void close();                           // sets is_visible = false + emits closed
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Window Title="My App" Width="1024" Height="768">
    <ContentPage>
        <Label Text="Hello"/>
    </ContentPage>
</Window>
```

The single child is `Page` (set as the `ContentProperty`).

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Window` | C++/WinRT | Full geometry, `TitleBar`, `IsMinimizable`/`IsMaximizable`, drag rectangles, and `FlowDirection` all supported. |
| Android | `android.app.Activity` | fbjni / JNI | Geometry and title-bar properties are ignored; one activity per logical window. |
| Linux | `GtkWindow` (under `GtkApplication`) | GTK4 | All geometry properties map; title bar uses `GtkHeaderBar`. |
| macOS | `NSWindow` | AppKit via [[Objective-Cpp]] | `Maximum*`/`Minimum*` and `TitleBar` mapped; same key constraints as MAUI's MacCatalyst build. |
| iOS | `UIWindow` | UIKit via [[Objective-Cpp]] | Geometry ignored on phone; multi-window only on iPadOS scenes. |

## Side-by-side Examples

### MAUI

```csharp
protected override Window CreateWindow(IActivationState? state)
    => new Window(new MainPage()) {
        Title = "Hello",
        Width = 1024,
        Height = 768,
    };
```

### MPAPP (XAML)

```xml
<Window Title="Hello" Width="1024" Height="768">
    <MainPage/>
</Window>
```

### MPAPP (C++)

```cpp
auto w = std::make_shared<mpapp::window>();
w->title  = "Hello";
w->width  = 1024;
w->height = 768;
w->page   = std::make_shared<MainPage>();
```

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `IsActivated` | Read-only bindable, internal setter | `Computed<bool>` driven by handler events | Cannot be assigned from user code — mirrors MAUI's intent more strictly | [[ADR-0009-public-api-template-wrappers-only]] |
| Geometry on mobile | Silently ignored | Logged at debug level; values still settable | Eases shared XAML between desktop and mobile | [[Interop Parity]] |
| `Page` cardinality | Single-page; modal stack lives on `Navigation` | Same | None | N/A |

## Implementation

- Surface: [`include/mpapp/window.hpp`](../../../include/mpapp/window.hpp)
- Mock handler: [`include/mpapp/handlers/mock/window_handler.hpp`](../../../include/mpapp/handlers/mock/window_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/window_handler.hpp`](../../../include/mpapp/handlers/windows/window_handler.hpp) + [`src/handlers/windows/window_handler.cpp`](../../../src/handlers/windows/window_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/window_handler.hpp`](../../../include/mpapp/handlers/linux/window_handler.hpp) + [`src/handlers/linux/window_handler.cpp`](../../../src/handlers/linux/window_handler.cpp)
  - Android: [`include/mpapp/handlers/android/window_handler.hpp`](../../../include/mpapp/handlers/android/window_handler.hpp) + [`src/handlers/android/window_handler.cpp`](../../../src/handlers/android/window_handler.cpp)
- Tests: [`tests/mock_handlers/window_test.cpp`](../../../tests/mock_handlers/window_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Application]]
- [[Element]]
- [[Page]]
- [[TitleBar]]
- [[Toolbar]]
