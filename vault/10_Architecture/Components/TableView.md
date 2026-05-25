---
type: component
mauiHandler: "TableView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/tableview"
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
> **android-real** — Windows wraps `mux::Controls::ListView`, Linux uses `GtkListBox` in `GtkScrolledWindow`, Android wraps `android.widget.ListView` with `ArrayAdapter<String>`. Sections flatten to a section-title row (prefixed with `▾ `) followed by data rows. On Linux the section row is set non-selectable + non-activatable; on Win/Android the equivalent is a v2 enhancement. **Cell-typed rendering** per [[ADR-0021-tableview-cell-types]] is wired through string rows in v1; richer cell types land when the table_view surface evolves from `vec<{title, vec<string>}>` to `vec<table_section{title, vec<unique_ptr<cell>>}>`. `row_height` honoring deferred per platform.

# TableView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

`TableView` displays a scrollable list of rows organized into named sections — typically used for settings screens, forms, and other heterogeneous layouts where each row is a `Cell` declared inline rather than driven by an `ItemTemplate`. Unlike [[ListView]], the content is built statically from a `TableRoot` containing `TableSection`s, which makes it ideal for fixed-shape UIs. MAUI flags it as obsolete in favor of `CollectionView`, and MPAPP ports it for parity with the legacy MAUI surface but does not recommend it for new code.


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_table_view` | [`include/mpapp/internal/basic_table_view.hpp`](../../../include/mpapp/internal/basic_table_view.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::table_view` | [`include/mpapp/table_view.hpp`](../../../include/mpapp/table_view.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_tv_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/table_view.hpp>

mpapp::table_view w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/table_view.hpp>
#include <mpapp/handlers/mock/table_view_handler.hpp>

mpapp::internal::basic_table_view w;
mpapp::table_view_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::table_view_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::table_view_handler<>` and `mpapp::table_view_handler<platform::mock>` valid spellings without naming `internal::`.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\Compatibility\Handlers\TableView\` (compat renderers; no `Microsoft.Maui.Handlers` mapper)
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\TableView\TableView.cs`
- **Docs:** [Microsoft .NET MAUI — TableView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/tableview)

## MPAPP C++ API

```cpp
namespace mpapp {

enum class table_intent { menu, settings, form, data };

class table_section : public element {
public:
    Observable<std::string>               title;
    Observable<color>                     text_color;
    Observable<observable_vector<cell>>   cells;
};

class table_root : public table_section {
public:
    Observable<observable_vector<table_section>> sections;
};

class tableview : public view<tableview> {
public:
    Observable<table_root>          root;
    Observable<table_intent>        intent;        // default: data
    Observable<int>                 row_height;    // -1 for native default
    Observable<bool>                has_uneven_rows;

    // No selection event — TableView dispatches taps to cells, which
    // surface their own events (text_cell::tapped, switch_cell::on, ...).
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<TableView Intent="Settings">
    <TableRoot>
        <TableSection Title="Profile">
            <EntryCell Label="Name"  Text="{Binding Name}"/>
            <EntryCell Label="Email" Text="{Binding Email}"/>
        </TableSection>
        <TableSection Title="Preferences">
            <SwitchCell Text="Notifications" On="{Binding NotifyEnabled}"/>
        </TableSection>
    </TableRoot>
</TableView>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.ListView` with grouped data | C++/WinRT | No native sectioned table; uses `CollectionViewSource` with grouping and `GroupStyle` headers. |
| Android | `android.widget.ListView` with a sectioned `BaseAdapter` (`TableViewModelRenderer`) | fbjni / JNI | Section headers rendered as non-clickable rows. |
| Linux | `GtkListBox` with header rows (`gtk_list_box_set_header_func`) | GTK4 | Each section title row is rendered as an unselectable header. |
| macOS | `NSTableView` (grouped style) inside `NSScrollView` | AppKit | Uses `NSTableView`'s native grouped row appearance. |
| iOS | `UITableView` with style `UITableViewStyle.Grouped` | UIKit | Matches MAUI: TableView always renders as grouped. `Intent` maps to visual styling only. |

## Side-by-side Examples

### MAUI

```xml
<TableView Intent="Settings">
    <TableRoot>
        <TableSection Title="Account">
            <TextCell Text="Sign out"/>
        </TableSection>
    </TableRoot>
</TableView>
```

### MPAPP (XAML)

```xml
<TableView Intent="Settings">
    <TableRoot>
        <TableSection Title="Account">
            <TextCell Text="Sign out"/>
        </TableSection>
    </TableRoot>
</TableView>
```

### MPAPP (C++)

```cpp
auto tv = std::make_shared<mpapp::tableview>();
tv->intent = mpapp::table_intent::settings;

mpapp::table_root root;
mpapp::table_section account;
account.title = "Account";
account.cells.get().push_back(mpapp::text_cell{"Sign out"});
root.sections.get().push_back(std::move(account));
tv->root = std::move(root);
```

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| Deprecation | `[Obsolete]` — kept for source-compat only | Same — documented as legacy | Mirror MAUI guidance | — |
| Section model | `TableRoot` + `TableSection` + `Cell` | Same shape, but section/cell collections use `observable_vector` | Compile-time observable contract per [[ADR-0009-public-api-template-wrappers-only]] | — |
| Cell mutation | Cells reparented imperatively; raises `ChildAdded` | Reparenting handled by `observable_vector` move semantics | C++ value semantics | — |
| macOS native control | `UITableView` (Catalyst) | `NSTableView` (AppKit) | [[ADR-0005-ios-macos-separate-interop]] | — |

## Implementation

- Surface: [`include/mpapp/table_view.hpp`](../../../include/mpapp/table_view.hpp)
- Mock handler: [`include/mpapp/handlers/mock/table_view_handler.hpp`](../../../include/mpapp/handlers/mock/table_view_handler.hpp)
- Real handlers:
  - Windows: [`include/mpapp/handlers/windows/table_view_handler.hpp`](../../../include/mpapp/handlers/windows/table_view_handler.hpp) + [`src/handlers/windows/table_view_handler.cpp`](../../../src/handlers/windows/table_view_handler.cpp)
  - Linux: [`include/mpapp/handlers/linux/table_view_handler.hpp`](../../../include/mpapp/handlers/linux/table_view_handler.hpp) + [`src/handlers/linux/table_view_handler.cpp`](../../../src/handlers/linux/table_view_handler.cpp)
  - Android: [`include/mpapp/handlers/android/table_view_handler.hpp`](../../../include/mpapp/handlers/android/table_view_handler.hpp) + [`src/handlers/android/table_view_handler.cpp`](../../../src/handlers/android/table_view_handler.cpp)
- Tests: [`tests/mock_handlers/table_view_test.cpp`](../../../tests/mock_handlers/table_view_test.cpp)

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[ListView]]
- [[CollectionView]]
