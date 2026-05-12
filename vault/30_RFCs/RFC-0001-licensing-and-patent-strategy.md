---
type: rfc
id: RFC-0001
title: Licensing and patent strategy for MPAPP
status: draft
author: Alex Tsvetanov
created: 2026-05-12
area: legal
relatedADRs: [ADR-0010]
tags:
  - type/rfc
  - status/draft
  - area/legal
---

# RFC-0001 — Licensing and patent strategy for MPAPP

> [!info] Status
> **draft** — under discussion.

> [!note] Closed by
> Recommendations promoted to [[ADR-0010-licensing-and-patent-strategy]] (proposed 2026-05-12). This RFC stays `draft` until ADR-0010 is accepted.

## Problem

MPAPP is intended for **eventual commercialization** and **possibly patent filing**. Decisions made early about licensing, contributor agreements, and dependency choices will determine whether those options remain available later.

Specifically:

1. **Outbound license.** What license does MPAPP itself ship under?
2. **Contributor agreement.** How do we collect copyright/patent grants from contributors so we can later sublicense commercially?
3. **Dependency licenses.** Which third-party libraries are safe? GTK4 is LGPL — what does that mean for us?
4. **Patent strategy.** What's novel enough to file? What's prior art? When is the right time?

## Proposal

### 1. Outbound license — dual license

- **Open-source track:** Apache 2.0. Strong patent grant (defensive), permissive enough for adoption, explicit termination on patent litigation against the project. Industry-standard for serious infrastructure.
- **Commercial track:** custom enterprise license sold separately for users who want indemnification, priority support, custom builds, or who cannot accept Apache 2.0's notice requirements.
- Both tracks ship the same source. The license is a *grant* to the user; it does not affect what we ship.

### 2. Contributor License Agreement (CLA)

- Adopt the Apache Individual / Corporate CLA pattern (used by Apache Foundation, Kubernetes, Google, Meta projects).
- Every external contributor signs the CLA before their PR can be merged.
- The CLA grants us: (a) copyright license to use their contribution, (b) patent license, (c) the right to **re-license** under any future license. This is what unlocks the commercial track.
- Use a tool like [cla-assistant.io](https://cla-assistant.io) to automate signature collection on PRs.

### 3. Dependency licenses — vetting policy

Each third-party dependency must be recorded in [[70_References/Third-Party Dependencies]] with: license, version, transitive deps, linking model (static / dynamic), and a posture note.

| License class | Examples | Posture |
|---|---|---|
| **Permissive** (Apache 2.0, MIT, BSD) | fbjni, C++/WinRT, Catch2 | ✅ Use freely |
| **Weak copyleft** (LGPL, MPL) | GTK4, glib | ⚠️ Dynamic linking only. Document license. Provide rebuild instructions per LGPL. |
| **Strong copyleft** (GPL) | — | ❌ Forbidden as a runtime dependency |
| **Source-available / commercial** | Live++, Qt commercial | ⚠️ Forbidden unless re-licensed or replaced |

For GTK4 specifically: dynamic linking from MPAPP-licensed code to LGPL GTK is permitted by LGPL section 4. Users get a notice + offer to provide rebuildable LGPL portions.

### 4. Patent strategy

**Prior-art audit (before any filing).** Compare MPAPP's novel claims against:

- .NET MAUI (Microsoft)
- Qt (TQT Group / KDAB / Qt Co.)
- Slint (Slint GmbH)
- JUCE (Raw Material Software / PACE)
- Avalonia (AvaloniaUI OÜ)
- Flutter (Google)
- React Native (Meta)
- Dear ImGui (Omar Cornut)

**Candidate novel claims** for patent (to evaluate, not yet filed):

- The `Computed<&member, &member>` template-parameter dependency-tracking pattern.
- Zero-cost link-time selection of native event-loop integrations via CMake.
- The XAML-to-`consteval`-C++ compilation pipeline with source-mapped diagnostics.
- Mock-first cross-platform handler validation pattern.

For each candidate, document: state of the art, our novelty, business value, freedom-to-operate concern. Filing decisions wait until P3 or later — we need the code to ship first.

## Detailed Design

(To be filled as items above are decided.)

## Alternatives

- **MIT-only.** Simpler but no explicit patent grant; harder commercial story.
- **GPL.** Maximally protective but kills enterprise adoption.
- **Closed source from day 1.** Kills community adoption and external contributors.

## Open Questions

> [!todo] Open
> - [ ] Confirm dual-license is compatible with the CLA pattern
> - [ ] Pick a CLA assistant (cla-assistant.io vs Easy CLA vs hand-rolled)
> - [ ] Decide on default copyright assignee — Alex Tsvetanov (individual), or a future LLC?
> - [ ] Decide whether GTK4 dependency is acceptable given LGPL constraints, or whether to replace with Qt (commercial) / pure custom
> - [ ] Patent: jurisdiction (US? EU? PCT?), attorney engagement timing

## Migration / Compatibility

N/A — this is initial licensing, no prior state.

## References

- [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0)
- [GNU LGPL v3](https://www.gnu.org/licenses/lgpl-3.0.html)
- [Apache CLA template](https://www.apache.org/licenses/contributor-agreements.html)
- [[70_References/Third-Party Dependencies]]
