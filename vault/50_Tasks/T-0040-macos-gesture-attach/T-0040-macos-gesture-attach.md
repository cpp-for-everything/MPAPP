---
type: task
id: T-0040
title: macOS real gesture wire-up — macos_gestures::attach over NSGestureRecognizer
status: todo
milestone: M-07
owner: ""
area: handlers
blockedBy:
  - M-07
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/handlers
  - phase/p6
  - platform/macos
---

# T-0040 — macOS real gesture wire-up

## Goal

AppKit analog of `linux_gestures::attach` — install `NSGestureRecognizer` subclasses on the seed `NSView*` that each per-component macOS handler owns. Replaces the no-op `map_gestures` stubs landed in commit `b0a999d`.

## Per-recognizer wire-up table

| Recognizer | AppKit API |
|---|---|
| `tap` | `NSClickGestureRecognizer` — `numberOfClicksRequired = number_of_taps_required`; `buttonMask` from `button_mask`. |
| `pan` | `NSPanGestureRecognizer` — `translation(in:view:)` + state (`began`/`changed`/`ended`/`cancelled` map to gesture_status). |
| `pinch` | `NSMagnificationGestureRecognizer` — `magnification` is incremental scale. |
| `swipe` | `NSPanGestureRecognizer` configured with a direction filter — derive single direction from final velocity, gate on bitmask + threshold (NSSwipeGestureRecognizer exists but only handles trackpad 3-finger swipes; pan-with-derivation matches MAUI's single-finger flick semantics). |
| `pointer` | `NSTrackingArea` (mouseEntered/exited/moved) + `mouseDown:` / `mouseUp:` on a custom view subclass. |

## Acceptance Criteria

- [ ] `include/mpapp/handlers/macos/gesture_attach.hpp` + `src/handlers/macos/gesture_attach.mm` ship.
- [ ] Per-component `map_gestures` stubs replaced.
- [ ] Apple host available (see [[M-07-macOS-Real]]'s readiness checklist).
- [ ] Rule-11 closure: macOS Accessibility-API-driven test fires a tap on a non-button widget.

## Blocker

[[M-07-macOS-Real]] — needs an Apple host to validate the `.mm` builds + run a UI test.

## Links

- RFC: [[RFC-0003-gesture-recognizers]] §Detailed Design.
- Apple host gating: [[M-07-macOS-Real]].
- Linux precedent: [[T-0037-linux-gesture-bulk-port]].
