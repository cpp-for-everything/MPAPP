---
type: task
id: T-0005
title: Populate Controls Inventory and per-component docs from MAUI source
status: done
milestone: M-01
owner: ""
area: docs
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/docs
  - phase/p0
---

# T-0005 — Inventory MAUI controls

## Goal

Walk `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\` and `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\` and populate every per-component note in [[Components/]] with substantive content extracted from the MAUI source: overview, default property list, supported XAML usage, link to Microsoft docs.

The stubs already exist (day 1 created 56). This task fills them in with real MAUI-derived content.

## Acceptance Criteria

- [ ] Every component note in [[Components/]] has:
  - [ ] **Overview** section filled in (from MAUI XML doc comments or Microsoft docs).
  - [ ] **MAUI Reference** has accurate file paths.
  - [ ] At least the **most common properties / events** listed (even if the C++ API is TBD).
  - [ ] A representative **MAUI XAML example** in the Side-by-side section.
- [ ] [[Controls Inventory]] snapshot table updated with any new components discovered.
- [ ] Notes folder records any controls in MAUI not yet mirrored (e.g. legacy / compatibility classes) with a decision: include or omit.
- [ ] This task is mostly research/writing — no code coverage is required. Set `coveragePercent: 100` once all 56 notes are fleshed out.

## Notes

This is a large but mechanical task. Could be split into batches (10 components per sub-task) if more granular tracking helps.

## Links

- Milestone: [[M-01-Foundations]]
- Related: [[Controls Inventory]], [[Components/README]], [[XAML Compatibility]]
- Source: `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\`, `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\`

## Closure notes

- **Closed:** 2026-05-12
- **Merged commits:** Day-1 inventory stubs (vault init) plus Batch 1 component-doc population — `f5f462c` (inputs, 9), `222c9d2` (pickers/images, 8), `0e16288` (navigation, 6), `1e60b8d` (menus/toolbars, 9), `a003f84` (app-level, 8), `b5a2f96` (layout, 7), `c9ce958` (collections/complex, 9), and downstream merges `cff83de`, `532ef93`, `a0bdb04`, `c7a0169`, `5cde621`, `d98267d`, `6f28fbc`.
- **Delivered:** All 56 per-component docs under `vault/10_Architecture/Components/` populated from MAUI source with overview, MAUI reference paths, common properties/events, and representative XAML side-by-side examples. Controls Inventory snapshot kept in sync.
- **Coverage:** docs-only task — no code coverage applicable; gate set to `100%` per the task spec.
