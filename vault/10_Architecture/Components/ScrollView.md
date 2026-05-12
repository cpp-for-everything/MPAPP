---
type: component
mauiHandler: "ScrollView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/scrollview"
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

# ScrollView

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

To be filled. What does ScrollView do for the user?

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\ScrollView\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\ScrollView\`
- **Docs:** [Microsoft .NET MAUI — ScrollView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/scrollview)

## MPAPP C++ API

```cpp
namespace mpapp {

class scrollview : public control<scrollview> {
public:
    // Properties to be designed.

    // Events / commands to be designed.
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<ScrollView/>
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

- Mock tests: `tests/components/scrollview/mock_test.cpp` (planned)
- Windows handler: `tests/components/scrollview/windows_test.cpp` (planned)
- Android handler: `tests/components/scrollview/android_test.cpp` (planned)
- Linux handler: `tests/components/scrollview/linux_test.cpp` (planned)
- macOS handler: `tests/components/scrollview/macos_test.cpp` (planned)
- iOS handler: `tests/components/scrollview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
