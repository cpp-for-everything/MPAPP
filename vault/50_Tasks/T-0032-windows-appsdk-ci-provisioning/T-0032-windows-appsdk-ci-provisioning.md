---
type: task
id: T-0032
title: Provision WindowsAppSDK on the cloud Windows runner so windows-native can rejoin pr.yml
status: todo
milestone: M-04
owner: ""
area: build
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/build
  - area/tooling
  - phase/p4
  - platform/windows
---

# T-0032 — Provision WindowsAppSDK on the cloud Windows runner

## Goal

Make the cloud `windows-latest` runner able to build `mpapp-core` (and therefore everything that depends on it) without the project lead having to install WindowsAppSDK by hand. Result: the `windows-native` cloud job currently removed from [`pr.yml`](../../../.github/workflows/pr.yml) (see [[CI Strategy]] §*Windows on the cloud runner is deferred*) can be reinstated as a green per-PR check.

## Background

[[ADR-0024-wrapper-component-pattern]] changed every component wrapper to **embed** the platform handler by value. Consequently:

- Any TU that `#include <mpapp/<name>.hpp>` instantiates the wrapper's ctor → which calls the embedded `internal::<name>_handler<platform::current>` member's ctor → which on Windows means a `winrt::Microsoft::UI::Xaml::Controls::Button{}` constructor call → which means the WinRT headers must be reachable.
- `mpapp-core/src/mpapp.cpp` `#includes <mpapp/mpapp.hpp>` (the umbrella header), which pulls in every wrapper, which pulls in every per-platform handler header transitively.

On the project lead's local Windows machine, WindowsAppSDK is installed via NuGet using the project's [`cmake/WindowsAppSDK.cmake`](../../../cmake/WindowsAppSDK.cmake) helper, so `winrt/Microsoft.UI.Xaml.Controls.h` is on the compiler's include path and the build succeeds. The `windows-latest` GitHub-hosted runner ships Visual Studio 2022 with C++/WinRT but **does not** ship WindowsAppSDK, so the build fails at `error C1083: Cannot open include file: 'winrt/Microsoft.UI.Xaml.Controls.h'` while compiling `mpapp-core.vcxproj`.

The cloud Windows job was deleted from `pr.yml` in commit `e45d969` to stop burning runner minutes on the known failure. This task captures the work needed to bring it back.

## Two viable paths

### Path A — Provision WindowsAppSDK in CI

1. Add a workflow step that invokes `mpapp_install_windows_app_sdk()` (or its equivalent shell-script form) before the `cmake -S . -B build` step. The helper today only runs when an example target uses it; refactor to either:
   - expose a `cmake -P` entry point (`cmake -P cmake/install-windows-app-sdk.cmake`), or
   - add a CMake option `-DMPAPP_PROVISION_WINDOWSAPPSDK=ON` that triggers the install at configure time even when no example target is being built.
2. Auto-add the WinRT projection includes to `mpapp-core` when WindowsAppSDK is detected (currently only the example targets call `mpapp_add_winappsdk_runtime`).
3. GitHub-Actions cache the `${BUILD_DIR}/packages/Microsoft.WindowsAppSDK.<ver>` tree keyed on the NuGet version pin. Cache size ~250 MB compressed; cold restore is ~3 min.
4. Reinstate the `windows-native` job in `pr.yml` with `timeout-minutes: 25`, expect ~12 min cold + ~6 min cache-warm wall clock.

### Path B — Decouple `mpapp-core` from the umbrella

1. Make `src/mpapp.cpp` a trivial TU (`// placeholder — see ADR-0024`); drop the `#include <mpapp/mpapp.hpp>` line. The static library still has at least one object file, but it no longer pulls in any wrapper.
2. Audit the other umbrella consumers (`tests/smoke_test.cpp`, `tests/template_type_spike/test.cpp`, `tools/mpapp-xc/src/emitter.cpp`, `tools/mpapp-xc/tests/fixtures/empty_expected.gen.hpp`) and either:
   - replace umbrella include with explicit per-component surface includes (`#include <mpapp/internal/basic_button.hpp>`), or
   - guard the umbrella's wrapper section behind a `MPAPP_SURFACE_ONLY` macro the test TUs define before including.
3. Once `mpapp-core` compiles standalone on Windows without WindowsAppSDK, reinstate the `windows-native` job at the smaller scope: just the core library + the surface-only test TUs. Tests that genuinely exercise WinUI 3 stay opt-in (require local Windows or the self-hosted runner).
4. Wall-clock target: same ~10 min cold ceiling as the original `windows-native` job, no NuGet restore.

## Recommended path

**Path B first, Path A second.** Path B fixes a real architectural smell (mpapp-core dragging in platform headers) and is what the comment block at the top of `CMakeLists.txt` (lines 55-56: "UI handlers ... are NOT compiled into mpapp-core because they pull in heavy native SDKs") already promises. Path A is a workaround that keeps the smell.

The two paths are not mutually exclusive — Path A gives full test coverage; Path B is the right starting point.

## Acceptance Criteria

- [ ] `mpapp-core` builds on `windows-latest` without WindowsAppSDK installed (Path B exit gate).
- [ ] `windows-native` job in `pr.yml` is green for at least 3 consecutive merges to `main`.
- [ ] CI Strategy doc updated: the "Windows on the cloud runner is deferred" subsection moves from §Active to §History (or is deleted).
- [ ] If Path A is also implemented: the NuGet cache lands a hit on the second run; cold restore time measured + recorded under `logs/`.
- [ ] If Path A is implemented: `tests/smoke_test.cpp` and `tests/template_type_spike/test.cpp` execute green under MSVC in CI.
- [ ] Per CLAUDE Rule 11: `coveragePercent: 100` on any new CMake / workflow code introduced; this is build-side work so screenshots / recordings are N/A.

## Risks

> [!warning]
> - Path B may surface latent dependencies on the umbrella header that aren't grep-able. Compile the affected TUs early to catch them.
> - Path A's NuGet restore is brittle when NuGet.org has an outage; mirror the cache via the self-hosted runner if it becomes a problem.
> - Even after Path B, **user code** that `#include <mpapp/mpapp.hpp>` still needs WindowsAppSDK on Windows. That is correct — apps need a real WinUI 3 install. The fix here is only for the framework's own build, not for downstream consumers.

## Notes

The CI rewrite that documented this gap is commit `e45d969`. The same commit's update to [[CI Strategy]] §Windows on the cloud runner is deferred describes the user-visible symptom + reinstate triggers, in case future contributors hit the same wall.

## Links

- Blocker for: cloud-runner Windows validation in [[CI Strategy]].
- Related: [[ADR-0024-wrapper-component-pattern]] (the cause), [[ADR-0008-mock-first-implementation]] (the link-isolation contract the wrapper preserves on the test side), [[Build System]], `cmake/WindowsAppSDK.cmake`.
- Milestone home: [[M-04-Windows-Real]] (the Windows-real milestone) — this is operational rather than feature work, but lives there because it gates Windows CI signal.
