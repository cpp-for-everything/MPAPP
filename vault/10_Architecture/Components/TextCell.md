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


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_text_cell` | [`include/mpapp/internal/basic_text_cell.hpp`](../../../include/mpapp/internal/basic_text_cell.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::text_cell` | [`include/mpapp/text_cell.hpp`](../../../include/mpapp/text_cell.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/text_cell.hpp>

mpapp::text_cell w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/text_cell.hpp>
#include <mpapp/handlers/mock/text_cell_handler.hpp>

mpapp::internal::basic_text_cell w;
mpapp::text_cell_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::text_cell_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::text_cell_handler<>` and `mpapp::text_cell_handler<platform::mock>` valid spellings without naming `internal::`.

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
