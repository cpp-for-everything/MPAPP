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

## Risks

> [!warning]
> - iOS real-device hot reload may not be feasible without dev-only entitlements; document the boundary.
> - UIKit and AppKit share enough that some code reuse is possible — but resist the temptation to merge handlers, per ADR-0005.

## Tasks

Linked via [[_Bases/Tasks.base]] filtered by `milestone == "M-08"`.

## Related

- [[ADR-0005-ios-macos-separate-interop]]
- [[ADR-0006-interop-parity]]
- [[Hot Reload]]
- [[Test Harness]]
