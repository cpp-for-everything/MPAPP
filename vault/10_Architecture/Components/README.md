---
type: moc
area: handlers
tags:
  - area/handlers
  - area/markup
---

# Components

This folder contains **one note per MAUI control / handler** MPAPP must implement. Each note is the authoritative documentation for that control: its public C++ API, XAML usage, platform notes, side-by-side examples with MAUI, tests, and known differences.

The full inventory of 55 components lives in [[Controls Inventory]]. The live filterable view is [[_Bases/Components.base]].

## Per-component note structure

Every component note uses the [[_Templates/Component]] template. Frontmatter:

```yaml
type: component
mauiHandler: <MAUI handler name>
mauiDocUrl: <Microsoft docs URL>
mpappStatus: not-started | mock | windows-real | android-real | linux-real | macos-real | ios-real | parity-complete
platformWindows: false | true
platformAndroid: false | true
platformLinux: false | true
platformMacos: false | true
platformIos: false | true
```

Body sections (in order):

1. **Overview** — what the control does for the user.
2. **MAUI Reference** — links to MAUI source and docs.
3. **MPAPP C++ API** — the public type signature.
4. **XAML Usage** — XAML syntax (must match MAUI per [[ADR-0004-maui-xaml-superset-compat]]).
5. **Platform Notes** — per-platform native widget mapping.
6. **Side-by-side Examples** — MAUI XAML / MPAPP XAML / MPAPP C++.
7. **Implementation** — links to the actual source files: surface header (`include/mpapp/<snake>.hpp`), mock handler (`include/mpapp/handlers/mock/<snake>_handler.hpp`), real per-platform handlers (`src/handlers/<plat>/<snake>_handler.cpp`), and tests (`tests/mock_handlers/<snake>_test.cpp`). Cross-link from the prose status callout when the platform-specific narrative names a native widget that lives in a specific file.
8. **Tests** — links to test files (often folded into Implementation).
9. **Known Differences** — documented divergences (any cell here is a candidate bug or RFC).

Component-name → snake-case rule: PascalCase header name with a separator inserted before each uppercase letter (`BoxView` → `box_view`, `CollectionView` → `collection_view`). Two exceptions today: `Grid` → `grid_layout` (avoids shadowing layout API), `Layout` → `layout` (abstract base — has surface + mock + tests but no per-platform handler).

## Status conventions

| Status | Meaning |
|---|---|
| `not-started` | Note exists but no design or code yet |
| `mock` | C++ class with full public API + dummy handler implementation (per [[ADR-0008-mock-first-implementation]]) |
| `windows-real` | Real WinUI 3 handler complete; passes platform UI tests |
| `android-real` | Real fbjni / Android handler complete |
| `linux-real` | Real GTK4 handler complete |
| `macos-real` | Real AppKit handler complete |
| `ios-real` | Real UIKit handler complete |
| `parity-complete` | All five platforms shipped; passes conformance + UI tests |

## See also

- [[Controls Inventory]]
- [[XAML Compatibility]]
- [[Handlers]]
- [[_Templates/Component]]
