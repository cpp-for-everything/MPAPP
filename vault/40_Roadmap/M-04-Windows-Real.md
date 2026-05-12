---
type: milestone
id: M-04
title: Windows real platform — WinUI 3 handlers and hot reload
phase: P3
status: planned
deliverables:
  - WinUI 3 handlers for every mocked control
  - Sample apps (calculator, todo, gallery) as native Windows apps
  - UI test automation via WinAppDriver / UIA
  - Hot reload on Windows desktop landed
exitCriteria:
  - "Every Controls Inventory row at mpappStatus: windows-real"
  - "platformWindows: true on every component"
  - "Gallery app passes automated UI smoke tests on every PR"
  - "Windows hot reload working for both XAML and C++"
tags:
  - type/milestone
  - phase/p3
  - status/planned
  - platform/windows
---

# M-04 — Windows Real Platform

> [!info] Status
> **planned**. Starts after [[M-03-Mock-Surface]] closes.

## Scope

Convert every dummy `*_handler<platform::mock>` into a real `*_handler<platform::windows>` using C++/WinRT and WinUI 3. This is the first real platform — its sample apps (calculator, todo, gallery) become the smoke-test surface for every subsequent platform.

## Exit Criteria

- [ ] Every component has a working `*_handler<platform::windows>` implementation.
- [ ] Every component's `platformWindows: true` in frontmatter.
- [ ] Calculator, todo, and gallery sample apps run as native unpackaged WinUI 3 apps.
- [ ] WinAppDriver / UIA tests for the gallery green in CI on every PR.
- [ ] Hot reload working on Windows desktop for both XAML and C++.

## Risks

> [!warning]
> - C++/WinRT generics with WinUI 3 controls have sharp edges; expect some hand-written `IInspectable` boilerplate.
> - WinUI 3 unpackaged story has matured but installer corner cases exist; test on clean VMs.

## Tasks

Linked via [[_Bases/Tasks.base]] filtered by `milestone == "M-04"`. Tasks include a per-component handler PR and a per-sample-app integration task.

## Related

- [[Platform Interop]]
- [[Handlers]]
- [[Hot Reload]]
- [[70_References/CppWinRT]]
