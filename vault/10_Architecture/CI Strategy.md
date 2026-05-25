---
type: moc
area: tooling
tags:
  - area/tooling
  - area/build
---

# CI Strategy

GitHub Actions cloud minutes are unlimited on this public repo, but minimal **wall-clock time** and minimal **self-hosted-runner wear** still matter per CLAUDE rule 8: every matrix axis must be justified. This note describes the budget-aware CI design as it stands today and the deferred axes waiting on their enabling work.

## Goals

1. Validate every code PR against the conformance test suite on Windows + Linux (the two host platforms that have real native handlers wired up today).
2. Don't run C++ builds at all on PRs that only touch documentation.
3. Cancel old in-flight runs the moment a newer commit lands on the same branch.
4. Catch interop-parity regressions early (per [[ADR-0006-interop-parity]]).
5. Don't burn runner minutes on placeholder jobs (jobs whose every step is `continue-on-error` are not running anything — delete them, reinstate when their enabling toolchain lands).

## Active job matrix

What runs today, per workflow.

### `pr.yml` (every code PR + every push to `main`)

| Job | Runs on | What it does | Trigger condition |
|---|---|---|---|
| `linux-native` | `ubuntu-latest` | Configure / build / ctest the full project on Ubuntu 24.04 with apt-installed GTK4 + Cairo + WebKitGTK. Examples + tools enabled. Canonical validator. | Any PR that touches non-docs paths; push to main. |
| `android-emulator` | self-hosted `[mpapp-windows-self]` | Conditional Android cross-build + emulator smoke (no-op until [[T-0009]]'s toolchain files land). | Only for in-repo PRs (fork PRs skipped — security boundary), non-blocking. |

#### Windows on the cloud runner is deferred

A `windows-native` cloud job lived in `pr.yml` previously; it has been **removed**. After [[ADR-0024-wrapper-component-pattern]] the wrapper layer embeds the platform handler by value, so building any TU that pulls in `mpapp.hpp` (the umbrella) on Windows requires the WinUI 3 / WindowsAppSDK headers (`winrt/Microsoft.UI.Xaml.Controls.h` etc.) to be reachable. `mpapp-core/src/mpapp.cpp` does pull the umbrella in, and the `windows-latest` GitHub runner does not ship WindowsAppSDK — the project's [`cmake/WindowsAppSDK.cmake`](../../cmake/WindowsAppSDK.cmake) helper is only auto-invoked from example targets, not from `mpapp-core`.

Per-PR Windows validation in the meantime comes from:

- Local builds on the project lead's Windows machine (where WindowsAppSDK is installed).
- The self-hosted `mpapp-windows-self` runner (slot reserved for a future `windows-self` job in the android-emulator family — WindowsAppSDK is already provisioned there).

Reinstate the cloud Windows job when either:

1. `mpapp-core` is decoupled from the umbrella (`src/mpapp.cpp` becomes a trivial TU that doesn't `#include <mpapp/mpapp.hpp>`, and the few tests / tools that do are made conditional on a `MPAPP_HAS_PLATFORM_HANDLERS` flag), OR
2. The Windows-CI step gains a `mpapp_install_windows_app_sdk()` invocation + NuGet cache.

Until then the cloud Windows minutes are wasted on a known-failing job, and `linux-native` is the canonical per-PR gate.

### `release.yml` (push to `v*` tag)

| Job | Runs on | What it does |
|---|---|---|
| `linux-native` | `ubuntu-latest` | Same shape as the PR job. |
| `windows-native` | `windows-latest` | Currently expected to fail with the WinUI 3 header gap described above — left in place so a future "windows in CI" enabler PR has an obvious target to fix. Tag builds are gated by humans, so a red Windows job here is informational, not blocking merge of code into `main`. |
| `macos-native` | `macos-latest` | Configure / build / ctest macos-arm64 (Xcode Clang), then configure / build ios-arm64 Simulator. Examples disabled until [[M-07-macOS-Real]] / [[M-08-iOS-Real]] complete the Apple handler set. |

## Deferred axes

These were in earlier versions of the workflows but have been **removed** because they cost runner time without validating anything. Reinstate once the enabling work lands.

| Job | Reinstate when |
|---|---|
| `windows-cross` (Zig → linux-x64 + android-arm64) | [[T-0009]] lands `cmake/toolchains/{linux-x64,android-arm64}.cmake`. Until then every cmake/build step had `continue-on-error: true` and ran no-op. |
| `linux-cross` (Zig → windows-x64 + android-arm64) | Same. |
| `wslg-gtk4-smoke` (self-hosted Windows + WSL) | [[T-0010]] (or wherever the WSLg smoke runner is provisioned). |

## Skip rules

### Docs-only PRs skip C++ builds entirely

Both `pr.yml` workflows declare:

```yaml
on:
  pull_request:
    paths-ignore: ['vault/**', '**/*.md', '.gitignore', 'LICENSE', 'tools/dev/**']
  push:
    branches: [main]
    paths-ignore: ['vault/**', '**/*.md', '.gitignore', 'LICENSE', 'tools/dev/**']
```

`tools/dev/**` is in the ignore list because the migration scripts under `tools/dev/` (e.g. `migrate-component.py`, `sweep-component-docs.py`) are tested out-of-band — they touch generated headers, the resulting headers are what CI validates, not the scripts.

### Concurrency cancel on new pushes

```yaml
concurrency:
  group: pr-${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```

A force-push or a rapid two-commit push to the same PR branch cancels the in-flight run.

## Submodule discipline

The `references/maui*` and `references/dotnet-community-toolkit` submodules are research-only — `.NET MAUI` source-of-truth lookups per [[CLAUDE]] Rule 7. **None of them are on a C++ build path** (`grep -rn "references/" --include=CMakeLists.txt` returns nothing). Every CI checkout therefore uses `submodules: false`:

```yaml
- uses: actions/checkout@v4
  with:
    submodules: false
    fetch-depth: 1
```

Why this matters:

- `references/maui-community-toolkit` contains paths > 260 chars that fail Windows checkout with `Filename too long` even on `core.longpaths=true`-enabled runners — every previous Windows-side build silently failed at the checkout step for this reason.
- The four reference submodules together clone multiple GB of C# source — pointless on a job that only builds C++.

## Toolchain

Per [[ADR-0011-cross-compilation-toolchain]], cross-compile jobs *will* use Zig (`zig cc`) once `cmake/toolchains/` ships the per-target files [[T-0009]] is tracking. The pinned Zig version lives in `cmake/toolchains/zig.cmake`; runners auto-install it via the `mpapp` CLI on first use and cache it between runs. Native jobs use the host's default compiler:

- Linux: system Clang (`apt install clang lld`)
- Windows: MSVC 2022 via the windows-latest image's installed VS
- macOS (release only): Xcode-bundled Clang

## Caching

| What | Where | Key |
|---|---|---|
| `ccache` / `sccache` compiler cache | per-platform | `<job-name>-${{ hashFiles('CMakeLists.txt', 'tests/CMakeLists.txt', 'cmake/**') }}`, max 200–300 MB |
| Per-build directory cache | (removed) | Earlier workflows cached `build/` keyed on CMakeLists hashes. ccache subsumes the value at lower restore cost and without invalidation foot-guns; the build-dir cache is gone. |
| Zig toolchain | (deferred with cross-compile jobs) | reinstate alongside the cross job |

## Self-hosted runners

| Runner | Hosted on | Purpose |
|---|---|---|
| `mpapp-windows-self` | User's Windows machine | Android emulator runs (today: deferred); future WSLg smoke tests |
| `mpapp-macos-self` (future) | User's MacBook Pro | All Apple-platform builds + Simulator runs once available |

Self-hosted runners are tagged so cloud runners only handle the budget-light jobs. The `android-emulator` job in `pr.yml` is conditional on `github.event.pull_request.head.repo.full_name == github.repository` — fork PRs are skipped entirely so an untrusted contributor can't execute code on the user's Windows machine.

## Conformance gates

A PR is mergeable when:

- All `every PR` cloud jobs are green (`linux-native` + `windows-native`).
- Per CLAUDE rule 11: any task being closed in this PR has 100% coverage + screenshots/recordings.
- The self-hosted `android-emulator` job is informational only (`continue-on-error: true`) until [[T-0009]] lands the toolchain — it does not block merge.

## Apple platform timeline

Until the MacBook self-hosted runner comes online:

- macOS-hosted CI runs only on tagged releases (in `release.yml`).
- iOS Simulator runs are scheduled, not blocking.

Once the runner is online:

- Apple becomes a per-PR target.
- The automated UI test harness designed in P1 ([[T-0008]]) runs without human intervention.

## Budget tracking

Public-repo Actions minutes are unlimited, but wall-clock matters for developer feedback latency. Approximate per-run targets at steady state:

| Job | Target wall-clock | Notes |
|---|---|---|
| `linux-native` (per PR) | ≤ 1 min cache-warm, ≤ 5 min cold | apt install → cached ccache → cmake configure (~30s) + parallel build (~1.5min) + parallel ctest (~30s). The canonical per-PR validator. |
| `windows-native` (release only) | currently fails | See § *Windows on the cloud runner is deferred*. Tag-only runs so the cost is occasional. |
| `macos-native` (release only) | ≤ 15 min | full macOS-arm64 + ios-arm64 build |
| Cross + Skia /MD prebuild | manual / out-of-band | not on the PR critical path |

A docs-only PR consumes **zero** runner minutes (paths-ignore short-circuits the whole workflow). The most recent runs show `linux-native` at ~54 s for an incremental cache-warm build, well inside the budget.

## See in code

- [`.github/workflows/pr.yml`](../../.github/workflows/pr.yml) — per-PR + push-to-main matrix (Linux + Windows + non-blocking Android self-hosted).
- [`.github/workflows/release.yml`](../../.github/workflows/release.yml) — tagged-release full matrix including macOS.
- [`.github/workflows/build-skia-md-windows.yml`](../../.github/workflows/build-skia-md-windows.yml) — `workflow_dispatch`-only Skia /MD prebuild for [[_Archive/T-0030-skia-backend]]'s auto-fetch path; never auto-runs.
- [`cmake/toolchains/`](../../cmake/toolchains/) — per-target toolchain files the deferred cross-build jobs will use: `windows-x64.cmake`, `linux-{x64,arm64}.cmake`, `android-arm64.cmake`, `macos-arm64.cmake`, `ios-arm64.cmake`. `zig.cmake` is the cross-compiler frontend.
- [`tools/mpapp/`](../../tools/mpapp/) — the developer CLI runners install on first use; pins the Zig version per [[ADR-0011-cross-compilation-toolchain]].

## See also

- [[ADR-0008-mock-first-implementation]]
- [[ADR-0011-cross-compilation-toolchain]]
- [[Test Harness]]
- [[Build System]]
- [[CLAUDE]] rule 8
