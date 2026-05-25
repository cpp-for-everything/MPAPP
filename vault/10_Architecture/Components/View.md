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

> [!info] Terminal status (per [[40_Roadmap/M-04b-handler-bulk-port|M-04b]])
> **mock** is the **terminal state** for `View`. It is the abstract base every visual control inherits from; the real handler work happens in concrete subclasses (Button, Label, BoxView, etc.). The mock handler records every property-mapper invocation so derived classes inherit a correct C++ template instantiation. See [[Controls Inventory]] for the full porting matrix.

## Overview

`View` is the abstract base class for every visual control in MAUI — it is the root of the visible UI hierarchy and the contract every [[Handlers|handler]] consumes. It surfaces the cross-cutting layout, transform, accessibility, and input properties that every concrete control (Label, Button, Layout, Border, etc.) inherits. In MAUI, the public `View` derives from `VisualElement` and `IView`, and the platform handler binds these abstract properties through a `PropertyMapper` to the underlying platform widget. In MPAPP it serves the same role: a single template-wrapped base type that gives every derived class a uniform observable surface.



## Wrapper + Surface

> [!info] Abstract base class
> `mpapp::view` is a CRTP / abstract base inherited by concrete components — it is not a leaf component itself and does not follow the [[ADR-0024-wrapper-component-pattern]] wrapper / surface split.
>
> Concrete components that inherit `view` each have their own `mpapp::internal::basic_<...>` surface and `mpapp::<...>` wrapper; this base class participates in the chain as the inheritance root.

## Gesture recognizers

Per [[RFC-0003-gesture-recognizers]] `view` ships a polymorphic collection of attached gesture recognizers that every derived component inherits automatically:

```cpp
class view {
    // ... existing Observables ...

    std::vector<std::shared_ptr<internal::basic_gesture_recognizer>>
        gesture_recognizers{};

    template <class T, class... Args>
    T& add_gesture(Args&&... args);   // emplace + return ref
};
```

App-code usage:

```cpp
mpapp::button btn;
auto& tap = btn.add_gesture<mpapp::tap_gesture_recognizer>();
tap.number_of_taps_required = 2;
tap.tapped.subscribe(slot, [&](const mpapp::tapped_event_args& a) {
    std::println("double-tapped at ({}, {})", a.x, a.y);
});
```

Per-platform `view_handler<P>::map_gestures(view&)` walks the collection at bind time and installs the matching native listener for each recognizer's `kind()` — `UITapGestureRecognizer` on iOS, `GtkGestureClick` on Linux, `UIElement.Tapped` on Windows, etc. The mock handler records `gesture.<kind>_attached` per recognizer + provides `simulate_tap(view&, …)` for tests to drive synthetic events. See [[../50_Tasks/T-0033-gesture-recognizers-tap-slice/T-0033-gesture-recognizers-tap-slice]] for the first slice (tap only) shipped today.

## Resources + parent pointer

Per [[RFC-0005-resource-dictionaries-and-styling]] `view` carries two new members:

```cpp
class view {
    // ... existing Observables + gesture_recognizers ...

    std::shared_ptr<resource_dictionary> resources{};   // optional local dict

    view* parent() const noexcept;                       // visual-tree up-link
    void  set_parent(view*) noexcept;                    // maintained by layout::add/remove
};
```

`mpapp::find_in<T>(view, key)` walks `view.resources → view.parent()->resources → ... → root.resources` and returns the first typed match — same semantics as MAUI's `{StaticResource Key}`. `layout::add` / `insert` / `remove` / `clear` keep the parent pointer in sync; tests that exercise the walker directly can call `set_parent` manually. Default-null `resources` keeps the per-view memory cost at a single pointer for views that never touch styling.

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

## Implementation

- Surface: [`include/mpapp/view.hpp`](../../../include/mpapp/view.hpp)
- Mock handler: [`include/mpapp/handlers/mock/view_handler.hpp`](../../../include/mpapp/handlers/mock/view_handler.hpp)
- Tests: [`tests/mock_handlers/view_test.cpp`](../../../tests/mock_handlers/view_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Observable Properties]]
