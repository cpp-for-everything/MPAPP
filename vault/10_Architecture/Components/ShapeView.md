---
type: component
mauiHandler: "ShapeView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/shapeview"
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

# ShapeView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

To be filled. What does ShapeView do for the user?

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\ShapeView\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\ShapeView\`
- **Docs:** [Microsoft .NET MAUI — ShapeView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/shapeview)

## MPAPP C++ API

```cpp
namespace mpapp {

class shapeview : public control<shapeview> {
public:
    // Properties to be designed.

    // Events / commands to be designed.
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<ShapeView/>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | TBD | C++/WinRT | |
| Android | TBD | fbjni / JNI | |
| Linux | TBD | GTK4 | |
| macOS | TBD | AppKit | |
| iOS | TBD | UIKit | |

## Side-by-side Examples

### MAUI

```xml
<!-- TBD -->
```

### MPAPP (XAML)

```xml
<!-- TBD -->
```

### MPAPP (C++)

```cpp
// TBD
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/shapeview/mock_test.cpp` (planned)
- Windows handler: `tests/components/shapeview/windows_test.cpp` (planned)
- Android handler: `tests/components/shapeview/android_test.cpp` (planned)
- Linux handler: `tests/components/shapeview/linux_test.cpp` (planned)
- macOS handler: `tests/components/shapeview/macos_test.cpp` (planned)
- iOS handler: `tests/components/shapeview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
