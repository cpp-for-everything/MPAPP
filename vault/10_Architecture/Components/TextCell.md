---
type: component
mauiHandler: "TextCell"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/cells"
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

# TextCell

> [!info] Status
> **android-real** — Windows wraps `mux::Controls::Border` containing a vertical `StackPanel` of two `TextBlock`s (primary + detail). Linux wraps a vertical `GtkBox` of two `GtkLabel`s. Android wraps a vertical `LinearLayout` of two `TextView`s. Detail row hides via `Visibility::Collapsed` / `gtk_widget_set_visible(FALSE)` / `VIEW_GONE` when the detail string is empty. Native row padding (12px horizontal / 6px vertical) baked in so cells look right when used as a list item. text_cell self-registers via the ADR-0013 dispatch registry so it can be nested anywhere a view fits — useful today inside `view_cell` / `border` / `stack_layout`, and inside `table_view` once the cell-typed-section surface refactor lands. macOS / iOS pending Apple host.

## Overview

`text_cell` is a TableView row showing a primary `text` value and an optional `detail` string. On iOS / macOS this renders as the Settings-style two-line cell; on Android as a default list-item layout; on Windows / Linux as a label-pair inside the platform's list row.

## MPAPP C++ API

```cpp
class text_cell : public cell {
public:
    Observable<std::string> text{""};
    Observable<std::string> detail{""};
    Observable<std::string> text_color{""};
    Observable<std::string> detail_color{""};
};
```

Inherits `is_enabled` + `tapped` from [[Components/Cell]].

## Implementation

- Surface: [`include/mpapp/text_cell.hpp`](../../../include/mpapp/text_cell.hpp) — text / detail / text_color / detail_color (all `Observable<std::string>`).
- Mock handler: [`include/mpapp/handlers/mock/text_cell_handler.hpp`](../../../include/mpapp/handlers/mock/text_cell_handler.hpp).
- Real handlers:
  - Windows: [`src/handlers/windows/text_cell_handler.cpp`](../../../src/handlers/windows/text_cell_handler.cpp) — `mux::Controls::Border` + vertical `StackPanel` of two `TextBlock`s.
  - Linux: [`src/handlers/linux/text_cell_handler.cpp`](../../../src/handlers/linux/text_cell_handler.cpp) — vertical `GtkBox` of two `GtkLabel`s.
  - Android: [`src/handlers/android/text_cell_handler.cpp`](../../../src/handlers/android/text_cell_handler.cpp) — vertical `LinearLayout` of two `TextView`s.
- Self-registers in the [[ADR-0013-data-driven-widget-dispatch]] registry so it can be nested anywhere a `view` fits.

## See also

- [[ADR-0021-tableview-cell-types]] · [[TableView]] · [[EntryCell]] · [[ImageCell]]
