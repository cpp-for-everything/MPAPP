---
type: moc
title: MPAPP Vault — Home
tags:
  - type/moc
cssclasses:
  - mpapp-home
---

# MPAPP Vault — Home

Welcome to the MPAPP vault. **AI agents:** start with [[CLAUDE]]. **Humans:** see [[README]] for onboarding.

> [!info] Current focus
> See [[Current Focus]] — updated weekly.

---

## Top risks (banner)

> [!warning] Top risks
> 1. **`mpapp-xc` is the bus factor.** Ship it as a product with its own test suite.
> 2. **Diagnostic quality determines adoption.** Source-mapped errors every phase.
> 3. **CI minutes are scarce.** Shard aggressively. See [[CI Strategy]].
> 4. **Apple platform automation** — UI tests must be human-free from P1.
> 5. **License contagion.** GTK4 LGPL — dynamic linking only. See [[ADR-0010-licensing-and-patent-strategy]].
> 6. **Interop parity drift.** Every PR lists which platforms it affects. See [[Interop Parity]].
> 7. **MAUI component surface is large.** 55 components × 5 platforms = lots of work.
> 8. **Patent prior-art audit** required before any filing.
> 9. **iOS real-device hot reload** is unclear — track as research, don't promise.

---

## Maps of Content

- [[Architecture MOC]] — the technical design surface.
- [[Decisions MOC]] — index of accepted ADRs by area.
- [[Roadmap MOC]] — phases, milestones, and exit criteria.
- [[Components MOC]] — per-component documentation index.

---

## Live dashboards

### Open RFCs

![[_Bases/RFCs.base]]

### Open tasks

![[_Bases/Tasks.base]]

### Blockers

![[_Bases/Blockers.base]]

### Recently accepted ADRs

![[_Bases/ADRs.base]]

### Component porting status

![[_Bases/Components.base]]

### Milestones

![[_Bases/Roadmap.base]]

---

## The 12 Rules (summary)

See [[CLAUDE]] for the full text.

1. **No macros in public API** (ADR-0002, ADR-0009)
2. **Interop parity** — every feature on every platform (ADR-0006)
3. **No time estimates, ever**
4. **No editing accepted ADRs** — supersede instead
5. **Component work tracked in the inventory** ([[Controls Inventory]])
6. **Mock before real** (ADR-0008)
7. **MAUI is the spec** — check `D:\GitHub\MPAPP\references\maui\src\` first
8. **CI budget awareness** — see [[CI Strategy]]
9. **License vigilance** — see [[70_References/Third-Party Dependencies]]
10. **Read [[Current Focus]] first**
11. **Task closure gate** — 100% coverage + screenshots/recordings, then archive
12. **Cross-platform tooling** — all tools run on Windows + macOS + Linux (ADR-0007)

---

## Quick navigation

- **21 accepted + 2 proposed ADRs** (proposed are ADR-0015 graphics dual + ADR-0023 shell route guards): see [[Decisions MOC]] or [[_Bases/ADRs.base]]
- 0 open RFCs (RFC-0001 and RFC-0002 both accepted): see [[_Bases/RFCs.base]]
- **12 milestones** (M-04c `active`): [[Roadmap MOC]] or [[_Canvases/Phase-Roadmap.canvas|Phase Roadmap canvas]]
- **64 components, 61 at `android-real` on Win + Linux + Android**: [[Components MOC]] or [[Controls Inventory]]
- Architecture canvases: [[_Canvases/Architecture-Overview.canvas|Architecture]], [[_Canvases/Build-Dependency-Graph.canvas|Build Graph]], [[_Canvases/Phase-Roadmap.canvas|Roadmap]], [[_Canvases/Interop-Parity-Matrix.canvas|Interop Parity]], [[_Canvases/Cross-Compilation-Matrix.canvas|Cross-compilation]]
- Most recent log: [[90_Logs/2026-W21-autonomous-m04c-push|2026-W21 autonomous M-04c push]]
- Decision log: [[Decision Log]]
- MAUI deep-dive (moved-in research): [[dotnet-maui-deep-dive]]
