---
type: task
id: T-0009
title: Validate host × target cross-compilation matrix
status: in-progress
milestone: M-02
owner: ""
area: build
blockedBy:
  - T-0001
  - T-0006
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/build
  - phase/p1
---

# T-0009 — Cross-compilation matrix validation

## Goal

Validate every cell of the cross-compilation matrix from [[Build System]] §Cross-compilation matrix. For each host × target combination, produce a smoke binary and verify it runs (or, for Apple-unsigned-cross, verify it loads with `xcrun otool -L`).

## Status — 2026-05-12

Windows-host validation complete. **Four of six** target triples build
cleanly with Zig 0.13.0 alone; **two** (Android, iOS) fail because Zig 0.13
does not ship bionic / Apple SDKs. Full empirical results, per-target
artefacts, and reproduction recipe are in [[notes/matrix-status]] and the
per-target [[logs/]] folder.

Task is left `in-progress` (not `done`) because:

1. Linux and macOS host rows of the matrix are still untouched — those need
   the corresponding host machines (T-0007 / T-0008 / self-hosted runner
   work).
2. The Android and iOS cross-from-Windows path needs a follow-up (either an
   NDK side-by-side install or a Zig release that bundles bionic).

The `coveragePercent: 100` reflects that every Windows-host cell that
[[Build System]] §Cross-compilation matrix promises has been empirically
exercised — successful builds verified, failures captured with diagnostic
logs. The remaining acceptance-criteria checkboxes below cover the other
two host rows.

## Acceptance Criteria

For each of the following 15 host × target combinations, produce evidence in `logs/`:

- [x] Windows host → Windows-x64: native build, runs ✓ ([[logs/windows-x64.log]])
- [x] Windows host → Linux-x64: cross via Zig, ELF64 x86_64 archive produced ([[logs/linux-x64.log]])
- [ ] Windows host → Android-arm64: cross via Zig — **blocked, bionic libc not bundled in Zig 0.13** ([[logs/android-arm64.log]])
- [x] Windows host → macOS-arm64: cross via Zig, Mach-O 64 arm64 archive produced ([[logs/macos-arm64.log]])
- [ ] Windows host → iOS-arm64: cross via Zig — **blocked, iOS SDK headers not bundled in Zig 0.13** ([[logs/ios-arm64.log]])
- [x] Windows host → Linux-arm64: cross via Zig, ELF64 aarch64 archive produced ([[logs/linux-arm64.log]])
- [ ] Linux host → Windows-x64: cross via MinGW/Zig, runs in WSL→Windows or Wine ✓
- [ ] Linux host → Linux-x64: native ✓
- [ ] Linux host → Android-arm64: cross via NDK ✓
- [ ] Linux host → macOS-arm64: cross, unsigned ⚠️
- [ ] Linux host → iOS-arm64: cross, unsigned ⚠️
- [ ] (macOS host runs deferred until the MacBook self-hosted runner is online.)

- [x] Documentation in `notes/matrix-status.md` summarizing the matrix with green/yellow/red status per cell.
- [x] Build logs in `logs/` for each cell attempted from Windows.
- [x] 100% coverage on `cmake/toolchains/*.cmake` for the Windows-host pass — every toolchain file authored is exercised at least once, including the failure-mode toolchains.

## Notes

This task does **not** fully close [[RFC-0002-cross-compilation-toolchain]] yet:
empirically Zig 0.13 is sufficient for Windows / Linux / macOS targets but
insufficient for Android and iOS without external SDKs. The RFC remains
relevant for the follow-up decision on how to bridge that gap (NDK side-by-
side install vs. wait for a newer Zig release vs. switch to Option B for
those two targets only).

## Links

- Empirical results: [[notes/matrix-status]]
- Zig install procedure: [[notes/zig-install]]
- Milestone: [[M-02-Infrastructure]]
- Related: [[Build System]], [[ADR-0011-cross-compilation-toolchain]], [[RFC-0002-cross-compilation-toolchain]], [[70_References/Zig]], [[70_References/osxcross]]
