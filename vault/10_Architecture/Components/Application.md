---
type: component
mauiHandler: "Application"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/application"
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

# Application

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Application` is the singleton entry point and lifecycle owner for an MPAPP app. It owns the collection of [[Window|Windows]], the global resource dictionary, the theme (`light`/`dark`/`unspecified`), and the `Quit`/`OpenWindow`/`CloseWindow`/`ActivateWindow` verbs. Unlike a visual control it has no `PlatformView` in the rendering sense — its handler binds to the native application object (`Microsoft.UI.Xaml.Application` on Windows, `android.app.Application`, `IUIApplicationDelegate` on iOS, etc.) and exists only to broker lifecycle and theming. Apps subclass `application` and override `create_window()` to produce the initial [[Window]] (the modern replacement for the obsolete `MainPage`).

In MAUI's hierarchy, `Application` derives from `Element` and is the root parent of all `Window`s — every `View` ultimately reaches it through `Parent.Window.Application`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Application\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Application\`
- **Docs:** [Microsoft .NET MAUI — Application](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/application)

## MPAPP C++ API

```cpp
namespace mpapp {

class application : public element<application> {
public:
    // The currently running application instance. Set in the constructor.
    static application* current();

    // All windows currently owned by the application. Updated by open_window/close_window.
    Observable<std::vector<std::shared_ptr<window>>> windows;

    // Application-level resource dictionary (styles, brushes, templates).
    Observable<resource_dictionary> resources;

    // User-requested theme override; falls back to platform_app_theme when unspecified.
    Observable<app_theme> user_app_theme { app_theme::unspecified };

    // The OS-reported theme. Read-only — driven by platform notifications.
    Observable<app_theme> platform_app_theme { app_theme::unspecified };

    // Computed: user_app_theme if set, else platform_app_theme.
    Computed<app_theme> requested_theme;

    // Verbs.
    Command<>                            quit;
    Command<void(std::shared_ptr<window>)> open_window;
    Command<void(std::shared_ptr<window>)> close_window;
    Command<void(std::shared_ptr<window>)> activate_window;

protected:
    // Override to produce the initial window when the OS starts the app.
    // Equivalent to MAUI's Application.CreateWindow.
    virtual std::shared_ptr<window> create_window(activation_state state) = 0;
};

} // namespace mpapp
```

There is no static `set_current_application` — assignment happens implicitly in the constructor and is exposed read-only through `current()` to discourage swapping at runtime.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Application xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             x:Class="MyApp.App">
    <Application.Resources>
        <ResourceDictionary>
            <Color x:Key="Accent">#1E90FF</Color>
        </ResourceDictionary>
    </Application.Resources>
</Application>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Application` | C++/WinRT | Theme changes flow via `OnRequestedThemeChangedPlatform`. |
| Android | `android.app.Application` | fbjni / JNI | App theme is observed through `Configuration.uiMode`. |
| Linux | `GtkApplication` | GTK4 | `g_application_run` drives the main loop; window opening goes through `gtk_application_add_window`. |
| macOS | `NSApplication` (`NSApplicationDelegate`) | AppKit via [[Objective-Cpp]] | Theme observed via `effectiveAppearance`. |
| iOS | `IUIApplicationDelegate` | UIKit via [[Objective-Cpp]] | `OpenWindow`/`CloseWindow` requires iOS 13+ scene support. |

## Side-by-side Examples

### MAUI

```csharp
public partial class App : Application
{
    protected override Window CreateWindow(IActivationState? activationState)
        => new Window(new MainPage()) { Title = "Hello" };
}
```

### MPAPP (XAML)

```xml
<Application x:Class="MyApp.App">
    <Application.Resources>
        <Color x:Key="Accent">#1E90FF</Color>
    </Application.Resources>
</Application>
```

### MPAPP (C++)

```cpp
class my_app : public mpapp::application {
public:
    std::shared_ptr<mpapp::window> create_window(mpapp::activation_state) override {
        auto w = std::make_shared<mpapp::window>();
        w->title = "Hello";
        w->page = std::make_shared<MainPage>();
        return w;
    }
};

int main(int argc, char** argv) {
    return mpapp::host::run<my_app>(argc, argv);
}
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/application/mock_test.cpp` (planned)
- Windows handler: `tests/components/application/windows_test.cpp` (planned)
- Android handler: `tests/components/application/android_test.cpp` (planned)
- Linux handler: `tests/components/application/linux_test.cpp` (planned)
- macOS handler: `tests/components/application/macos_test.cpp` (planned)
- iOS handler: `tests/components/application/ios_test.cpp` (planned)

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `MainPage` property | Still present, deprecated; mutates `Windows[0].Page` | Not implemented — only `create_window()` | Avoids deprecated path entirely from day one | N/A (cleaner break) |
| `Current` getter | Mutable static, can be reassigned | Read-only, assigned in constructor | Reassigning at runtime is almost always a bug | TBD |
| Multi-window on mobile | Throws on platforms without scene support | Returns an error future from `open_window` | Aligns with [[Interop Parity]] — observable behavior on all platforms | TBD |
| Theme model | `AppTheme` (`Unspecified`/`Light`/`Dark`) | Same enum, but `requested_theme` is a [[Observable-Property\|Computed]] property | Lets theme bind directly into XAML without an event handler | [[ADR-0009-public-api-template-wrappers-only]] |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Element]]
- [[Window]]
- [[Page]]
