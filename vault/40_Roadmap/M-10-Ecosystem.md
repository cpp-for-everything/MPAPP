---
type: milestone
id: M-10
title: Ecosystem and packaging — vcpkg, Conan, docs, migration guide, licensing
phase: P9
status: planned
deliverables:
  - vcpkg recipe published
  - Conan recipe published
  - Docs site (mirroring MAUI docs site structure)
  - MAUI → MPAPP migration guide
  - Conformance test suite published as a tagged release
  - Licensing finalized per RFC-0001 (dual license + CLA workflow)
exitCriteria:
  - "MPAPP installable via vcpkg install mpapp"
  - "MPAPP installable via conan install mpapp"
  - "Migration guide covers all 55 components"
  - "Conformance suite available for third-party verification"
  - "Dual-license + CLA workflow live on GitHub"
tags:
  - type/milestone
  - phase/p9
  - status/planned
  - area/docs
  - area/legal
---

# M-10 — Ecosystem & Packaging

> [!info] Status
> **planned**.

## Scope

Make MPAPP installable, documentable, and commercially viable. This is also the milestone where the licensing strategy ([[RFC-0001-licensing-and-patent-strategy]]) operationalizes: dual license, CLA assistant, third-party-dep registry stable.

## Exit Criteria

- [ ] **Packaging**:
  - [ ] `vcpkg install mpapp` works.
  - [ ] `conan install mpapp` works.
- [ ] **Docs**:
  - [ ] Docs site live (structure mirrors MAUI's so MAUI users feel at home).
  - [ ] Migration guide for every of the 55 components.
- [ ] **Conformance**:
  - [ ] Test suite tagged and published.
- [ ] **Licensing**:
  - [ ] Apache 2.0 + commercial dual license live.
  - [ ] CLA assistant integrated on GitHub.
  - [ ] Third-party dependency register published.
  - [ ] Patent prior-art audit complete; filing decisions made (per RFC-0001 §4).

## Risks

> [!warning]
> - vcpkg / Conan recipes have their own maintenance burden — automate updates.
> - Docs site quality is a Bus Factor — track contributor count.

## Tasks

Linked via [[_Bases/Tasks.base]].

## Related

- [[RFC-0001-licensing-and-patent-strategy]]
- [[70_References/Third-Party Dependencies]]
