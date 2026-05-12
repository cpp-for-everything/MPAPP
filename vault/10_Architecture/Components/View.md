---
type: component
mauiHandler: "View"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/view"
mpappStatus: mock
platformWindows: false
platformAndroid: false
platformLinux: false
platformMacos: false
platformIos: false
tags:
  - type/component
  - status/mock
---

# View

> [!info] Status
> **mock** — full cross-platform surface lives at `include/mpapp/view.hpp`; mock handler records every property mapper invocation. See [[Controls Inventory]] for the full porting matrix.

## Overview

`View` is the abstract base class for every visual control in MAUI — it is the root of the visible UI hierarchy and the contract every [[Handlers|handler]] consumes. It surfaces the cross-cutting layout, transform, accessibility, and input properties that every concrete control (Label, Button, Layout, Border, etc.) inherits. In MAUI, the public `View` derives from `VisualElement` and `IView`, and the platform handler binds these abstract properties through a `PropertyMapper` to the underlying platform widget. In MPAPP it serves the same role: a single template-wrapped base type that gives every derived class a uniform observable surface.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\View\`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\View\`
- **Docs:** [Microsoft .NET MAUI — View](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/view)

The `ViewHandler` static `ViewMapper` lists the canonical property surface every subclass inherits: `AutomationId`, `Clip`, `Shadow`, `Visibility`, `Background`, `FlowDirection`, `Width`/`Height`, `MinimumWidth`/`MinimumHeight`, `MaximumWidth`/`MaximumHeight`, `IsEnabled`, `Opacity`, `Semantics`, `TranslationX`/`Y`, `Scale`/`ScaleX`/`ScaleY`, `Rotation`/`RotationX`/`RotationY`, `AnchorX`/`AnchorY`, `InputTransparent`, `ToolTip`, `ContextFlyout`, `SafeAreaEdges`. The `ViewCommandMapper` exposes `InvalidateMeasure`, `Frame`, `ZIndex`, `Focus`, and `Unfocus`.

## MPAPP C++ API

```cpp
namespace mpapp {

// Common cross-cutting surface for every visible control.
// Mirrors MAUI's IView / ViewHandler ViewMapper.
class view : public control<view> {
public:
    // Identity / accessibility
    Observable<std::string>           automation_id;
    Observable<semantics>             semantics_value;
    Observable<std::optional<std::string>> tool_tip;

    // Layout
    Observable<double>                width;
    Observable<double>                height;
    Observable<double>                minimum_width;
    Observable<double>                minimum_height;
    Observable<double>                maximum_width;
    Observable<double>                maximum_height;
    Observable<flow_direction>        flow_direction;   // match-parent | ltr | rtl
    Observable<safe_area_edges>       safe_area_edges;

    // Visual state
    Observable<visibility>            visibility;       // visible | hidden | collapsed
    Observable<bool>                  is_enabled;
    Observable<double>                opacity;          // 0.0 - 1.0
    Observable<brush_ref>             background;
    Observable<shadow>                shadow;
    Observable<geometry_ref>          clip;

    // Transforms
    Observable<double>                translation_x;
    Observable<double>                translation_y;
    Observable<double>                scale;
    Observable<double>                scale_x;
    Observable<double>                scale_y;
    Observable<double>                rotation;
    Observable<double>                rotation_x;
    Observable<double>                rotation_y;
    Observable<double>                anchor_x;
    Observable<double>                anchor_y;
    Observable<int>                   z_index;

    // Hit testing
    Observable<bool>                  input_transparent;

    // Commands
    Command<>                         invalidate_measure;
    Command<focus_request>            focus;
    Command<>                         unfocus;
};

} // namespace mpapp
```

No macros are used — every property is a template wrapper (see [[No Macros In Public API]] and [[Observable Properties]]).

## XAML Usage

`View` is abstract in MAUI; it is never instantiated directly, but its properties are usable on every concrete descendant. The fragment below shows the inherited surface in use on a `Label`:

```xml
<Label Text="Hello"
       Opacity="0.8"
       Rotation="15"
       AutomationId="greeting"
       IsEnabled="True"
       HorizontalOptions="Center" />
```

## Platform Notes

| Platform | Native control                           | Header / source            | Notes |
|----------|------------------------------------------|----------------------------|-------|
| Windows  | `Microsoft.UI.Xaml.FrameworkElement`     | C++/WinRT                  | Base of every WinUI control; handler stores `FrameworkElement?` as `PlatformView`. |
| Android  | `android.views.View`                     | fbjni / JNI                | The root of every Android UI widget hierarchy. |
| Linux    | `GtkWidget` (GTK4)                       | gtk4-rs / C bindings       | All GTK widgets derive from `GtkWidget`. |
| macOS    | `NSView` (AppKit) / `UIView` (Catalyst)  | AppKit / UIKit interop     | Mac Catalyst path uses `UIView`. |
| iOS      | `UIKit.UIView`                           | UIKit                      | Hosts every iOS-side control. |

## Side-by-side Examples

### MAUI

```xml
<Button Text="Tap"
        Opacity="0.9"
        Rotation="0"
        IsEnabled="True" />
```

### MPAPP (XAML)

```xml
<Button Text="Tap"
        Opacity="0.9"
        Rotation="0"
        IsEnabled="True" />
```

### MPAPP (C++)

```cpp
auto btn = mpapp::make<mpapp::button>();
btn->text       = "Tap";
btn->opacity    = 0.9;
btn->rotation   = 0.0;
btn->is_enabled = true;
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/view/mock_test.cpp` (planned)
- Windows handler: `tests/components/view/windows_test.cpp` (planned)
- Android handler: `tests/components/view/android_test.cpp` (planned)
- Linux handler: `tests/components/view/linux_test.cpp` (planned)
- macOS handler: `tests/components/view/macos_test.cpp` (planned)
- iOS handler: `tests/components/view/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Mock implementation

The P2 mock surface (ADR-0008) lands in this repository:

- **Cross-platform header:** `include/mpapp/view.hpp` — `mpapp::view` with the cross-cutting Observable surface and `Command<>`-tagged helpers (`invalidate_measure`, `focus`, `unfocus`).
- **Mock handler:** `include/mpapp/handlers/mock/view_handler.hpp` — `view_handler<platform::mock>` records every property-mapper invocation into `calls()`.
- **Mock tests:** `tests/mock_handlers/view_test.cpp` — Catch2 cases covering initial-bind recording, idempotent set short-circuiting, multi-property ordering, and symbolic enum repr.

Rich types (`brush_ref`, `shadow_desc`) are lightweight stand-ins in the mock layer and are replaced by the real graphics types alongside the per-platform handlers in P3.

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Observable Properties]]
