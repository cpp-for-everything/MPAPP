---
type: task
id: T-0054
title: RFC-0010 Animations — mock surface + cross-platform verify
status: completed
milestone: M-09
owner: ""
area: widgets
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/widgets
  - area/threading
  - phase/p2
---

# T-0054 — Animations

## Goal

Land [[RFC-0010-animations]]: `easing`, `animation` (advanceable tween), `animation_manager`, and `ViewExtensions` (`fade_to`/`scale_to`/`rotate_to`/`translate_to`). Platform-neutral interpolation; native frame ticker is Phase 6.

## Scope

In: `include/mpapp/animation/{easing,animation,animation_manager,view_animations}.hpp` + `tests/mock_handlers/animation_test.cpp` (7 cases / 38 assertions).
Out: native vsync ticker (Phase 6 / ADR-0019); `co_await` sugar; keyframe/parallel/sequential composition; Storyboard XAML lowering.

## Per-platform verification

Platform-neutral interpolation; only the per-vsync `tick` cadence is native (Phase 6).

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ ctest 425 → **432**; `[animation]` 7 cases / 38 assertions |
| Windows MSVC | ✅ via windows-native gate (header-only, no platform code) |
| Android NDK r26 aarch64 | ✅ all animation headers compile clean (`tests/android-animation-smoke.cpp`) |
| Apple | ❌ no host (pure C++23/STL + `<cmath>`) |

## Acceptance Criteria

- [x] Easing curves hit known anchors + clamp out-of-range t (linear, quad, cubic, sin, bounce_out, spring_out overshoot).
- [x] `animation` interpolates from→to, fires on_finished once, no-ops after finish; honours easing mid-flight.
- [x] `animation_manager` advances active + drops finished.
- [x] `fade_to`/`scale_to`/`rotate_to`/`translate_to` drive the right `view` Observables; translate covers both axes.
- [x] No macros; deterministic (explicit `advance(dt)`).
- [x] Green Linux; compiles Android NDK; MSVC via gate.

## Build evidence

```
$ ./build-wsl/tests/mock_handlers_test '[animation]'   -> 38 assertions, 7 cases
$ ctest --test-dir build-wsl                           -> 432/432
$ aarch64-linux-android28-clang++ -std=c++2b -c android-animation-smoke.cpp -> ok
```

## Links

- RFC: [[RFC-0010-animations]]. Frame ticker: [[ADR-0019-async-executor-native-dispatcher]] (Phase 6).
