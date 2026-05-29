---
type: task
id: T-0061
title: Label typography — font size / weight / family on basic_label
status: done
milestone: M-10
owner: ""
area: widgets
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/widgets
  - phase/p2
---

# T-0061 — Label typography

## Goal

Close RFC-0015 gap #1 (and the goal's "fonts loading" requirement): give
`mpapp::label` real font control so the УИСС portal headings, titles and chrome
render with weight/size instead of flat body text.

## Scope

In: `basic_label` surface (`font_size` double / `font_bold` bool / `font_family`
string / `text_color` `mpapp::color`) + mock recorder + tests; real handlers on
**all platforms** — GTK4 (Pango `PangoAttrList` incl. foreground attr), WinUI 3
(`FontSize/FontWeight/FontFamily` + `Foreground` SolidColorBrush), Android
(`TextView.setTextSize(PT)` + `setTypeface` + `setTextColor`), AppKit (`NSFont` +
`NSColor`) + UIKit (`UIFont` + `UIColor`) blind; `mpapp::label` wrapper auto-maps
them in its ctor. `mpapp::color` extracted to `include/mpapp/color.hpp` (shared
by box_view + label). Applied in УИСС (`section_page` headings/title in TU navy
`#1D3557`, login title, nav menu).
Out: RFC-0012 custom-font-file registration resolving `font_family` aliases.

## Per-platform verification

| Platform | Result |
|---|---|
| Linux WSL/GTK4 | ✅ full build (45 targets) + **ctest 466/466** (added 2 `[label]` font cases); Pango attrs applied. |
| Windows MSVC/WinUI 3 | ✅ `label_handler.cpp` (FontSize/FontWeight/FontFamily) + `uiss.exe` relinked — `BUILD_UISS_SUCCESS`. |
| Android NDK r26 | ✅ `src/handlers/android/label_handler.cpp` (JNI setTextSize/setTypeface) + `examples/uiss/main.cpp` cross-compile clean (aarch64, EXIT=0). |
| Apple | ❌ no host — AppKit/UIKit font code written blind. |

## Acceptance Criteria

- [x] `font_size` / `font_bold` / `font_family` / `text_color` on `basic_label`; mock records + tests (3 `[label]` cases).
- [x] `mpapp::label` wrapper auto-maps the new properties.
- [x] Real handlers on GTK4 + Android (verified) + WinUI/AppKit/UIKit (compile).
- [x] УИСС applies bold/sized + TU-navy headings + titles.
- [x] `text_color` — `mpapp::color` extracted to `color.hpp`; real on all platforms.

## Links

- RFC: [[RFC-0015-uiss-reference-app]] (gap #1). Composes with [[RFC-0012-fonts]].
