---
type: component
mauiHandler: "ImageCell"
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

# ImageCell

> [!info] Status
> **android-real** — Win `mux::Controls::Border` + 2-col Grid (Image 40×40 auto-col + vertical StackPanel(TextBlock+TextBlock)) — BitmapImage source handles file://, http://, ms-appx://. Linux horizontal `GtkBox` (`GtkImage` 40px + vertical GtkBox of label pair) — `icon:foo` prefix routes through `gtk_image_set_from_icon_name`, plain paths through `gtk_image_set_from_file`. Android horizontal `LinearLayout` (ImageView 80px + vertical LinearLayout of TextView pair weight=1) — BitmapFactory.decodeFile for filesystem paths.

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
