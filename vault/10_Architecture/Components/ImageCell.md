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

## Implementation

- Surface: [`include/mpapp/image_cell.hpp`](../../../include/mpapp/image_cell.hpp) — inherits text/detail/text_color/detail_color from text_cell + adds `image_uri`.
- Mock handler: [`include/mpapp/handlers/mock/image_cell_handler.hpp`](../../../include/mpapp/handlers/mock/image_cell_handler.hpp).
- Real handlers:
  - Windows: [`src/handlers/windows/image_cell_handler.cpp`](../../../src/handlers/windows/image_cell_handler.cpp) — `mux::Controls::Border` + 2-col Grid; `BitmapImage` source handles `file://` / `http://` / `ms-appx://`.
  - Linux: [`src/handlers/linux/image_cell_handler.cpp`](../../../src/handlers/linux/image_cell_handler.cpp) — horizontal `GtkBox` (`GtkImage` 40px + label pair); `icon:foo` prefix routes through `gtk_image_set_from_icon_name`, plain paths through `gtk_image_set_from_file`.
  - Android: [`src/handlers/android/image_cell_handler.cpp`](../../../src/handlers/android/image_cell_handler.cpp) — horizontal `LinearLayout` (`ImageView` 80px + nested label pair); `BitmapFactory.decodeFile` for filesystem paths.

## See also

- [[ADR-0021-tableview-cell-types]] · [[TableView]] · [[TextCell]] · [[Image]]
