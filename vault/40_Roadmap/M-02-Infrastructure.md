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

## See in code

- Root CMake project + `mpapp-core` target: [`CMakeLists.txt`](../../CMakeLists.txt).
- Cross-compilation toolchain files (one per target): [`cmake/toolchains/`](../../cmake/toolchains/) — windows-x64 / linux-{x64,arm64} / android-arm64 / macos-arm64 / ios-arm64 / zig (the cross-compiler frontend per [[ADR-0011-cross-compilation-toolchain]]).
- Developer CLI bootstrap: [`tools/mpapp/`](../../tools/mpapp/).
- XAML compiler skeleton: [`tools/mpapp-xc/`](../../tools/mpapp-xc/).
- Test harness: [`tests/CMakeLists.txt`](../../tests/CMakeLists.txt) (Catch2 via FetchContent + `CONFIGURE_DEPENDS` glob for `tests/mock_handlers/*_test.cpp`) + [`tests/smoke_test.cpp`](../../tests/smoke_test.cpp) + [`tests/executor_test.cpp`](../../tests/executor_test.cpp).
- CI workflows: [`.github/workflows/`](../../.github/workflows/) — `pr.yml` (per-PR matrix) + `release.yml` (tagged-release full matrix) + `build-skia-md-windows.yml` (Skia /MD prebuilt publisher).
- T-0009 cross-compile matrix validation: [`vault/50_Tasks/T-0009-cross-compilation-matrix/`](../50_Tasks/T-0009-cross-compilation-matrix/) — still `in-progress` (Apple-host rows pending).

## Related

- [[ADR-0007-cross-platform-tooling]]
- [[RFC-0002-cross-compilation-toolchain]]
- [[Build System]]
- [[CI Strategy]]
- [[Test Harness]]
