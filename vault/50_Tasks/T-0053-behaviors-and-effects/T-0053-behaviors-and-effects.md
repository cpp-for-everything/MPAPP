---
type: task
id: T-0053
title: RFC-0009 Behaviors + Effects — mock surface + cross-platform verify
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
  - phase/p2
---

# T-0053 — Behaviors + Effects

## Goal

Land [[RFC-0009-behaviors-and-effects]]: `mpapp::behavior` + `view::behaviors`/`add_behavior`/`remove_behavior`; `mpapp::effect` + `view::effects`/`add_effect`. Attached extensibility points mirroring `gesture_recognizers`.

## Scope

In: `include/mpapp/behaviors/behavior.hpp`, `include/mpapp/effects/effect.hpp`, `view.hpp` collections + add/remove, `tests/mock_handlers/behavior_test.cpp` (4 cases / 13 assertions).
Out: `<Behavior>`/`<Effect>` XAML lowering (M-09); typed `behavior_of<TView>`; per-platform effect registry.

## Per-platform verification

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ ctest 421 → **425**; `[behavior]` 4 cases / 13 assertions |
| Windows MSVC | ✅ covered by windows-native gate (view.hpp + header-only, no platform code) |
| Android NDK r26 aarch64 | ✅ view.hpp consumer recompiles clean with the new includes |
| Apple | ❌ no host (pure C++23/STL) |

## Acceptance Criteria

- [x] `add_behavior<B>` constructs (forwarding ctor args), pushes, calls `on_attached`; returns ref.
- [x] `remove_behavior` calls `on_detached` + erases.
- [x] `add_effect<E>` attaches; `effect::resolution_id()` exposed.
- [x] No macros; mirrors `gesture_recognizers`; platform-neutral.
- [x] Green Linux; compiles Android NDK + (via windows-native gate) MSVC.

## Build evidence

```
$ ./build-wsl/tests/mock_handlers_test '[behavior]'   -> 13 assertions, 4 cases
$ ctest --test-dir build-wsl                          -> 425/425
$ aarch64-linux-android28-clang++ -std=c++2b -c (view.hpp consumer) -> ok
```

## Links

- RFC: [[RFC-0009-behaviors-and-effects]]. Sibling: [[RFC-0008-triggers]].
