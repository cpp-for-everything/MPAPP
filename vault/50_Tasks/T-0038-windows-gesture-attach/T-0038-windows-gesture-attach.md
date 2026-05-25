---
type: task
id: T-0038
title: Windows real gesture wire-up — windows_gestures::attach over WinUI 3 / C++/WinRT
status: todo
milestone: M-04
owner: ""
area: handlers
blockedBy:
  - T-0032
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/handlers
  - phase/p4
  - platform/windows
---

# T-0038 — Windows real gesture wire-up

## Goal

Land the Windows analog of `linux_gestures::attach` (commit `c5c4d1a`): a `windows_gestures::attach(winrt::Microsoft::UI::Xaml::UIElement, view&)` free function that walks `view::gesture_recognizers` and installs the matching WinUI 3 listener for each `kind()`. After this lands, the per-component `<name>_handler<platform::windows>::map_gestures` stubs land in commit `b0a999d` get a real implementation in a follow-up bulk sweep (same pattern as Linux T-0037).

## Per-recognizer wire-up table

| Recognizer | WinUI 3 API |
|---|---|
| `tap` | `UIElement::Tapped` (`RoutedEvent`). Multi-tap via `UIElement::DoubleTapped` for `number_of_taps_required == 2`; gate manually for higher counts. |
| `pan` | `UIElement::ManipulationDelta` + `UIElement::ManipulationStarted` + `UIElement::ManipulationCompleted`. Set `UIElement::ManipulationMode = TranslateX \| TranslateY` before subscribing. |
| `pinch` | `UIElement::ManipulationDelta` reading `ManipulationDeltaRoutedEventArgs.Delta.Scale`. Set `ManipulationMode = Scale`. |
| `swipe` | `UIElement::ManipulationCompleted` reading `ManipulationCompletedRoutedEventArgs.Cumulative.Translation` — derive dominant axis + sign, gate against `direction` bitmask + `threshold`. |
| `pointer` | `UIElement::PointerEntered / PointerExited / PointerMoved / PointerPressed / PointerReleased`. `PointerRoutedEventArgs.GetCurrentPoint(element).Position` gives view-local position. |

## Acceptance Criteria

- [ ] `include/mpapp/handlers/windows/gesture_attach.hpp` declares `void attach(::winrt::Microsoft::UI::Xaml::UIElement, view&)` under `mpapp::internal::windows_gestures`.
- [ ] `src/handlers/windows/gesture_attach.cpp` implements all 5 recognizers; lifetimes match the Linux ownership model (event tokens stored alongside the recognizer raw pointer; both die with the wrapper before the surface's `gesture_recognizers` vector unwinds).
- [ ] `button_handler<platform::windows>::map_gestures` calls `windows_gestures::attach(handler().native(), b)`.
- [ ] Bulk-sweep the other 58 components' `map_gestures` to call the real impl. Same script as `tools/dev/sweep-gesture-map.py` with a `--platform=windows` switch — extend the existing one rather than fork.
- [ ] At least the cloud `windows-native` job (once reinstated per T-0032) compiles + links clean.
- [ ] Rule-11 closure: recording of a tap landing on a non-button widget on a Windows desktop.

## Blocker

`mpapp-core` does not build on the cloud `windows-latest` runner without WindowsAppSDK provisioning ([[T-0032-windows-appsdk-ci-provisioning]]). T-0032's Path B (decouple `mpapp-core` from the umbrella) gets us to "code compiles standalone"; Path A is required for tests to actually run. Either order works — T-0038 can proceed locally on the project lead's Windows machine where WindowsAppSDK is installed, and CI catches up once T-0032 lands.

## Links

- RFC: [[RFC-0003-gesture-recognizers]] §Detailed Design.
- Linux precedent: [[T-0037-linux-gesture-bulk-port]].
- CI blocker: [[T-0032-windows-appsdk-ci-provisioning]].
