---
type: component
mauiHandler: "EntryCell"
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

# EntryCell

> [!info] Status
> **android-real** — Win `mux::Controls::Border` + 2-col Grid (TextBlock auto + TextBox star) — `InputScope` maps `keyboard_kind` → `Email/Number/TelephoneNumber/Url/Chat/Default`. `TextChanged` echoes user input via a suppress-echo guard; `KeyDown` on `VirtualKey::Enter` emits `completed`. Linux horizontal `GtkBox` (`GtkLabel` + `GtkEntry`) — `gtk_entry_set_input_purpose` maps keyboard. `"changed"` echoes text; `"activate"` (Enter key) emits `completed`. Android horizontal `LinearLayout` (`TextView` + `EditText` weight=1) — `setInputType` maps keyboard. Reuses `MppTextWatcher` with `kind=3` for echo; new `MppEditorActionListener` (`kind=1` for entry_cell) routes IME `IME_ACTION_{DONE,GO,NEXT,SEARCH,SEND}` to `completed`.

## Overview

`entry_cell` is a TableView row with a `label` + an inline editable `text` field. Two-way bound; emits `completed` when the user commits.

## MPAPP C++ API

```cpp
enum class keyboard_kind { default_, chat, email, numeric, telephone, text, url };

class entry_cell : public cell {
public:
    Observable<std::string>   label;
    Observable<std::string>   text;
    Observable<std::string>   placeholder;
    Observable<keyboard_kind> keyboard{keyboard_kind::default_};

    signal<const std::string&> completed;
};
```

Inherits `is_enabled` + `tapped` from [[Components/Cell]].

## Implementation

- Surface: [`include/mpapp/entry_cell.hpp`](../../../include/mpapp/entry_cell.hpp) — observable label/text/placeholder/keyboard + `completed` signal.
- Mock handler: [`include/mpapp/handlers/mock/entry_cell_handler.hpp`](../../../include/mpapp/handlers/mock/entry_cell_handler.hpp).
- Real handlers:
  - Windows: [`src/handlers/windows/entry_cell_handler.cpp`](../../../src/handlers/windows/entry_cell_handler.cpp) — `mux::Controls::Border` + 2-col Grid (TextBlock + TextBox); `InputScope` maps `keyboard_kind`.
  - Linux: [`src/handlers/linux/entry_cell_handler.cpp`](../../../src/handlers/linux/entry_cell_handler.cpp) — horizontal `GtkBox` (`GtkLabel` + `GtkEntry`); `gtk_entry_set_input_purpose` for keyboard.
  - Android: [`src/handlers/android/entry_cell_handler.cpp`](../../../src/handlers/android/entry_cell_handler.cpp) — `LinearLayout` (TextView + EditText weight=1); `MppEditorActionListener` (kind=1) routes IME actions to `completed`.
- Cell tree dispatch: [[ADR-0021-tableview-cell-types]] + [[ADR-0013-data-driven-widget-dispatch]] (widget_dispatch registry).

## See also

- [[ADR-0021-tableview-cell-types]] · [[TableView]] · [[Entry]]
