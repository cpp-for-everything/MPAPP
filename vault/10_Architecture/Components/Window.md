---
type: component
mauiHandler: "Window"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/window"
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

# Window

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Window` represents a top-level OS surface — a desktop window on Windows/Linux/macOS, an `Activity` on Android, and a `UIWindow` on iOS. It carries a single root [[Page]] (`Page`), plus geometry (`X`, `Y`, `Width`, `Height` with `Maximum*`/`Minimum*` bounds), title, flow direction, optional [[TitleBar]] (desktop only), [[Toolbar]], and [[MenuBar]]. `Window` derives from `NavigableElement` (a [[Element]] subclass) — it is *not* a `View` and has no visual layout slot of its own; instead the [[Handler]] maps `Window.Page` onto the native window's content surface and forwards platform commands like `RequestDisplayDensity`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Window\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Window\`
- **Docs:** [Microsoft .NET MAUI — Window](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/window)

## MPAPP C++ API

```cpp
namespace mpapp {

class window : public element<window> {
public:
    // The root page hosted by this window. Setting it triggers re-layout.
    Observable<std::shared_ptr<page>> page;

    Observable<std::string>           title;
    Observable<flow_direction>        flow_direction { flow_direction::match_parent };

    // Geometry. `unset` means "let the OS choose".
    Observable<double> x      { dimension::unset };
    Observable<double> y      { dimension::unset };
    Observable<double> width  { dimension::unset };
    Observable<double> height { dimension::unset };

    Observable<double> minimum_width  { dimension::minimum };
    Observable<double> minimum_height { dimension::minimum };
    Observable<double> maximum_width  { dimension::maximum };
    Observable<double> maximum_height { dimension::maximum };

    // Desktop-only chrome.
    Observable<std::shared_ptr<title_bar>> title_bar;
    Observable<bool>                       is_minimizable { true };
    Observable<bool>                       is_maximizable { true };

    // True once the OS has shown and focused the window.
    Computed<bool> is_activated;

    // Verbs.
    Command<double()> request_display_density; // returns scale factor (DIPs per px)
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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/window/mock_test.cpp` (planned)
- Windows handler: `tests/components/window/windows_test.cpp` (planned)
- Android handler: `tests/components/window/android_test.cpp` (planned)
- Linux handler: `tests/components/window/linux_test.cpp` (planned)
- macOS handler: `tests/components/window/macos_test.cpp` (planned)
- iOS handler: `tests/components/window/ios_test.cpp` (planned)

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `IsActivated` | Read-only bindable, internal setter | `Computed<bool>` driven by handler events | Cannot be assigned from user code — mirrors MAUI's intent more strictly | [[ADR-0009-public-api-template-wrappers-only]] |
| Geometry on mobile | Silently ignored | Logged at debug level; values still settable | Eases shared XAML between desktop and mobile | [[Interop Parity]] |
| `Page` cardinality | Single-page; modal stack lives on `Navigation` | Same | None | N/A |

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
