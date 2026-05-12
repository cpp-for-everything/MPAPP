---
type: component
mauiHandler: "ContentPage"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/contentpage"
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

# ContentPage

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`ContentPage` is the simplest concrete [[Page]] subclass: a single-child container holding one [[View]] (the page's `Content`). It is the workhorse 99% of app screens use, and is the default leaf for every navigation pattern ([[NavigationPage]] pushes it, [[TabbedPage]] hosts it per tab, [[Shell]] points routes at it). It adds three things over `Page`: a `Content` property, a `HideSoftInputOnTapped` convenience flag, and `SafeAreaEdges` for declaratively opting individual edges in or out of platform safe-area insets.

## MAUI Reference

- **Handler:** Uses the base `PageHandler` (`D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Page\PageHandler.*.cs`). No dedicated `ContentPageHandler` — `ContentPage` is a control-layer concept.
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\ContentPage\ContentPage.cs` (mapper at `ContentPage.Mapper.cs`)
- **Docs:** [Microsoft .NET MAUI — ContentPage](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/contentpage)

## MPAPP C++ API

```cpp
namespace mpapp {

class content_page : public page {
public:
    // The single hosted view. Defaults to nullptr (empty page).
    Observable<view*>            content;
    // If true, a tap outside the focused input dismisses the soft keyboard.
    Observable<bool>             hide_soft_input_on_tapped { false };
    // Per-edge safe-area opt-in. Default: edges respect platform insets.
    Observable<safe_area_edges>  safe_area_edges { safe_area_edges::none };

    content_page() = default;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<ContentPage Title="Home">
    <VerticalStackLayout Spacing="16" Padding="16">
        <Label Text="Welcome"/>
        <Button Text="Continue"/>
    </VerticalStackLayout>
</ContentPage>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.ContentControl` inside the host `Frame` | C++/WinRT | Content swap is direct child replacement; no animations at this level (the parent `NavigationPage`/`Shell` handles transitions). |
| Android | `Fragment` whose `onCreateView` returns the mapped `content` root | fbjni / JNI | `HideSoftInputOnTapped` wires a touch listener that calls `InputMethodManager.hideSoftInputFromWindow`. |
| Linux | `GtkBox` wrapping the rendered `content` view | GTK4 | `safe_area_edges` is a no-op on Linux (no system insets). |
| macOS | `NSView` content of the owning `NSViewController` | AppKit | `HideSoftInputOnTapped` ends first responder via `NSWindow.makeFirstResponder(nil)`. |
| iOS | `UIView` content of `UIViewController` | UIKit | `safe_area_edges` controls which edges call through `UIView.safeAreaInsets`. |

## Side-by-side Examples

### MAUI

```xml
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             Title="Home"
             HideSoftInputOnTapped="True">
    <VerticalStackLayout Padding="16" Spacing="12">
        <Label Text="Welcome"/>
        <Entry Placeholder="Tap outside to dismiss keyboard"/>
    </VerticalStackLayout>
</ContentPage>
```

### MPAPP (XAML)

```xml
<ContentPage Title="Home" HideSoftInputOnTapped="True">
    <VerticalStackLayout Padding="16" Spacing="12">
        <Label Text="Welcome"/>
        <Entry Placeholder="Tap outside to dismiss keyboard"/>
    </VerticalStackLayout>
</ContentPage>
```

### MPAPP (C++)

```cpp
class home_page : public content_page {
public:
    home_page() {
        title                    = u8"Home";
        hide_soft_input_on_tapped = true;

        auto stack = new vertical_stack_layout();
        stack->padding = thickness{16};
        stack->spacing = 12.0;
        stack->add(new label{.text = u8"Welcome"});
        stack->add(new entry{.placeholder = u8"Tap outside to dismiss keyboard"});

        content = stack;
    }
};
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/contentpage/mock_test.cpp` (planned)
- Windows handler: `tests/components/contentpage/windows_test.cpp` (planned)
- Android handler: `tests/components/contentpage/android_test.cpp` (planned)
- Linux handler: `tests/components/contentpage/linux_test.cpp` (planned)
- macOS handler: `tests/components/contentpage/macos_test.cpp` (planned)
- iOS handler: `tests/components/contentpage/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `Content` typing | `View` (runtime-typed bindable). | `view*` (compile-time-typed `Observable`). | Stricter type safety per project goals. | n/a |
| Control template | `ControlTemplate` swap rebinds inherited binding context. | Same semantics; implemented in `mpapp::templated_page`. | Parity. | n/a |
| Hot reload | `IHotReloadableView` interface. | Hot reload via the dev daemon; no user-facing interface. | Different reload pipeline. | RFC TBD |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Page]]
- [[ContentView]]
