---
type: task
id: T-0063
title: Accessibility — semantic_description (accessible name) on controls
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

# T-0063 — Accessibility: semantic_description

## Goal

Start the goal's "accessibilities" requirement: give controls a screen-reader
accessible name (MAUI `SemanticProperties.Description`) and apply it to the
native accessibility tree on every platform.

## Scope

In: `view::semantic_description` (`Observable<std::string>`, "" = unset) on the
cross-platform base; `basic_button` applies it via `map_semantics` on **all
platforms** — GTK4 (`gtk_accessible_update_property` LABEL), WinUI 3
(`AutomationProperties.Name`), Android (`View.setContentDescription`), AppKit
(`setAccessibilityLabel:`) + UIKit (`accessibilityLabel`) blind; the
`mpapp::button` wrapper auto-maps it. Mock recorder + test. УИСС gives the ≡
hamburger an accessible name ("Отвори навигацията") since the glyph alone is
meaningless to assistive tech.
Out: applying to every control (label/entry/picker/… — labels expose their text
to AT natively); `semantic_hint` / `HeadingLevel`; focus order. Follow-ups.

## Per-platform verification

| Platform | Result |
|---|---|
| Linux WSL/GTK4 | ✅ full build (143 targets) + ctest `[button]` (added `[a11y]` case); GTK accessible LABEL applied. |
| Windows MSVC/WinUI 3 | ⏳ uiss rebuild (AutomationProperties.Name). |
| Android NDK r26 | ✅ button_handler.cpp (setContentDescription) + uiss main cross-compile (aarch64). |
| Apple | ❌ no host — AppKit/UIKit accessibilityLabel written blind. |

## Acceptance Criteria

- [x] `semantic_description` on `view`; mock records + test.
- [x] `basic_button` applies it on all platforms; wrapper auto-maps.
- [x] УИСС names the ≡ hamburger for screen readers.
- [ ] Extend to more controls + `semantic_hint`/heading level (follow-up).

## Links

- Used by [[T-0060-uiss-reference-app]]. First slice of the goal's accessibility requirement.
