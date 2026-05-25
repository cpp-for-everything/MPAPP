---
type: component
mauiHandler: "NavigationPage"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/navigationpage"
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

# NavigationPage

> [!info] Status
> **android-real** — page_stack engine per [[ADR-0014-page-navigation-stack]] drives the real per-platform handlers. Windows wraps `mux::Controls::Page` with a Grid (back button + title row + content host). Linux uses a vertical GtkBox with a horizontal bar + content host. Android uses a vertical LinearLayout (bar + FrameLayout). The handler subscribes to `page_did_appear` and swaps the host's child via the ADR-0013 dispatch registry on every stack mutation. Back-button visibility tracks `stack_depth > 1`. macOS / iOS real handlers pending Apple host.

## Overview

`NavigationPage` is a stack-based navigation host. It manages a LIFO stack of [[Page]] instances and renders the top of the stack along with a navigation bar showing the page title and a back button. Apps push pages onto the stack with `PushAsync` and pop them with `PopAsync` / `PopToRootAsync`. It also exposes attached properties (`HasBackButton`, `HasNavigationBar`, `BackButtonTitle`, `TitleView`, `TitleIconImageSource`, `IconColor`) that individual pushed pages set to customize the bar while they are visible. For more complex routing — flyout + tabs + URI-based navigation — prefer [[Shell]].


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_navigation_page` | [`include/mpapp/internal/basic_navigation_page.hpp`](../../../include/mpapp/internal/basic_navigation_page.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::navigation_page` | [`include/mpapp/navigation_page.hpp`](../../../include/mpapp/navigation_page.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/navigation_page.hpp>

mpapp::navigation_page w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/navigation_page.hpp>
#include <mpapp/handlers/mock/navigation_page_handler.hpp>

mpapp::internal::basic_navigation_page w;
mpapp::navigation_page_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::navigation_page_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::navigation_page_handler<>` and `mpapp::navigation_page_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\NavigationPage\` (`NavigationViewHandler.cs`, plus `.Windows.cs`, `.Android.cs`, `.iOS.cs`). Implements `IStackNavigationView`.
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\NavigationPage\NavigationPage.cs` (+ `NavigationPage.Legacy.cs`, `NavigationPage.iOS.cs`, `NavigationPageToolbar.cs`)
- **Docs:** [Microsoft .NET MAUI — NavigationPage](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/navigationpage)

## MPAPP C++ API

```cpp
namespace mpapp {

class navigation_page : public page {
public:
    // Stack state (read-only from user code; mutated by push/pop commands).
    Observable<page*>           current_page;     // top of stack
    Observable<page*>           root_page;        // bottom of stack
    Observable<list<page*>>     navigation_stack; // all pages, root-first

    // Navigation bar appearance.
    Observable<color>           bar_background_color;
    Observable<brush>           bar_background;
    Observable<color>           bar_text_color;

    explicit navigation_page(page* root = nullptr);

    // Async navigation commands.
    Command<task<void>(page*, bool /*animated*/)>   push_async;
    Command<task<page*>(bool /*animated*/)>         pop_async;
    Command<task<void>(bool /*animated*/)>          pop_to_root_async;
    Command<void(page* /*before*/, page* /*page*/)> insert_page_before;
    Command<void(page*)>                            remove_page;

    // Events.
    Event<page*>                pushed;
    Event<page*>                popped;
    Event<>                     popped_to_root;

    // Per-pushed-page attached props (set on the child page, read by the host).
    static void  set_has_navigation_bar(page&, bool);
    static bool  get_has_navigation_bar(const page&);
    static void  set_has_back_button(page&, bool);
    static bool  get_has_back_button(const page&);
    static void  set_back_button_title(page&, std::u8string_view);
    static void  set_title_view(page&, view*);
    static void  set_title_icon_image_source(page&, image_source);
    static void  set_icon_color(page&, color);
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<NavigationPage>
    <x:Arguments>
        <local:HomePage/>
    </x:Arguments>
</NavigationPage>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.Frame` for stack + `NavigationView` header | C++/WinRT | Forward/back transitions use `EntranceNavigationTransitionInfo`. |
| Android | `androidx.fragment.app.FragmentManager` back stack + `Toolbar` | fbjni / JNI | Each push is a fragment transaction; hardware back maps to `pop_async`. |
| Linux | `GtkStack` with `crossfade` transition + `AdwHeaderBar` | GTK4 | No native swipe-back; pop only via toolbar or `pop_async()`. |
| macOS | Stack of `NSViewController` inside an `NSStackView` window content | AppKit | Title bar accessory renders the back button; toolbar items merge into the window toolbar. |
| iOS | `UINavigationController` | UIKit | Native swipe-from-edge back gesture works automatically. `title_view` binds to `navigationItem.titleView`. |

## Side-by-side Examples

### MAUI

```csharp
// App.xaml.cs
MainPage = new NavigationPage(new HomePage());

// HomePage
async void OnDetails(object sender, EventArgs e)
{
    await Navigation.PushAsync(new DetailsPage(id));
}
```

### MPAPP (XAML)

```xml
<NavigationPage>
    <x:Arguments>
        <local:HomePage/>
    </x:Arguments>
</NavigationPage>
```

### MPAPP (C++)

```cpp
// App start
auto nav = new navigation_page(new home_page());
get_window().page = nav;

// HomePage
task<void> home_page::on_details() {
    co_await find_ancestor<navigation_page>().push_async(new details_page(id), /*animated*/ true);
}
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Navigation API surface | `INavigation` interface on every `Page`. | Navigation commands live on `navigation_page`; reached via `find_ancestor<>()`. | Avoids virtual-method dispatch tax on every `page`. | RFC TBD |
| iOS legacy renderer | Mixed Maui/Legacy iOS handler (`UseMauiHandler = false` on iOS/MacCatalyst). | Single handler per platform. | New codebase; no legacy debt. | n/a |
| `LayoutChildren` (obsolete) | Still present; returns early. | Not implemented (replaced by `arrange_override`). | Deprecated upstream. | n/a |
| Swipe-back | iOS only (native); other platforms toolbar-only. | Same — iOS native; others toolbar-only. | Platform conventions. | RFC TBD |

## Implementation

- Surface: [`include/mpapp/navigation_page.hpp`](../../../include/mpapp/navigation_page.hpp)
- Mock handler: [`include/mpapp/handlers/mock/navigation_page_handler.hpp`](../../../include/mpapp/handlers/mock/navigation_page_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/navigation_page_handler.hpp`](../../../include/mpapp/handlers/windows/navigation_page_handler.hpp) + [`src/handlers/windows/navigation_page_handler.cpp`](../../../src/handlers/windows/navigation_page_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/navigation_page_handler.hpp`](../../../include/mpapp/handlers/linux/navigation_page_handler.hpp) + [`src/handlers/linux/navigation_page_handler.cpp`](../../../src/handlers/linux/navigation_page_handler.cpp)
  - Android: [`include/mpapp/handlers/android/navigation_page_handler.hpp`](../../../include/mpapp/handlers/android/navigation_page_handler.hpp) + [`src/handlers/android/navigation_page_handler.cpp`](../../../src/handlers/android/navigation_page_handler.cpp)
- Tests: [`tests/mock_handlers/navigation_page_test.cpp`](../../../tests/mock_handlers/navigation_page_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Page]]
- [[ContentPage]]
- [[Shell]]
- [[Toolbar]]
