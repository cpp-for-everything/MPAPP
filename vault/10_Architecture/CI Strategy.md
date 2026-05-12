---
type: moc
area: tooling
tags:
  - area/tooling
  - area/build
---

# CI Strategy

GitHub Actions minutes are finite. Per CLAUDE rule 8, every matrix axis must be justified. This note describes the budget-aware CI design.

## Goals

1. Validate every PR against the conformance test suite on all supported platforms.
2. Catch interop-parity regressions early (per [[ADR-0006-interop-parity]]).
3. Stay within free-tier or sponsored Action minutes.
4. Make Apple-platform builds work without a paid macOS runner until the user's MacBook comes online (then it becomes a self-hosted runner).

## Job matrix

| Job | Runs on | Targets built | Trigger |
|---|---|---|---|
| `windows-native` | `windows-latest` | windows-x64 | Every PR |
| `windows-cross` | `windows-latest` | linux-x64, android-arm64 | Every PR |
| `linux-native` | `ubuntu-latest` | linux-x64 | Every PR |
| `linux-cross` | `ubuntu-latest` | windows-x64, android-arm64 | Every PR |
| `android-emulator` | self-hosted (user's Windows machine) | android-arm64 + Android emulator run | Every PR — async, status-checked at merge time |
| `macos-native` | `macos-latest` | macos-arm64, ios-arm64 (Simulator) | Tagged releases only initially; every PR once self-hosted macOS runner comes online |
| `wslg-gtk4-smoke` | self-hosted (user's Windows machine with WSL2) | linux-x64 GUI smoke test | Daily, not per-PR |

## Sharding patterns

- **No full matrix on every PR.** Even within "every PR" jobs, we shard test suites by component group:
  - Group A: layout primitives + simple controls (Button, Label, Entry, Switch, Slider, …).
  - Group B: collection controls (CollectionView, ListView, TableView).
  - Group C: navigation (NavigationPage, FlyoutPage, TabbedPage, Shell).
  - Group D: complex controls (Editor, WebView, GraphicsView, HybridWebView, SwipeView).
  - Group E: platform-specific superset features.
- Each group runs in parallel on the same runner; total wall-clock for a PR build is the slowest group.

## Toolchain

Per [[ADR-0011-cross-compilation-toolchain]], all cross-compilation in CI uses **Zig (`zig cc`)**. Cross jobs (`windows-cross`, `linux-cross`) invoke the toolchain files in `cmake/toolchains/` that wrap `zig cc --target=<triple>`. Native jobs continue to use the host's default compiler (MSVC on Windows runners, system Clang on Ubuntu runners, Xcode Clang on macOS runners). The Zig version is pinned in `cmake/toolchains/zig.cmake`; runners auto-install it via the `mpapp` CLI on first use and cache it between runs.

## Caching

- **ccache** for C++ compilation, keyed on toolchain + flags + source hashes.
- **GitHub Actions cache** for the pinned Zig toolchain (per [[ADR-0011-cross-compilation-toolchain]]), and any per-target Apple SDK pieces (osxcross) used by cross-build jobs. The Android NDK is no longer cached separately since Zig bundles the Android cross-target.
- **Build outputs** between matrix jobs (Windows-native shares its CMake configure cache with Windows-cross).

## Self-hosted runners

Two now, more later:

| Runner | Hosted on | Purpose |
|---|---|---|
| `mpapp-windows-self` | User's Windows machine | Android emulator runs, WSLg smoke tests |
| `mpapp-macos-self` (future) | User's MacBook Pro | All Apple-platform builds + Simulator runs |

Self-hosted runners are tagged in workflows so cloud runners only handle the budget-light jobs.

## Conformance gates

A PR is mergeable when:

- All `every PR` jobs are green.
- Conformance test results match on Windows, Android emulator, and (once available) Linux + Apple platforms.
- Per CLAUDE rule 11: any task being closed in this PR has 100% coverage + screenshots/recordings.

## Apple platform timeline

Until the MacBook self-hosted runner comes online:

- Apple targets compile via osxcross on Linux runners (unsigned).
- macOS-hosted CI runs only on tagged releases.
- iOS Simulator runs are scheduled, not blocking.

Once the runner is online:

- Apple becomes a per-PR target.
- The automated UI test harness designed in P1 (T-0008) runs without human intervention.

## Budget tracking

Estimated minutes (target: free-tier 2000/mo with overflow):

| Job | Avg minutes/run | Runs/mo | Subtotal |
|---|---|---|---|
| windows-* (2 jobs) | 8 each | ~120 PRs | 1920 |
| linux-* (2 jobs) | 5 each | ~120 PRs | 1200 |
| macos-* (release only) | 20 | ~4 tags | 80 |
| **Self-hosted (Android, WSL)** | — | — | not counted |

→ Total cloud minutes: ~3200/mo. Plan to reduce via aggressive caching + shard skipping (no-op PRs that touch only docs skip C++ jobs entirely).

## See also

- [[ADR-0008-mock-first-implementation]]
- [[ADR-0011-cross-compilation-toolchain]]
- [[Test Harness]]
- [[Build System]]
- [[CLAUDE]] rule 8
