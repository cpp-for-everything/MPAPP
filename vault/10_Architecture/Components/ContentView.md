---
type: component
mauiHandler: "ContentView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/contentview"
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

# ContentView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`ContentView` is the simplest possible container: a [[TemplatedView]] that hosts a single child [[View]] through its `Content` property. It is the standard base class for compound, reusable custom controls — a card, a labeled field, an icon-with-text — where the consumer just wants "one thing, with my chrome around it". `ContentView` also adds a `SafeAreaEdges` property so the control can opt into iOS/Android safe-area insets independently of its parent [[Page]].

`Content` is the XAML `ContentProperty`, so the child can be written as a direct nested element.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\ContentView\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\ContentView\`
- **Docs:** [Microsoft .NET MAUI — ContentView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/contentview)

## MPAPP C++ API

```cpp
namespace mpapp {

class content_view : public templated_view<content_view> {
public:
    // The single hosted child. Designated XAML content property.
    Observable<std::shared_ptr<view_base>> content;

    // Which edges should obey safe-area insets. Default: none (edge-to-edge).
    Observable<safe_area_edges> safe_area_edges { safe_area_edges::none() };
};

} // namespace mpapp
```

Because `content_view` inherits from `templated_view`, it also picks up `control_template`, `padding`, `is_clipped_to_bounds`, and `cascade_input_transparent`. The handler maps `Content` onto the native container's single child slot.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<ContentView Padding="12" SafeAreaEdges="Top,Bottom">
    <VerticalStackLayout>
        <Label Text="Header" FontAttributes="Bold"/>
        <Label Text="Body"/>
    </VerticalStackLayout>
</ContentView>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.Maui.Platform.ContentPanel` | C++/WinRT | A `Panel` that lays out its single child via the cross-platform layout core. |
| Android | `Microsoft.Maui.Platform.ContentViewGroup` | fbjni / JNI | Custom `ViewGroup` that hands measure/layout to the cross-platform core. |
| Linux | Custom `GtkWidget` subclass (`MpappContentBox`) | GTK4 | Single-child container with `padding`. |
| macOS | `Microsoft.Maui.Platform.ContentView` (flipped `NSView`) | AppKit via [[Objective-Cpp]] | `safe_area_edges` is a no-op (no notch). |
| iOS | `Microsoft.Maui.Platform.ContentView` (`UIView`) | UIKit via [[Objective-Cpp]] | `safe_area_edges` drives `UIView.safeAreaInsets`. |

## Side-by-side Examples

### MAUI

```xml
<ContentView Padding="12">
    <Label Text="Hello"/>
</ContentView>
```

### MPAPP (XAML)

```xml
<ContentView Padding="12">
    <Label Text="Hello"/>
</ContentView>
```

### MPAPP (C++)

```cpp
auto card = std::make_shared<mpapp::content_view>();
card->padding = mpapp::thickness{12};

auto label = std::make_shared<mpapp::label>();
label->text = "Hello";
card->content = label;
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/contentview/mock_test.cpp` (planned)
- Windows handler: `tests/components/contentview/windows_test.cpp` (planned)
- Android handler: `tests/components/contentview/android_test.cpp` (planned)
- Linux handler: `tests/components/contentview/linux_test.cpp` (planned)
- macOS handler: `tests/components/contentview/macos_test.cpp` (planned)
- iOS handler: `tests/components/contentview/ios_test.cpp` (planned)

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `ISafeAreaView2.SafeAreaInsets` | Public no-op setter on the interface | Internal — only the handler can drive it | Users should not be writing safe-area insets manually | TBD |
| `SafeAreaEdges` default | `None` (edge-to-edge) | Same | Matches MAUI; mobile apps must opt in explicitly | N/A |
| `Content`-as-`Element` parenthood | `Content` is wired as a logical child of the `ContentView` | Same | Required for binding-context inheritance | N/A |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[TemplatedView]]
- [[View]]
- [[ContentPage]]
- [[Layout]]
