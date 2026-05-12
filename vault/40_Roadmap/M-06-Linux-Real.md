---
type: milestone
id: M-06
title: Linux real platform — GTK4 handlers via WSLg
phase: P5
status: planned
deliverables:
  - GTK4 handlers for every mocked control
  - WSLg primary dev surface validated
  - Hyper-V VM fallback documented
  - Hot reload on Linux desktop
exitCriteria:
  - "Every Controls Inventory row at mpappStatus: linux-real"
  - "platformLinux: true on every component"
  - "Native Linux build green in CI on ubuntu-latest"
tags:
  - type/milestone
  - phase/p5
  - status/planned
  - platform/linux
---

# M-06 — Linux Real Platform

> [!info] Status
> **planned**. Starts after [[M-05-Android-Real]] closes.

## Scope

GTK4 handlers. WSLg is the primary dev surface on the user's Windows host; native Linux build verified in CI.

## Exit Criteria

- [ ] Every component has a working `*_handler<platform::linux>` against GTK4.
- [ ] Every component's `platformLinux: true`.
- [ ] WSLg-based dev loop documented.
- [ ] Native Linux CI on `ubuntu-latest` green.
- [ ] Hot reload working on Linux desktop.

## Risks

> [!warning]
> - GTK4 LGPL constraints (dynamic linking only) per [[RFC-0001-licensing-and-patent-strategy]].
> - WSLg GTK4 may have edge cases vs native; document divergences.

## Tasks

Linked via [[_Bases/Tasks.base]] filtered by `milestone == "M-06"`.

## Related

- [[Platform Interop]]
- [[70_References/GTK4]]
- [[70_References/WSLg]]
- [[Hot Reload]]
- [[RFC-0001-licensing-and-patent-strategy]]
