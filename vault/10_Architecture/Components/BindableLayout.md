---
type: component
mauiHandler: "BindableLayout"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/bindablelayout"
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

# BindableLayout

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`BindableLayout` is **not a control** — it is a set of attached properties (`ItemsSource`, `ItemTemplate`, `ItemTemplateSelector`, `EmptyView`, `EmptyViewTemplate`) that turn any existing [[Layout]] subclass (`Grid`, `StackLayout`, `FlexLayout`, …) into a lightweight, non-virtualizing items-control. Setting `BindableLayout.ItemsSource` instantiates one child per item using the supplied `DataTemplate` and keeps the children in sync via `INotifyCollectionChanged`. It is the simplest way to render a small, known collection without the weight of `CollectionView`. In MPAPP this surfaces as a static attached-property facility on `bindable_layout`, plus an `enable(...)` helper for purely-C++ wiring.

## MAUI Reference

- **Handler:** *(none — `BindableLayout` has no dedicated handler; it composes with the host [[Layout]]'s handler)*
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\BindableLayout\BindableLayout.cs` (also contains the internal `BindableLayoutController`)
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

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/bindablelayout/mock_test.cpp` (planned)
- Windows handler: `tests/components/bindablelayout/windows_test.cpp` (planned)
- Android handler: `tests/components/bindablelayout/android_test.cpp` (planned)
- Linux handler: `tests/components/bindablelayout/linux_test.cpp` (planned)
- macOS handler: `tests/components/bindablelayout/macos_test.cpp` (planned)
- iOS handler: `tests/components/bindablelayout/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[Layout]]
- [[View]]
