---
type: component
mauiHandler: "SearchBar"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/searchbar"
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

# SearchBar

> [!info] Status
> **not-started** — placeholder. See [[Controls Inventory]] for the full porting matrix.

## Overview

To be filled. What does SearchBar do for the user?

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\SearchBar\`
- **Control:** `D:\GitHub\MPAPP\maui\src\Controls\src\Core\SearchBar\`
- **Docs:** [Microsoft .NET MAUI — SearchBar](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/searchbar)

## MPAPP C++ API

```cpp
namespace mpapp {

class searchbar : public control<searchbar> {
public:
    // Properties to be designed.

    // Events / commands to be designed.
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<SearchBar/>
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

- Mock tests: `tests/components/searchbar/mock_test.cpp` (planned)
- Windows handler: `tests/components/searchbar/windows_test.cpp` (planned)
- Android handler: `tests/components/searchbar/android_test.cpp` (planned)
- Linux handler: `tests/components/searchbar/linux_test.cpp` (planned)
- macOS handler: `tests/components/searchbar/macos_test.cpp` (planned)
- iOS handler: `tests/components/searchbar/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
