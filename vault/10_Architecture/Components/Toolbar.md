---
type: component
mauiHandler: "Toolbar"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/toolbar"
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

# Toolbar

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

To be filled. What does Toolbar do for the user?

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\Toolbar\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\Toolbar\`
- **Docs:** [Microsoft .NET MAUI — Toolbar](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/toolbar)

## MPAPP C++ API

```cpp
namespace mpapp {

class toolbar : public control<toolbar> {
public:
    // Properties to be designed.

    // Events / commands to be designed.
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<Toolbar/>
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

- Mock tests: `tests/components/toolbar/mock_test.cpp` (planned)
- Windows handler: `tests/components/toolbar/windows_test.cpp` (planned)
- Android handler: `tests/components/toolbar/android_test.cpp` (planned)
- Linux handler: `tests/components/toolbar/linux_test.cpp` (planned)
- macOS handler: `tests/components/toolbar/macos_test.cpp` (planned)
- iOS handler: `tests/components/toolbar/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
