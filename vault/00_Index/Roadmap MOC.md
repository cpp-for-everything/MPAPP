---
type: moc
tags:
  - type/moc
---

# Roadmap MOC

Project phases, in order. **No time estimates** (CLAUDE rule 3) — just sequence and exit criteria. The authoritative live view is [[_Bases/Roadmap.base]] and the visual is [[_Canvases/Phase-Roadmap.canvas]].

## Sequential through P5, parallel from P6

| Phase | Milestone | Status |
|---|---|---|
| P0 | [[M-01-Foundations]] | active |
| P1 | [[M-02-Infrastructure]] | planned |
| P2 | [[M-03-Mock-Surface]] | planned |
| P3 | [[M-04-Windows-Real]] | planned |
| P4 | [[M-05-Android-Real]] | planned |
| P5 | [[M-06-Linux-Real]] | planned |
| P6 | [[M-07-macOS-Real]] | planned |
| P7 | [[M-08-iOS-Real]] | planned |
| P8 | [[M-09-Tooling-DX]] | planned |
| P9 | [[M-10-Ecosystem]] | planned |

## Phase narrative

**P0 → P1**: research and decisions before infrastructure.

**P1 → P2**: infrastructure is the substrate; mocks come once we can build and test cross-platform.

**P2**: the critical phase where the **entire** public API surface materializes as mocks. The user's strategic choice — design pressure from real platforms can't shape the API until it's locked.

**P3 → P5**: Windows → Android → Linux. Windows first because we're on Windows; Android second because it has fundamentally different interop (JNI); Linux third because GTK4 from WSLg validates the dev loop.

**P6 → P7**: macOS → iOS. Gated on user providing MacBook Pro. The human-free test harness from P1 ([[T-0008-mac-ios-test-harness-design]]) is the precondition.

**P8**: developer experience — VS Code extension, hot reload polish, Visual Studio integration.

**P9**: ecosystem — vcpkg / Conan, docs site, MAUI migration guide, finalized licensing.

## Visuals

- [[_Canvases/Phase-Roadmap.canvas]] — swimlane view.
- [[_Canvases/Interop-Parity-Matrix.canvas]] — component × platform progress.
- [[_Canvases/Cross-Compilation-Matrix.canvas]] — host × target build feasibility.
