---
type: component
mauiHandler: "TemplatedView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/templatedview"
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

# TemplatedView

> [!info] Status
> **android-real** — real handlers on Windows (`mux::Controls::ContentControl`), Linux (`GtkBox` single-child), and Android (`android.widget.FrameLayout`). The `content` slot is live on all three runtime platforms; `template_id` is recorded but template instantiation is deferred to the templating-engine ADR. See [[Controls Inventory]] for the full porting matrix.

## Overview

`TemplatedView` is a layout-aware [[View]] whose visual structure is supplied by a `ControlTemplate` rather than fixed children. It is the base class for [[ContentView]] and for any user-defined "lookless" control whose look is supplied externally via a template. It owns `ControlTemplate`, `Padding`, `IsClippedToBounds`, and `CascadeInputTransparent` as bindable properties, and overrides measure/arrange to delegate to the resolved template's root.

In MPAPP, `templated_view` is the building block users subclass when authoring a *behavior* whose chrome should be theme-driven — a search box, a chip, a custom card — letting consumers redefine the appearance without touching code.

## MAUI Reference

- **Handler:** Inherited via `ViewHandler` — no dedicated `TemplatedViewHandler`.
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\TemplatedView\`
- **Docs:** [Microsoft .NET MAUI — TemplatedView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/templatedview)

## MPAPP C++ API

```cpp
namespace mpapp {

class templated_view : public view<templated_view> {
public:
    // The template that supplies the visual tree.
    Observable<std::shared_ptr<control_template>> control_template;

    // Padding applied inside the template root.
    Observable<thickness> padding { thickness::zero() };

    // Clip children to the view's bounds.
    Observable<bool> is_clipped_to_bounds { false };

    // If this view is input-transparent, propagate the flag to all children.
    Observable<bool> cascade_input_transparent { false };

protected:
    // Resolve the active template. Override to choose dynamically.
    virtual std::shared_ptr<control_template> resolve_control_template() const;

    // Called once the template has been applied and named parts are reachable.
    virtual void on_apply_template();

    // Look up a named element produced by the template.
    template <typename T>
    std::shared_ptr<T> get_template_child(std::string_view name) const;
};

} // namespace mpapp
```

`get_template_child` corresponds to MAUI's `GetTemplateChild(name)` — the way a templated control reaches into its own template to wire events.

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<TemplatedView Padding="8">
    <TemplatedView.ControlTemplate>
        <ControlTemplate>
            <Border Stroke="Gray" StrokeThickness="1">
                <ContentPresenter/>
            </Border>
        </ControlTemplate>
    </TemplatedView.ControlTemplate>
</TemplatedView>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.Maui.Platform.ContentPanel` | C++/WinRT | Same handler as [[ContentView]]; the template's root is what actually renders. |
| Android | `Microsoft.Maui.Platform.ContentViewGroup` | fbjni / JNI | Custom `ViewGroup` that delegates measure/arrange to MAUI's cross-platform layout. |
| Linux | Custom `GtkWidget` subclass (`MpappContentBox`) | GTK4 | Measure/arrange routed through MPAPP's cross-platform layout core. |
| macOS | `Microsoft.Maui.Platform.ContentView` | AppKit via [[Objective-Cpp]] | A flipped `NSView` that respects `padding`. |
| iOS | `Microsoft.Maui.Platform.ContentView` | UIKit via [[Objective-Cpp]] | A `UIView` subclass with `intrinsicContentSize` driven by the template. |

## Side-by-side Examples

### MAUI

```xml
<TemplatedView>
    <TemplatedView.ControlTemplate>
        <ControlTemplate>
            <Label Text="{TemplateBinding BindingContext.Caption}"/>
        </ControlTemplate>
    </TemplatedView.ControlTemplate>
</TemplatedView>
```

### MPAPP (XAML)

```xml
<TemplatedView>
    <TemplatedView.ControlTemplate>
        <ControlTemplate>
            <Label Text="{TemplateBinding BindingContext.Caption}"/>
        </ControlTemplate>
    </TemplatedView.ControlTemplate>
</TemplatedView>
```

### MPAPP (C++)

```cpp
auto tv = std::make_shared<mpapp::templated_view>();
tv->padding = mpapp::thickness{8};
tv->control_template = mpapp::control_template::from_factory([] {
    auto root = std::make_shared<mpapp::border>();
    root->stroke = mpapp::color::from_name("Gray");
    root->stroke_thickness = 1.0;
    root->content = std::make_shared<mpapp::content_presenter>();
    return root;
});
```

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Inherits from | `Compatibility.Layout` (legacy ILayout shim) | `view<templated_view>` directly | The compatibility layer is C# specific and adds no value | [[ADR-0009-public-api-template-wrappers-only]] |
| Obsolete `LowerChild`/`RaiseChild`/`UpdateChildrenLayout` | Still present, marked `[Obsolete]` | Not implemented | Z-order is managed via the `z_index` property | N/A |
| `OnMeasure` / `LayoutChildren` overrides | Both obsolete overloads remain | Only `measure_override` / `arrange_override` exposed | Single, modern layout protocol | N/A |

## Implementation

- Surface: [`include/mpapp/templated_view.hpp`](../../../include/mpapp/templated_view.hpp)
- Mock handler: [`include/mpapp/handlers/mock/templated_view_handler.hpp`](../../../include/mpapp/handlers/mock/templated_view_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/templated_view_handler.hpp`](../../../include/mpapp/handlers/windows/templated_view_handler.hpp) + [`src/handlers/windows/templated_view_handler.cpp`](../../../src/handlers/windows/templated_view_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/templated_view_handler.hpp`](../../../include/mpapp/handlers/linux/templated_view_handler.hpp) + [`src/handlers/linux/templated_view_handler.cpp`](../../../src/handlers/linux/templated_view_handler.cpp)
  - Android: [`include/mpapp/handlers/android/templated_view_handler.hpp`](../../../include/mpapp/handlers/android/templated_view_handler.hpp) + [`src/handlers/android/templated_view_handler.cpp`](../../../src/handlers/android/templated_view_handler.cpp)
- Tests: [`tests/mock_handlers/templated_view_test.cpp`](../../../tests/mock_handlers/templated_view_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ContentView]]
- [[View]]
- [[Layout]]
