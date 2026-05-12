---
type: component
mauiHandler: "NavigationPage"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/navigationpage"
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

# NavigationPage

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

To be filled. What does NavigationPage do for the user?

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\NavigationPage\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\NavigationPage\`
- **Docs:** [Microsoft .NET MAUI — NavigationPage](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/navigationpage)

## MPAPP C++ API

```cpp
namespace mpapp {

class navigationpage : public control<navigationpage> {
public:
    // Properties to be designed.

    // Events / commands to be designed.
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<NavigationPage/>
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

- Mock tests: `tests/components/navigationpage/mock_test.cpp` (planned)
- Windows handler: `tests/components/navigationpage/windows_test.cpp` (planned)
- Android handler: `tests/components/navigationpage/android_test.cpp` (planned)
- Linux handler: `tests/components/navigationpage/linux_test.cpp` (planned)
- macOS handler: `tests/components/navigationpage/macos_test.cpp` (planned)
- iOS handler: `tests/components/navigationpage/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
