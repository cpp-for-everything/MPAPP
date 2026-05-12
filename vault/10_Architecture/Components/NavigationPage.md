---
type: component
mauiHandler: "NavigationPage"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/navigationpage"
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

# NavigationPage

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`NavigationPage` is a stack-based navigation host. It manages a LIFO stack of [[Page]] instances and renders the top of the stack along with a navigation bar showing the page title and a back button. Apps push pages onto the stack with `PushAsync` and pop them with `PopAsync` / `PopToRootAsync`. It also exposes attached properties (`HasBackButton`, `HasNavigationBar`, `BackButtonTitle`, `TitleView`, `TitleIconImageSource`, `IconColor`) that individual pushed pages set to customize the bar while they are visible. For more complex routing — flyout + tabs + URI-based navigation — prefer [[Shell]].

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\NavigationPage\` (`NavigationViewHandler.cs`, plus `.Windows.cs`, `.Android.cs`, `.iOS.cs`). Implements `IStackNavigationView`.
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\NavigationPage\NavigationPage.cs` (+ `NavigationPage.Legacy.cs`, `NavigationPage.iOS.cs`, `NavigationPageToolbar.cs`)
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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/navigationpage/mock_test.cpp` (planned)
- Windows handler: `tests/components/navigationpage/windows_test.cpp` (planned)
- Android handler: `tests/components/navigationpage/android_test.cpp` (planned)
- Linux handler: `tests/components/navigationpage/linux_test.cpp` (planned)
- macOS handler: `tests/components/navigationpage/macos_test.cpp` (planned)
- iOS handler: `tests/components/navigationpage/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Navigation API surface | `INavigation` interface on every `Page`. | Navigation commands live on `navigation_page`; reached via `find_ancestor<>()`. | Avoids virtual-method dispatch tax on every `page`. | RFC TBD |
| iOS legacy renderer | Mixed Maui/Legacy iOS handler (`UseMauiHandler = false` on iOS/MacCatalyst). | Single handler per platform. | New codebase; no legacy debt. | n/a |
| `LayoutChildren` (obsolete) | Still present; returns early. | Not implemented (replaced by `arrange_override`). | Deprecated upstream. | n/a |
| Swipe-back | iOS only (native); other platforms toolbar-only. | Same — iOS native; others toolbar-only. | Platform conventions. | RFC TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Page]]
- [[ContentPage]]
- [[Shell]]
- [[Toolbar]]
