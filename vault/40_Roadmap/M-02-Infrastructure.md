---
type: milestone
id: M-02
title: Infrastructure — CMake, CI, cross-compilation, test harness
phase: P1
status: planned
deliverables:
  - CMake 3.28+ skeleton
  - mpapp CLI bootstrap
  - All five cross-compilation toolchain files
  - GitHub Actions matrix with budget-aware sharding
  - Self-hosted Windows runner for Android emulator runs
  - Catch2 or GoogleTest harness wired in
  - mpapp-xc XAML compiler skeleton
  - WSLg-based Linux GTK4 hello-window working from the Windows dev host
exitCriteria:
  - "'hello world' CMake target cross-builds to Windows, Linux, Android from the Windows host"
  - CI green on PRs
  - "mpapp build --target <name> works for all five targets (Apple unsigned-only is acceptable here)"
tags:
  - type/milestone
  - phase/p1
  - status/planned
---

# M-02 — Infrastructure

> [!info] Status
> **planned**. Starts after [[M-01-Foundations]] closes.

## Scope

Build the substrate everything else sits on: CMake configuration, the `mpapp` CLI, per-platform toolchain files for cross-compilation, the test harness, and the CI matrix that validates it all. No application code yet — just the scaffolding.

## Exit Criteria

- [ ] CMake skeleton: `mpapp-core` and `mpapp-xc` targets exist, build, and pass minimal smoke tests.
- [ ] `mpapp` CLI bootstrap: `mpapp new`, `mpapp build`, `mpapp build --target <t>` all work.
- [ ] Cross-compilation toolchain files exist for all 10 host×target combinations (with Apple unsigned-only acceptable for non-Mac hosts).
- [ ] GitHub Actions: windows-native, windows-cross, linux-native, linux-cross all green on a no-op PR.
- [ ] Self-hosted runner registered on the user's Windows machine.
- [ ] Catch2 (or GoogleTest, per sub-RFC) integrated; sample test passes.
- [ ] `mpapp-xc` parses an empty `MainPage.xaml` and emits a valid `consteval` C++ stub.
- [ ] WSLg GTK4 hello-window from the user's Windows host (T-0007).

## Risks

> [!warning]
> - CMake single-project multi-targeting is itself a multi-month effort. Budget accordingly.
> - Self-hosted runner reliability — document recovery procedures.
> - Cross-compilation sysroot management is brittle; pin versions aggressively.

## Tasks

- [[T-0001-cmake-skeleton]]
- [[T-0006-ci-skeleton]]
- [[T-0007-wslg-gtk4-hello]]
- [[T-0009-cross-compilation-matrix]]

## Related

- [[ADR-0007-cross-platform-tooling]]
- [[RFC-0002-cross-compilation-toolchain]]
- [[Build System]]
- [[CI Strategy]]
- [[Test Harness]]
