---
type: component
mauiHandler: "ImageCell"
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

# ImageCell

> [!info] Status
> **mock** — surface at `include/mpapp/image_cell.hpp` per [[ADR-0021-tableview-cell-types]]. Extends [[TextCell]]; adds `image_uri`. Mock handler tests cover the `text` + `image_uri` mappers and verify the inherited `detail` field survives the inheritance.

## Overview

`image_cell` is the most common TableView row — leading icon + primary `text` + optional `detail`. Settings menus, contact lists, conversation pickers all use this shape.

## MPAPP C++ API

```cpp
class image_cell : public text_cell {
public:
    Observable<std::string> image_uri;
};
```

Inherits `text`, `detail`, `text_color`, `detail_color` from [[TextCell]], plus `is_enabled` + `tapped` from [[Components/Cell]].

## See also

- [[ADR-0021-tableview-cell-types]] · [[TableView]] · [[TextCell]] · [[Image]]
