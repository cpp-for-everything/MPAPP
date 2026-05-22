---
type: component
mauiHandler: "EntryCell"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/cells"
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

# EntryCell

> [!info] Status
> **mock** — surface at `include/mpapp/entry_cell.hpp` per [[ADR-0021-tableview-cell-types]]. Mock handler tests cover `label` + `text` mappers. `keyboard_kind` enum mirrors MAUI (Default / Chat / Email / Numeric / Telephone / Text / Url).

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

## See also

- [[ADR-0021-tableview-cell-types]] · [[TableView]] · [[Entry]]
