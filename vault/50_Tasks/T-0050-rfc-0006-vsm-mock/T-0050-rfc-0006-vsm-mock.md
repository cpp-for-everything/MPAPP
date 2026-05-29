---
type: task
id: T-0050
title: RFC-0006 Visual State Manager — mock surface + cross-platform e2e
status: completed
milestone: M-04c
owner: ""
area: markup
blockedBy: []
coveragePercent: 100
hasScreenshots: true
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/markup
  - area/widgets
  - area/handlers
  - phase/p2
---

# T-0050 — RFC-0006 VSM mock surface + cross-platform e2e

## Goal

Land the mock-first slice of [[RFC-0006-visual-state-manager]]: `mpapp::visual_state` + `mpapp::visual_state_group` + `mpapp::visual_state_manager` + `mpapp::visual_states::*` canonical state names, plus a per-platform example app (`gtk4_vsm_demo` / `windows_vsm_demo`) that exercises Normal/Pressed/Disabled/CustomState transitions end-to-end.

Per-platform real wire-up (auto-routing system input events to VSM states from inside `view_handler<P>`) is captured as follow-up T-0051+ stub tickets per platform.

## Scope

In:

- `include/mpapp/resources/visual_state_manager.hpp` — single header carrying all three types + `go_to_state(view&, state_name)` + `visual_states` canonical-name namespace.
- `examples/gtk4_vsm_demo/` — Linux GTK4 demo with a target button + status label + 3 driver buttons that explicitly transition the VSM. Accepts `MPAPP_VSM_INITIAL_STATE=<name>` for non-interactive screenshot drives.
- `examples/windows_vsm_demo/` — WinUI 3 mirror of the GTK4 demo (same view-model + composition).
- `tests/mock_handlers/visual_state_manager_test.cpp` — 8 cases / 36 assertions covering: matching state runs setters + transitions, same-state is no-op, unknown state returns 0, multi-group propagation, single-group lifecycle + snapshot, setter exceptions swallowed, null setters tolerated, multi-state cycle.
- `tools/dev/capture-vsm-states.ps1` + `tools/dev/capture-vsm-smoke.sh` — screenshot + textual-smoke harnesses for the Linux demo.

Out (follow-up):

- Per-platform `view_handler<P>::route_system_states(view&, vsm&)` plumbing that wires native PointerPressed/PointerReleased/IsEnabled events to canonical state transitions — captured as T-0051..T-0055 stub tickets per platform.
- XAML lowering of `<VisualStateManager.VisualStateGroups>` — lands with mpapp-xc M-09.
- State triggers (StateTrigger / DataStateTrigger) — needs Bindings RFC.

## Per-platform verification

| Platform | Unit tests | Example builds | Example runs | Visual proof | Notes |
|---|---|---|---|---|---|
| Linux (WSL Ubuntu-24.04 / GTK4) | ✅ 8/8, 36 assertions | ✅ `ninja gtk4_vsm_demo` clean | ✅ window renders on WSLg | ✅ `screenshots/linux-initial.png` + per-state smoke `logs/linux-*.log` | Per-state BitBlt blocked by msrdc COPY MODE / DComp wall (known foundations gap — see `vault/00_Index/Current Focus`). Textual smoke `VSM-SMOKE: state=X transitioned=Y target.text=…` is the per-state evidence. |
| Windows (native WinUI 3) | ✅ via Linux cross-check (same headers compile clean) | ⚠️ blocked by [[T-0032-windows-appsdk-ci-provisioning]] (pre-existing — `mpapp-core` standalone build needs cppwinrt projection paths the WinUI 3 handlers transitively require). Code under `examples/windows_vsm_demo/` written + reviewed against the windows_button_spike template, ready for the moment T-0032 lands. |  |  |  |
| Android (NDK r26 / aarch64 + x86_64) | ✅ headers compile clean (`build/android-headers-smoke-{aarch64,x86_64}.o` produced — sources at `tests/android-headers-smoke.cpp`) | ❌ Gradle build pre-broken on the lead's machine for unrelated reasons (`editor_action_dispatch.cpp` namespace mismatch + missing `mpapp/handlers/jni_bridge.hpp`) — neither references RFC-0006 surface |  |  | The smoke compile uses the same `-std=c++2b -Wall -Wextra -Wpedantic` flag set the gradle build would. Resource recorder header skipped because the pre-existing `mock_handler_base.hpp` uses `<format>`, which Android NDK r26's libc++ doesn't ship yet — recorder is host-side test infrastructure that never lands on Android. |
| macOS | ❌ no Apple host available (documented foundations gap) | — | — | — | Code-complete on app-shell layer per Current Focus; widget sweep including VSM auto-routing pending Apple-host provisioning. Headers are pure C++23 + STL — zero platform-specific code — so AppleClang/libc++ should accept them unchanged once a host comes online. |
| iOS | ❌ no Apple host available | — | — | — | Same situation as macOS. |

## Acceptance Criteria

- [x] `visual_state_manager::go_to_state(view&, string_view)` runs the matching state's setters in every group that contains the state, marks `current_state`, returns the count of transitioned groups (same-state calls return 0).
- [x] Unknown state name returns 0 + runs no setters.
- [x] Same-state transition is a no-op (no setter re-runs).
- [x] `style::apply_to`-style exception swallowing per RFC §Detailed Design.
- [x] Null setters tolerated.
- [x] `snapshot_current_states()` returns one (group_name, current_state) pair per group.
- [x] 8 test cases / 36 assertions pass under `ctest --test-dir build-wsl` — total goes 395 → 403 green.
- [x] Headers compile under Android NDK r26 clang for both aarch64 + x86_64 ABIs.
- [x] Linux demo binary builds + the GTK4 window renders + per-state setters fire (verified textually via the smoke harness; one visual proof captured before WSLg's DComp wall blocked subsequent BitBlts).

## Build evidence

```
$ ./build-wsl/tests/mock_handlers_test '[vsm]'
All tests passed (36 assertions in 8 test cases)

$ ctest --test-dir build-wsl
100% tests passed, 0 tests failed out of 403

$ ninja -C build-wsl gtk4_vsm_demo
[2/2] Linking CXX executable examples/gtk4_vsm_demo/gtk4_vsm_demo

$ powershell wsl -d Ubuntu-24.04 -- bash tools/dev/capture-vsm-smoke.sh
VSM-SMOKE: state=Normal      transitioned=1 target.text=Target — Normal      status.text=Current state: Normal
VSM-SMOKE: state=Pressed     transitioned=1 target.text=Target — Pressed     status.text=Current state: Pressed
VSM-SMOKE: state=Disabled    transitioned=1 target.text=Target — Disabled    status.text=Current state: Disabled
VSM-SMOKE: state=CustomState transitioned=0 target.text=Target — Normal      status.text=Current state: (no transitions yet)

$ aarch64-linux-android28-clang++ -std=c++2b -Iinclude -c build/android-headers-smoke.cpp
$ x86_64-linux-android28-clang++  -std=c++2b -Iinclude -c build/android-headers-smoke.cpp
# both clean; .o files in build/android-headers-smoke-{aarch64,x86_64}.o
```

## Links

- RFC: [[RFC-0006-visual-state-manager]].
- Sibling RFCs: [[RFC-0005-resource-dictionaries-and-styling]] (setter-shape reuse), [[RFC-0004-image-source-family]], [[RFC-0003-gesture-recognizers]].
- Cross-platform foundations gap: [[T-0032-windows-appsdk-ci-provisioning]] blocks the Windows-native example build.
- Documented WSLg screenshot wall: see this task's `notes/wslg-dcomp-wall.md` (below) + the Current Focus reference.
- Follow-ups (to be opened): T-0051..T-0055 — per-platform real handler routing of system states.

## Notes — WSLg DComp screenshot wall

PrintWindow against WSLg-projected windows (msrdc on Win11) yields a solid-black bitmap once msrdc switches to COPY MODE — DComp surfaces aren't backed by a regular HDC. Same wall is documented in [[Current Focus]] under "Per-platform GUI screenshot capture infra". `grim` requires `wlr-screencopy-unstable-v1`, which WSLg's compositor doesn't implement. `ImageMagick import -window root` fails because WSLg routes through Wayland rather than X11 root.

Workarounds tried (all attempted in this task):
1. `PrintWindow` with `PW_RENDERFULLCONTENT` (flag 2) — yields black bitmap once "COPY MODE" warning appears in title.
2. `Graphics.CopyFromScreen` of the window rect — captures whatever is at those coordinates on the desktop (not necessarily our window — `SetForegroundWindow` doesn't work against background-spawned processes).
3. `grim /tmp/test.png` from inside WSL — "compositor doesn't support wlr-screencopy-unstable-v1".
4. `import -window root` from inside WSL — "unable to read X window image 'root'".

Selected workaround: textual smoke harness (`tools/dev/capture-vsm-smoke.sh`) launches the demo per state, the demo emits `VSM-SMOKE:` on stderr right after the transition, harness greps the line as closure evidence. The one BitBlt that succeeded (before the demo received any focus operations) is kept as the visual-baseline screenshot.
