---
type: component
mauiHandler: "BindableLayout"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/bindablelayout"
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

# BindableLayout

> [!info] Status
> **mock** — attached-property facility at `include/mpapp/bindable_layout.hpp`; mock handler snapshots items-source count, template name, and empty-view presence. See [[Controls Inventory]].

## Overview

`BindableLayout` is **not a control** — it is a set of attached properties (`ItemsSource`, `ItemTemplate`, `ItemTemplateSelector`, `EmptyView`, `EmptyViewTemplate`) that turn any existing [[Layout]] subclass (`Grid`, `StackLayout`, `FlexLayout`, …) into a lightweight, non-virtualizing items-control. Setting `BindableLayout.ItemsSource` instantiates one child per item using the supplied `DataTemplate` and keeps the children in sync via `INotifyCollectionChanged`. It is the simplest way to render a small, known collection without the weight of `CollectionView`. In MPAPP this surfaces as a static attached-property facility on `bindable_layout`, plus an `enable(...)` helper for purely-C++ wiring.


## Wrapper + Surface

> [!info] No wrapper layer
> `mpapp::bindable_layout` is an **explicit exception** to [[ADR-0024-wrapper-component-pattern]] — see the ADR's *Skipped categories*.
>
> **Why:** Static attached-property facility: `bindable_layout()` is `delete`d, every method is static, and the handler attaches to a *layout host* (not a `bindable_layout` instance). No instance exists to wrap.

The component is constructed and used as-is (no `internal::basic_bindable_layout` indirection, no embedded handler in the public class).

## MAUI Reference

- **Handler:** *(none — `BindableLayout` has no dedicated handler; it composes with the host [[Layout]]'s handler)*
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\BindableLayout\BindableLayout.cs` (also contains the internal `BindableLayoutController`)
- **Docs:** [Microsoft .NET MAUI — BindableLayout](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/bindablelayout)

The bindable properties (all attached) are: `ItemsSourceProperty`, `ItemTemplateProperty`, `ItemTemplateSelectorProperty`, `EmptyViewProperty`, `EmptyViewTemplateProperty`. An internal `BindableLayoutController` watches the source collection and materializes / removes child views on the host layout.

## MPAPP C++ API

```cpp
namespace mpapp {

// Static facility - not instantiable. The methods mirror MAUI's
// BindableLayout.GetItemsSource / SetItemsSource attached-property pattern.
class bindable_layout {
public:
    // Attached-property accessors (XAML compiler emits calls to these).
    static void                     set_items_source(layout& host, items_source items);
    static items_source             get_items_source(const layout& host);

    static void                     set_item_template(layout& host, data_template tpl);
    static data_template            get_item_template(const layout& host);

    static void                     set_item_template_selector(layout& host, data_template_selector sel);
    static data_template_selector   get_item_template_selector(const layout& host);

    static void                     set_empty_view(layout& host, std::shared_ptr<view> empty);
    static std::shared_ptr<view>    get_empty_view(const layout& host);

    static void                     set_empty_view_template(layout& host, data_template tpl);
    static data_template            get_empty_view_template(const layout& host);

    // C++-friendly fluent helper that wires all three together.
    static void enable(layout& host,
                       items_source items,
                       data_template item_template);
};

} // namespace mpapp
```

## XAML Usage

```xml
<StackLayout BindableLayout.ItemsSource="{Binding Items}">
    <BindableLayout.ItemTemplate>
        <DataTemplate>
            <Label Text="{Binding Name}" />
        </DataTemplate>
    </BindableLayout.ItemTemplate>
    <BindableLayout.EmptyView>
        <Label Text="No items" />
    </BindableLayout.EmptyView>
</StackLayout>
```

## Platform Notes

`BindableLayout` has no platform widget of its own — it composes with the host layout. The "native control" column lists the *parent* layout's platform type, since that is what actually renders.

| Platform | Native control                                       | Header / source            | Notes |
|----------|------------------------------------------------------|----------------------------|-------|
| Windows  | host layout's `Microsoft.UI.Xaml.Controls.Panel`     | C++/WinRT                  | Items materialised into host `Panel.Children`. |
| Android  | host layout's `android.view.ViewGroup`               | fbjni / JNI                | Items appended as child views. |
| Linux    | host layout's `GtkBox` / `GtkFixed`                  | gtk4-rs                    | One GTK widget per item. |
| macOS    | host layout's `NSView`                               | AppKit / Catalyst          | Non-virtualizing. |
| iOS      | host layout's `UIKit.UIView`                         | UIKit                      | Non-virtualizing — use `CollectionView` for large sources. |

## Side-by-side Examples

### MAUI

```xml
<StackLayout BindableLayout.ItemsSource="{Binding Names}">
    <BindableLayout.ItemTemplate>
        <DataTemplate>
            <Label Text="{Binding .}" />
        </DataTemplate>
    </BindableLayout.ItemTemplate>
</StackLayout>
```

### MPAPP (XAML)

```xml
<StackLayout BindableLayout.ItemsSource="{Binding Names}">
    <BindableLayout.ItemTemplate>
        <DataTemplate>
            <Label Text="{Binding .}" />
        </DataTemplate>
    </BindableLayout.ItemTemplate>
</StackLayout>
```

### MPAPP (C++)

```cpp
auto host = mpapp::make<mpapp::stack_layout>();

mpapp::bindable_layout::enable(
    *host,
    mpapp::items_source::from(view_model.names),
    mpapp::data_template::for_each([](const auto& name) {
        auto lbl = mpapp::make<mpapp::label>();
        lbl->text = name;
        return lbl;
    }));
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## Mock implementation

The P2 mock surface (ADR-0008) lands in this repository:

- **Cross-platform header:** `include/mpapp/bindable_layout.hpp` — `mpapp::bindable_layout` static facility with the attached-property accessor pair plus a `enable(...)` C++ helper. State is keyed on the host `layout*` in a static side table.
- **Mock handler:** `include/mpapp/handlers/mock/bindable_layout_handler.hpp` — `bindable_layout_handler<platform::mock>` records mapper invocations (items count, template name, empty-view presence). Unlike Observable-backed mock handlers there is no `changed` signal; tests re-invoke the mapper to observe propagation.
- **Mock tests:** `tests/mock_handlers/bindable_layout_test.cpp`.

The rich `items_source` / `data_template` / `data_template_selector` types are reduced to lightweight stand-ins for the mock; the full versions arrive with `CollectionView` in M-03.

## Implementation

- Surface: [`include/mpapp/bindable_layout.hpp`](../../../include/mpapp/bindable_layout.hpp)
- Mock handler: [`include/mpapp/handlers/mock/bindable_layout_handler.hpp`](../../../include/mpapp/handlers/mock/bindable_layout_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/bindable_layout_handler.hpp`](../../../include/mpapp/handlers/windows/bindable_layout_handler.hpp) + [`src/handlers/windows/bindable_layout_handler.cpp`](../../../src/handlers/windows/bindable_layout_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/bindable_layout_handler.hpp`](../../../include/mpapp/handlers/linux/bindable_layout_handler.hpp) + [`src/handlers/linux/bindable_layout_handler.cpp`](../../../src/handlers/linux/bindable_layout_handler.cpp)
  - Android: [`include/mpapp/handlers/android/bindable_layout_handler.hpp`](../../../include/mpapp/handlers/android/bindable_layout_handler.hpp) + [`src/handlers/android/bindable_layout_handler.cpp`](../../../src/handlers/android/bindable_layout_handler.cpp)
- Tests: [`tests/mock_handlers/bindable_layout_test.cpp`](../../../tests/mock_handlers/bindable_layout_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Layout]]
- [[View]]
