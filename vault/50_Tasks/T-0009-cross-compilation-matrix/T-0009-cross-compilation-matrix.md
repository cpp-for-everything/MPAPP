---
type: task
id: T-0009
title: Validate host × target cross-compilation matrix
status: todo
milestone: M-02
owner: ""
area: build
blockedBy:
  - T-0001
  - T-0006
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/build
  - phase/p1
---

# T-0009 — Cross-compilation matrix validation

## Goal

Validate every cell of the cross-compilation matrix from [[Build System]] §Cross-compilation matrix. For each host × target combination, produce a smoke binary and verify it runs (or, for Apple-unsigned-cross, verify it loads with `xcrun otool -L`).

## Acceptance Criteria

For each of the following 15 host × target combinations, produce evidence in `logs/`:

- [ ] Windows host → Windows-x64: native build, runs ✓
- [ ] Windows host → Linux-x64: cross via Zig/Clang, runs on WSL ✓
- [ ] Windows host → Android-arm64: cross via NDK/Zig, loads on emulator ✓
- [ ] Windows host → macOS-arm64: cross via osxcross, unsigned ⚠️
- [ ] Windows host → iOS-arm64: cross via osxcross, unsigned ⚠️
- [ ] Linux host → Windows-x64: cross via MinGW/Zig, runs in WSL→Windows or Wine ✓
- [ ] Linux host → Linux-x64: native ✓
- [ ] Linux host → Android-arm64: cross via NDK ✓
- [ ] Linux host → macOS-arm64: cross, unsigned ⚠️
- [ ] Linux host → iOS-arm64: cross, unsigned ⚠️
- [ ] (macOS host runs deferred until the MacBook self-hosted runner is online.)

- [ ] Documentation in `notes/matrix-status.md` summarizing the matrix with green/yellow/red status per cell.
- [ ] Build logs in `logs/` for each cell.
- [ ] 100% coverage on `cmake/toolchains/*.cmake` (each toolchain file is exercised at least once).

## Notes

This task closes RFC-0002 — once the matrix passes, the cross-compilation toolchain decision is empirically validated.

## Links

- Milestone: [[M-02-Infrastructure]]
- Related: [[Build System]], [[RFC-0002-cross-compilation-toolchain]], [[70_References/Zig]], [[70_References/osxcross]]
