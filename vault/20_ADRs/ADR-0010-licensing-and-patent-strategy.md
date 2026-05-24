---
type: adr
id: ADR-0010
title: Licensing and patent strategy
status: accepted
decisionDate: 2026-05-12
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: legal
tags:
  - type/adr
  - status/accepted
  - area/legal
---

# ADR-0010 — Licensing and patent strategy

> [!success] Status
> **accepted** on 2026-05-12.

## Context

MPAPP is intended for **eventual commercialization** and **possibly patent filing**. Decisions made early about licensing, contributor agreements, copyright ownership, and dependency choices determine whether those options remain available later. Reversing a permissive-license choice after the fact is essentially impossible; collecting retroactive copyright/patent grants from a long tail of contributors is at best painful and at worst infeasible. The same is true of dependency hygiene: a single transitively-pulled GPL library can poison the entire distribution.

[[RFC-0001-licensing-and-patent-strategy]] catalogued five interlocking questions that must land together:

1. **Outbound license** — what license MPAPP itself ships under.
2. **Contributor agreement** — how copyright and patent grants from outside contributors are collected so the project retains the ability to sublicense.
3. **Copyright assignee** — which legal entity (individual or company) owns the assembled work.
4. **Dependency license posture** — which third-party license classes are acceptable and under what linking model.
5. **Patent strategy** — when, where, and on what to file, and how to clear prior art first.

This ADR closes RFC-0001 by promoting its recommendations to a binding decision. Rule 9 in [[CLAUDE]] already references RFC-0001 as authoritative for dependency vetting; this ADR makes that reference durable.

## Decision

We will operationalize the RFC-0001 recommendations as five concrete commitments:

1. **Outbound license: Apache 2.0 (OSS) + custom commercial (paid).** A dual-track model. The same source tree ships both ways; the license is a *grant to the user*, not a property of the bits. Apache 2.0 gives a strong explicit patent grant with a defensive termination clause, is acceptable to enterprise legal teams, and is the de-facto standard for serious infrastructure projects. The commercial license sits alongside it for users who require indemnification, priority support, custom builds, or who cannot accept Apache 2.0's notice-and-attribution requirements.

2. **Contributor License Agreement: Apache-style individual + corporate CLA, enforced via [cla-assistant.io](https://cla-assistant.io).** Every external contributor signs before their PR can merge. The CLA grants the project (a) a copyright license to use the contribution, (b) a patent license covering anything the contribution implements, and (c) the right to **re-license** under any future license — which is precisely what unlocks the commercial track. Drive-by typo fixes are not exempt; cla-assistant gates the merge button.

3. **Copyright assignee: Alex Tsvetanov (individual)**, with a reserved right to transfer the assembled copyright to a future LLC. The CLA's grant flows to "Alex Tsvetanov and successors/assignees" so that the eventual entity transfer requires no second signing round.

4. **Dependency license posture.** Permissive (Apache 2.0, MIT, BSD) is used freely. Weak copyleft (LGPL, MPL) is permitted only via **dynamic linking**, with published rebuild instructions per LGPL §4 — this is the path that covers [[70_References/GTK4]]. Strong copyleft (GPL, AGPL) is **forbidden** as a runtime dependency. Source-available or commercial-only licenses (Live++, Qt commercial) are **forbidden** unless the dependency is replaced or separately re-licensed. The classification is tracked per-dependency in [[70_References/Third-Party Dependencies]].

5. **Patent strategy.** A prior-art audit across MAUI, Qt, Slint, JUCE, Avalonia, Flutter, React Native, and Dear ImGui is **required before any filing**. Actual filing is deferred until **P3** — after Windows real-platform support ships — so that claims are anchored to working, demonstrable code rather than aspirational designs. Candidate novel claims (the `Computed<&member, &member>` template-dependency pattern, zero-cost link-time event-loop selection, the XAML-to-`consteval` pipeline with source-mapped diagnostics, and mock-first cross-platform handler validation) are listed in RFC-0001 §4 and will be re-evaluated at audit time. Jurisdiction (US first, PCT for follow-ons) and attorney engagement are sequenced after the audit.

## Consequences

### Positive

- The **commercial track is enabled** from day one — no retroactive contributor outreach needed.
- The **OSS adoption path is unblocked**: Apache 2.0 is a license enterprise legal teams have already cleared.
- The **patent option remains open** without being prematurely exercised on weak claims.
- Dependency posture is **explicit and auditable**, satisfying [[CLAUDE]] rule 9.

### Negative

- The **CLA adds friction** for first-time contributors — one extra signing step before a PR can merge.
- **LGPL dependencies require ongoing compliance discipline** (dynamic linking, rebuild instructions, license notice in distributables).
- **Dual-license bookkeeping** grows over time — every release ships with a NOTICE file and a commercial-license addendum that must stay in sync with the dependency manifest.

### Neutral

- The choice of CLA tool (cla-assistant.io vs alternatives) is revisitable without re-signing.
- The copyright assignee can later be transferred from Alex Tsvetanov to an LLC; the CLA wording explicitly contemplates this.

## Alternatives Considered

- **MIT-only.** Simpler text and zero ceremony, but no explicit patent grant. Without a patent grant the commercial story is weaker (enterprise buyers want indemnification anchored in an explicit grant) and defensive termination on patent litigation is absent. **Rejected.**
- **GPL.** Maximally protective against proprietary forks, but immediately kills enterprise adoption — many corporate legal policies forbid GPL runtime dependencies entirely. **Rejected.**
- **Closed source from day 1.** Eliminates community contributions, removes the OSS adoption ramp, and forfeits the indirect marketing value of public development. **Rejected.**

## Implementation Notes

- License posture tracker: [`vault/70_References/Third-Party Dependencies.md`](../70_References/Third-Party%20Dependencies.md) — every third-party dep gets a row with license / version / linking model / posture. This is the operational artifact CLAUDE rule 9 enforces.
- Examples of the posture rules in shipped code:
  - **LGPL via dynamic link**: GTK4 (`pkg-config gtk4` resolves at link time on Linux), libcairo (LGPL-2.1, dynamic via `pkg_check_modules` — see [`CMakeLists.txt`](../../CMakeLists.txt) graphics-backend section).
  - **Permissive via static link**: Skia BSD-3 (static `.a` / `.lib` linked into mpapp-core when `MPAPP_GRAPHICS_BACKEND=skia` — see [`cmake/MpappFindSkia.cmake`](../../cmake/MpappFindSkia.cmake)).
  - **Permissive build-time only**: Zig (cross-compile toolchain — see [`cmake/toolchains/zig.cmake`](../../cmake/toolchains/zig.cmake)).
- No GPL / strong-copyleft runtime deps in the tree today. Verified by inspecting the dependency tracker; no port has been added without a matching row.

## References

- [[RFC-0001-licensing-and-patent-strategy]] — source RFC closed by this ADR
- [[70_References/Third-Party Dependencies]] — per-dependency license tracker
- [[CLAUDE]] — rule 9 (license vigilance)
- [Apache 2.0 license text](https://www.apache.org/licenses/LICENSE-2.0)
