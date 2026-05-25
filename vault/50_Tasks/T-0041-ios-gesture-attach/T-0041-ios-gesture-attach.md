---
type: task
id: T-0041
title: iOS real gesture wire-up — ios_gestures::attach over UIGestureRecognizer
status: todo
milestone: M-08
owner: ""
area: handlers
blockedBy:
  - T-0040
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/handlers
  - phase/p7
  - platform/ios
---

# T-0041 — iOS real gesture wire-up

## Goal

UIKit analog of `linux_gestures::attach` — install `UIGestureRecognizer` subclasses on the seed `UIView*` that each per-component iOS handler owns. Replaces the no-op `map_gestures` stubs landed in commit `b0a999d`.

## Per-recognizer wire-up table

| Recognizer | UIKit API |
|---|---|
| `tap` | `UITapGestureRecognizer` — `numberOfTapsRequired = number_of_taps_required`. |
| `pan` | `UIPanGestureRecognizer` — `translationInView:` + state. |
| `pinch` | `UIPinchGestureRecognizer` — `scale` is the cumulative ratio; we report it as incremental per tick. |
| `swipe` | `UISwipeGestureRecognizer` per direction (or one with `direction` set) — the recognizer itself fires `state == .ended` on success, so no manual filtering needed (matches MAUI semantics better than AppKit's pan-derivation). |
| `pointer` | `UIHoverGestureRecognizer` (iPadOS 13+) for enter/exit/move; touch events for press/release. |

## Acceptance Criteria

- [ ] `include/mpapp/handlers/ios/gesture_attach.hpp` + `src/handlers/ios/gesture_attach.mm` ship.
- [ ] Per-component `map_gestures` stubs replaced.
- [ ] Simulator host available (depends on [[T-0040-macos-gesture-attach]] for the Apple host work).
- [ ] Rule-11 closure: `XCUIApplication`-driven tap on a non-button widget in iOS Simulator.

## Blocker

[[T-0040-macos-gesture-attach]] first — Apple-host setup is shared between macOS and iOS; closing macOS removes most of the iOS plumbing risk.

## Links

- RFC: [[RFC-0003-gesture-recognizers]] §Detailed Design.
- iOS host gating: [[M-08-iOS-Real]].
- Linux precedent: [[T-0037-linux-gesture-bulk-port]].
