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


## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers:

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_image_cell` | [`include/mpapp/internal/basic_image_cell.hpp`](../../../include/mpapp/internal/basic_image_cell.hpp) |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::image_cell` | [`include/mpapp/image_cell.hpp`](../../../include/mpapp/image_cell.hpp) |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/image_cell.hpp>

mpapp::image_cell w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library (per [[ADR-0008-mock-first-implementation]]):

```cpp
#include <mpapp/image_cell.hpp>
#include <mpapp/handlers/mock/image_cell_handler.hpp>

mpapp::internal::basic_image_cell w;
mpapp::image_cell_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

The `mpapp::image_cell_handler<Platform>` alias (template, defaults to `platform::current`) keeps `mpapp::image_cell_handler<>` and `mpapp::image_cell_handler<platform::mock>` valid spellings without naming `internal::`.

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
