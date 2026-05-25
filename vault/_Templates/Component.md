---
type: component
mauiHandler: ""
mauiDocUrl: ""
mpappStatus: not-started
platformWindows: false
platformAndroid: false
platformLinux: false
platformMacos: false
platformIos: false
tags:
  - type/component
  - status/not-started
---

# <% tp.file.title %>

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]].

## Overview

What is this control? What does it do for the user?

## Wrapper + Surface

Per [[ADR-0024-wrapper-component-pattern]] this component is split into two layers (fill in once the surface + wrapper exist):

| Layer | Class | Header |
|---|---|---|
| Surface — platform-agnostic, handler held by pointer | `mpapp::internal::basic_<snake>` | `include/mpapp/internal/basic_<snake>.hpp` |
| Wrapper — user-facing, embeds the platform handler by value | `mpapp::<snake>` | `include/mpapp/<snake>.hpp` |

**App code uses the wrapper.** Its default constructor auto-binds the embedded handler — no `set_handler()` call, no `map_<property>(...)` calls:

```cpp
#include <mpapp/<snake>.hpp>

mpapp::<snake> w;
// w is bound to the platform handler in its ctor; assign properties directly.
```

**Mock-handler tests use the surface directly** so the test target stays link-isolated from the per-platform handler library:

```cpp
#include <mpapp/<snake>.hpp>
#include <mpapp/handlers/mock/<snake>_handler.hpp>

mpapp::internal::basic_<snake> w;
mpapp::<snake>_handler<mpapp::platform::mock> h;
// h.map_<property>(w);  // exercise the mapper contract
```

If this component is an [[ADR-0024-wrapper-component-pattern|ADR-0024]] exception (program-entry class or static facility) or an abstract base class, replace the table above with a `> [!info]` callout explaining which case applies.

## MAUI Reference

- Handler: `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\<HandlerName>\`
- Control: `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\<ControlName>\`
- Docs: `<mauiDocUrl>`

## MPAPP C++ API

```cpp
// To be designed.
```

## XAML Usage

```xml
<!-- To be designed. Should match MAUI XAML 1:1 per ADR-0004. -->
```

## Platform Notes

| Platform | Native control | Notes |
|---|---|---|
| Windows | | |
| Android | | |
| Linux | | |
| macOS | | |
| iOS | | |

## Side-by-side Examples

### MAUI

```xml
```

### MPAPP (XAML)

```xml
```

### MPAPP (C++)

```cpp
```

## Tests

Linked from the per-platform handler tasks.

## Known Differences

| Aspect | MAUI behavior | MPAPP behavior | Reason |
|---|---|---|---|
