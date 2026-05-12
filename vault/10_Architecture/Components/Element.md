---
type: component
mauiHandler: "Element"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/element"
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

# Element

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`Element` is the **root of MAUI's hierarchical object graph** — every [[Page]], [[View]], `Cell`, [[Application]], and [[Window]] derives from it. It supplies parenthood (`parent` / `logical_children`), identity (`id`, `automation_id`, `class_id`, `style_id`), resource inheritance, effect attachment, binding-context propagation (`BindableObject`), and the [[Handler]] hookup. `Element` itself has **no visual presence** — it is not a `View` and has no `PlatformView` beyond what the abstract `ElementHandler` base supplies. Subclasses introduce the visual surface ([[View]]) and the lifecycle surfaces ([[Page]], `Application`, `Window`).

Because every other component in MPAPP inherits from `element`, this class is the foundation for:

- the parent/child traversal walked by [[XAML-Compiler|the XAML compiler]] for name-scope resolution,
- `BindingContext` inheritance down the tree,
- merged `ResourceDictionary` lookup,
- handler attach/detach (via `set_virtual_view`),
- and effect propagation.

Because of its breadth, `element` is the only class with a hand-written set of platform-specific partial files in the controls assembly (`Element.Android.cs`, `Element.iOS.cs`, etc.) — each adds a minimal hook for the platform's resource/lifecycle quirks.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\Element\` (`ElementHandler`, `ElementHandlerOfT`)
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Element\`
- **Docs:** [Microsoft .NET MAUI — Element class](https://learn.microsoft.com/en-us/dotnet/api/microsoft.maui.controls.element)

## MPAPP C++ API

```cpp
namespace mpapp {

// CRTP base for everything in the visual hierarchy.
template <typename Self>
class element : public bindable_object {
public:
    // Identity --------------------------------------------------------------
    // Runtime-generated UUID; not stable across runs. Lazily allocated.
    [[nodiscard]] const std::string& id() const;

    // Settable exactly once. Throws on a second assignment, matching MAUI.
    Observable<std::string> automation_id;
    Observable<std::string> class_id;
    Observable<std::string> style_id;

    // Hierarchy -------------------------------------------------------------
    // Logical parent. Driven by the parent's child-list operations.
    Computed<element*> parent;

    // Logical children — read-only view; mutate via add_logical_child etc.
    Computed<std::span<const std::shared_ptr<element>>> logical_children;

    void add_logical_child(std::shared_ptr<element> child);
    void insert_logical_child(std::size_t index, std::shared_ptr<element> child);
    bool remove_logical_child(const std::shared_ptr<element>& child);
    void clear_logical_children();

    // Handler attach (called by the host; users do not invoke this directly).
    Computed<std::shared_ptr<i_element_handler>> handler;

    // Effects (post-hoc behaviors layered on top of the element).
    Observable<std::vector<std::shared_ptr<effect>>> effects;

protected:
    // Lifecycle hooks for subclasses.
    virtual void on_child_added(element& child);
    virtual void on_child_removed(element& child, std::size_t old_index);
    virtual void on_parent_set();
    virtual void on_binding_context_changed();
};

} // namespace mpapp
```

There is no public ctor that takes a parent — parenthood is established by the *parent's* `add_logical_child`. `handler` is exposed read-only; the host calls a non-public `attach_handler` overload behind the [[Property-Mapper]].

## XAML Usage

`Element` is abstract; it is not used directly in XAML. Every concrete control inherits the `AutomationId`, `ClassId`, `StyleId`, and resource-related attributes from this class:

```xml
<!-- Any element supports these attached identity properties. -->
<Label AutomationId="login_button_label"
       ClassId="primary"
       StyleId="LoginLabel"
       Text="Sign in" />
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | n/a (abstract) | C++/WinRT | Subclasses bind to `DependencyObject` descendants. |
| Android | n/a (abstract) | fbjni / JNI | `Element.Android.cs` adds the `Context` lookup helper. |
| Linux | n/a (abstract) | GTK4 | Resource inheritance walks the GTK widget tree via `gtk_widget_get_parent`. |
| macOS | n/a (abstract) | AppKit via [[Objective-Cpp]] | Same as Windows — concrete subclasses own a `NSResponder` descendant. |
| iOS | n/a (abstract) | UIKit via [[Objective-Cpp]] | Same as Windows — concrete subclasses own a `UIResponder` descendant. |

The element-level handler (`ElementHandler`) is platform-neutral — it manages the connect/disconnect state machine for any concrete element. Only subclasses (`ViewHandler`, `ApplicationHandler`, `WindowHandler`, etc.) actually own a platform view.

## Side-by-side Examples

### MAUI

```csharp
public class MyControl : Element
{
    public MyControl()
    {
        AutomationId = "my_widget";
        StyleId = "Primary";
    }

    protected override void OnBindingContextChanged()
    {
        base.OnBindingContextChanged();
        // Propagate to internal children.
    }
}
```

### MPAPP (XAML)

```xml
<!-- Element is abstract; subclasses inherit its identity attributes. -->
<ContentView AutomationId="my_widget" StyleId="Primary"/>
```

### MPAPP (C++)

```cpp
class my_control : public mpapp::element<my_control> {
public:
    my_control() {
        automation_id = "my_widget";
        style_id = "Primary";
    }

protected:
    void on_binding_context_changed() override {
        mpapp::element<my_control>::on_binding_context_changed();
        // Propagate to internal children.
    }
};
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/element/mock_test.cpp` (planned)
- Windows handler: `tests/components/element/windows_test.cpp` (planned)
- Android handler: `tests/components/element/android_test.cpp` (planned)
- Linux handler: `tests/components/element/linux_test.cpp` (planned)
- macOS handler: `tests/components/element/macos_test.cpp` (planned)
- iOS handler: `tests/components/element/ios_test.cpp` (planned)

The element-level test suite focuses on the *abstract* contract: parent/child propagation, binding-context inheritance, handler attach/detach ordering, and resource-merge lookup.

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| `Id` type | `Guid` (runtime-only, not stable) | `std::string` (UUID string) | Avoids dragging a `guid` type into the public API for no observable benefit | TBD |
| `AutomationId` set-once | Throws `InvalidOperationException` on second set | Throws `mpapp::invalid_operation` (`std::logic_error`-derived) | Matches the contract; just uses C++ exception hierarchy | N/A |
| `LogicalChildren` | `ReadOnlyCollection<Element>` | `std::span` over a stable internal vector | Zero-copy; no LINQ overhead | N/A |
| `Effects` mutation events | Backed by `TrackableCollection`, fires `Clearing`/`CollectionChanged` | Replaced wholesale via `Observable<std::vector<...>>` | Vector replacement is simpler and still observable | [[ADR-0009-public-api-template-wrappers-only]] |
| CRTP self-type | n/a (open-class inheritance) | `element<Self>` for static polymorphism in handlers | Lets handler mappers store typed function pointers | [[ADR-0009-public-api-template-wrappers-only]] |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Handler]]
- [[Markup]]
- [[Interop Parity]]
- [[Application]]
- [[Window]]
- [[Page]]
- [[View]]
- [[Observable Properties]]
- [[Component]]
