---
type: log
week: 2026-W19
date: 2026-05-12
tags:
  - type/log
---

# 2026-W19 — Weekly log

## What happened

- **Vault created.** `D:\GitHub\MPAPP\vault\` set up with full folder structure, templates, bases, canvases, glossary, and seed content.
- **9 ADRs accepted** in one session — see [[Decision Log]] for the chronological list.
- **2 RFCs opened** — licensing/patent strategy and cross-compilation toolchain.
- **MAUI inventoried** — 56 components stubbed in `10_Architecture/Components/` from MAUI source.
- **M-01 Foundations** activated as the current milestone.
- **10 tasks created** (T-0001 through T-0010), each as a folder with `screenshots/ recordings/ logs/ tests/ notes/` subfolders.
- **`maui.md` moved** to `vault/60_Research/dotnet-maui-deep-dive.md`.

## What's next

Active task list: [[_Bases/Tasks.base]].

Top three for next week:

1. [[T-0005-inventory-maui-controls]] — flesh out the 56 component stubs with MAUI-derived content.
2. [[T-0002-template-type-spike]] — prove the `Observable<T>` / `Computed<...>` / `Command<>` design.
3. Close [[RFC-0002-cross-compilation-toolchain]] — pick Zig or LLVM+sysroots.

## Notes

- All architectural decisions concentrated on day 1 deliberately, so the project starts with a clear contract.
- The user is operating on Windows 11 Pro N; WSLg available for Linux development from the same host (see [[70_References/WSLg]]).
- The user's MacBook Pro will be the macOS / iOS self-hosted runner later (M-07 onwards) — the human-free UI test harness ([[T-0008-mac-ios-test-harness-design]]) must be designed before then.

## Related

- [[00_Index/Current Focus]]
- [[M-01-Foundations]]
- [[Decision Log]]
