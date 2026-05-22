---
type: component
mauiHandler: "TextCell"
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

# TextCell

> [!info] Status
> **mock** — surface lives at `include/mpapp/text_cell.hpp` per [[ADR-0021-tableview-cell-types]]. Catch2 mock-handler tests exercise the `text` + `detail` mappers + the inherited `cell::is_enabled` / `cell::tapped`. Real per-platform handlers land alongside the TableView surface refactor that swaps `vec<{title, vec<string>}>` for `vec<table_section{title, vec<unique_ptr<cell>>}>`.

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

## See also

- [[ADR-0021-tableview-cell-types]] · [[TableView]] · [[EntryCell]] · [[ImageCell]]
