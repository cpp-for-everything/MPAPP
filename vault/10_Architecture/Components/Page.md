---
type: component
mauiHandler: "Page"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/page"
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

> [!info] Status
> **mock** — public surface (`title` / `content` / `is_busy`) + mock handler landed in [[T-0011-app-shell-abstraction]]. WinUI 3 real handler deferred to M-04 (the spike rewrite uses `mpapp::stack_layout` as the window content directly without an intermediate page; pages matter more for navigation, which lands later).

# Page

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Page` is the abstract base for any visual element that occupies an entire screen. It is the root of MAUI's page hierarchy ([[ContentPage]], [[NavigationPage]], [[FlyoutPage]], [[TabbedPage]], [[Shell]] all derive from it) and carries the cross-cutting concerns every full-screen surface needs: `Title`, `IconImageSource`, `BackgroundImageSource`, `Padding`, `ToolbarItems`, `MenuBarItems`, lifecycle events (`Appearing`, `Disappearing`, `NavigatedTo`, `NavigatedFrom`), and the modal-dialog helpers (`DisplayAlertAsync`, `DisplayActionSheetAsync`, `DisplayPromptAsync`). MPAPP follows the same shape: `mpapp::page` is an abstract class that user code never instantiates directly; concrete pages are subclasses.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_page` | [`include/mpapp/internal/basic_page.hpp`](../../../include/mpapp/internal/basic_page.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::page` | [`include/mpapp/page.hpp`](../../../include/mpapp/page.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/page.hpp>

mpapp::page w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/page.hpp>
#include <mpapp/handlers/mock/page_handler.hpp>

mpapp::internal::basic_page w;
mpapp::page_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::page_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::page_handler<>` and `mpapp::page_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Page\` (`PageHandler.cs`, `PageHandler.Windows.cs`, `PageHandler.Android.cs`, `PageHandler.iOS.cs`)
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Page\Page.cs`
- **Docs:** [Microsoft .NET MAUI — Page](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/page)

## MPAPP C++ API

```cpp
namespace mpapp {

// Abstract base for full-screen surfaces. Not directly instantiable;
// use content_page / navigation_page / flyout_page / tabbed_page / shell.
class page : public visual_element<page> {
public:
    // Title shown in flyout / tab bar / nav bar.
    Observable<std::u8string>            title;
    // Icon shown next to the title (flyout entry, tab item).
    Observable<image_source>             icon_image_source;
    // Full-screen background image painted behind content.
    Observable<image_source>             background_image_source;
    // Inner padding between page edge and content.
    Observable<thickness>                padding;
    // Editable collections; mutate observably.
    Observable<list<toolbar_item>>       toolbar_items;
    Observable<list<menu_bar_item>>      menu_bar_items;

    // Lifecycle events (raised by the navigation host).
    Event<>                              appearing;
    Event<>                              disappearing;
    Event<navigated_to_args>             navigated_to;
    Event<navigated_from_args>           navigated_from;
    Event<navigating_from_args>          navigating_from;

    // Modal helpers — return co_awaitable tasks.
    Command<task<void>(alert_options)>   display_alert;
    Command<task<bool>(confirm_options)> display_confirm;
    Command<task<std::u8string>(action_sheet_options)> display_action_sheet;
    Command<task<std::u8string>(prompt_options)>       display_prompt;

protected:
    page() = default; // abstract — subclass to use.
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Page itself is abstract — XAML always names a concrete subclass. -->
<!-- Common attached properties (Title, IconImageSource, Padding) live on the subclass. -->
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.Frame` content host | C++/WinRT | Title and toolbar items projected onto the containing `NavigationView` / `CommandBar`. |
| Android | `Fragment` + `ViewGroup` | fbjni / JNI | Lifecycle events bridge `Fragment.onResume` / `onPause`. Toolbar items map to `androidx.appcompat.widget.Toolbar` menu entries. |
| Linux | `GtkBox` inside the `GtkStack` page slot | GTK4 | Title surfaced via the parent `AdwHeaderBar`. `DisplayAlertAsync` uses `GtkAlertDialog`. |
| macOS | `NSViewController` | AppKit | `Title` binds to the window title bar when the page is the root. Action sheets use `NSAlert`. |
| iOS | `UIViewController` | UIKit | `Appearing/Disappearing` map to `viewWillAppear` / `viewWillDisappear`. Modal helpers route to `UIAlertController`. |

## Side-by-side Examples

### MAUI

```csharp
public class MyPage : ContentPage // (Page is abstract)
{
    public MyPage()
    {
        Title = "Settings";
        Padding = new Thickness(16);
        ToolbarItems.Add(new ToolbarItem { Text = "Save" });
    }

    async void OnDelete(object sender, EventArgs e)
    {
        bool ok = await DisplayAlert("Delete?", "This cannot be undone.", "Yes", "No");
    }
}
```

### MPAPP (XAML)

```xml
<ContentPage Title="Settings" Padding="16">
    <ContentPage.ToolbarItems>
        <ToolbarItem Text="Save"/>
    </ContentPage.ToolbarItems>
</ContentPage>
```

### MPAPP (C++)

```cpp
class my_page : public content_page {
public:
    my_page() {
        title         = u8"Settings";
        padding       = thickness{16};
        toolbar_items.mutate([](auto& v){ v.emplace_back(toolbar_item{.text = u8"Save"}); });
    }

    task<void> on_delete() {
        bool ok = co_await display_confirm({.title = u8"Delete?",
                                            .message = u8"This cannot be undone.",
                                            .accept = u8"Yes", .cancel = u8"No"});
    }
};
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Modal dialogs | `Task` returned by `DisplayAlert` etc. | `task<T>` (co_awaitable). | C++ coroutines, no managed `Task`. | RFC TBD |
| `IsBusy` | Bindable property (deprecated in .NET 11). | Omitted from public API. | Deprecated upstream; replaced with `ActivityIndicator`. | RFC TBD |
| `ContainerArea` | Obsolete `Rect` property. | Not exposed. | Internal/obsolete in MAUI. | n/a |
| Safe-area | `ISafeAreaView2.SafeAreaInsets` + iOS-only API. | Cross-platform `safe_area_edges` on every page. | Interop parity (Rule 2). | RFC TBD |

## Implementation

- Surface: [`include/mpapp/page.hpp`](../../../include/mpapp/page.hpp)
- Mock handler: [`include/mpapp/handlers/mock/page_handler.hpp`](../../../include/mpapp/handlers/mock/page_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/page_handler.hpp`](../../../include/mpapp/handlers/windows/page_handler.hpp) + [`src/handlers/windows/page_handler.cpp`](../../../src/handlers/windows/page_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/page_handler.hpp`](../../../include/mpapp/handlers/linux/page_handler.hpp) + [`src/handlers/linux/page_handler.cpp`](../../../src/handlers/linux/page_handler.cpp)
  - Android: [`include/mpapp/handlers/android/page_handler.hpp`](../../../include/mpapp/handlers/android/page_handler.hpp) + [`src/handlers/android/page_handler.cpp`](../../../src/handlers/android/page_handler.cpp)
- Tests: [`tests/mock_handlers/page_test.cpp`](../../../tests/mock_handlers/page_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ContentPage]]
- [[NavigationPage]]
- [[FlyoutPage]]
- [[TabbedPage]]
- [[Shell]]
