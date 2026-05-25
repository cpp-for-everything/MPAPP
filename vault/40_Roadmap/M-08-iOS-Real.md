---
type: milestone
id: M-08
title: iOS real platform — UIKit handlers and parity-complete
phase: P7
status: planned
deliverables:
  - UIKit (NOT Catalyst) handlers for every mocked control
  - iOS Simulator CI via xcrun simctl
  - Hot reload on iOS Simulator
  - iOS real-device hot reload investigation results documented
exitCriteria:
  - "Every Controls Inventory row at mpappStatus: ios-real, transitioning to parity-complete"
  - "platformIos: true on every component"
  - "All 55 components reach parity-complete"
  - "Conformance test suite green on all 5 platforms"
tags:
  - type/milestone
  - phase/p7
  - status/planned
  - platform/ios
---

# M-08 — iOS Real Platform (parity-complete)

> [!info] Status
> **planned**. Starts after [[M-07-macOS-Real]] closes.

## Scope

UIKit handlers, separate from AppKit per [[ADR-0005-ios-macos-separate-interop]]. This milestone closes parity for all 55 components — every row in [[Controls Inventory]] reaches `parity-complete`.

## Exit Criteria

- [ ] Every component has a working `*_handler<platform::ios>` against UIKit.
- [ ] Every component's `platformIos: true`.
- [ ] All 55 components reach `mpappStatus: parity-complete`.
- [ ] iOS Simulator UI tests via `xcrun simctl`.
- [ ] Hot reload working on iOS Simulator.
- [ ] iOS real-device hot reload investigation documented (likely "no, but here's the entitlement workaround for dev signing").
- [ ] Conformance test suite passes on all 5 platforms — [[Interop Parity]] verified.

## Readiness checklist

The seed `.mm` files under `src/handlers/ios/` follow the post-[[ADR-0024-wrapper-component-pattern]] structure (the `mpapp::internal::` move + `basic_<name>&` parameters were applied in the bulk migration commit `4754ac1`). Wrapper-pattern is **not** a blocker — what is missing is the Apple-host workflow.

Concrete entry conditions, in order:

1. [[M-07-macOS-Real]] closed — both Apple platforms share toolchain + host concerns, and macOS has fewer moving parts than iOS Simulator. M-07 also stress-tests the same `.mm` Obj-C++ build path UIKit needs.
2. **iOS Simulator harness running on the self-hosted Mac.** `xcrun simctl boot` + `xcrun simctl launch` is enough for headless smoke; the visual-UI test harness (T-0008 design) reuses the macOS recipe with `XCUIApplication` instead of `NSApplication`.
3. **Component fill-in** — extend the seed UIKit handlers (`application_handler.mm`, `button_handler.mm`, `label_handler.mm`, `window_handler.mm`) to cover the full inventory. The wrapper-pattern work means the surgical pattern is `*_handler<platform::ios>::map_<prop>(internal::basic_<name>&)` — identical shape to the AppKit work — so a bulk-port automation script can do most of the boilerplate.
4. **Real-device hot reload investigation** — document the entitlement / signing constraints; likely the answer is "Simulator only" with a documented workaround for dev signing.
5. **Conformance test green on all 5 platforms** — the parity-complete bar from [[ADR-0006-interop-parity]].

## Reinstate trigger

This milestone moves from `planned` → `active` when [[M-07-macOS-Real]] is shipped. Doing them strictly in order avoids fighting the Apple-host runner setup twice and lets the AppKit-vs-UIKit code reuse decisions land while macOS is fresh.

## Risks

> [!warning]
> - iOS real-device hot reload may not be feasible without dev-only entitlements; document the boundary.
> - UIKit and AppKit share enough that some code reuse is possible — but resist the temptation to merge handlers, per ADR-0005.

## Tasks

Linked via [[_Bases/Tasks.base]] filtered by `milestone == "M-08"`.

## See in code

- Seed UIKit handlers (Objective-C++ `.mm`): [`src/handlers/ios/`](../../src/handlers/ios/) — `application_handler.mm`, `button_handler.mm`, `label_handler.mm`, `window_handler.mm`. App-shell-only today; per-component fill-in pending an Apple host.
- Distinct from macOS per [[ADR-0005-ios-macos-separate-interop]]: iOS = UIKit (`UIWindow`, `UIButton`, `UICollectionView`), macOS = AppKit (`NSWindow`, `NSButton`, `NSTableView`). Separate `.mm` files, no Catalyst path anywhere.
- Apple-target toolchain files: [`cmake/toolchains/ios-arm64.cmake`](../../cmake/toolchains/ios-arm64.cmake).
- Apple-target cross-compile T-0009 row remains `in-progress` until an Apple host is online — see [`vault/50_Tasks/T-0009-cross-compilation-matrix/`](../50_Tasks/T-0009-cross-compilation-matrix/).

## Related

- [[ADR-0005-ios-macos-separate-interop]]
- [[ADR-0006-interop-parity]]
- [[Hot Reload]]
- [[Test Harness]]
