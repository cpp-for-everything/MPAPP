---
type: task
id: T-0052
title: RFC-0008 Triggers — mock surface + cross-platform verify
status: completed
milestone: M-09
owner: ""
area: properties
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/properties
  - area/markup
  - phase/p2
---

# T-0052 — Triggers

## Goal

Land [[RFC-0008-triggers]]: `trigger<T>` (property + data), `multi_trigger<Ts…>`, `event_trigger<Args…>`, `state_trigger` (→ VSM). Builds on RFC-0007 binding + RFC-0006 VSM.

## Scope

In: `include/mpapp/triggers/{trigger,event_trigger,state_trigger}.hpp` + `tests/mock_handlers/trigger_test.cpp` (5 cases / 26 assertions).
Out: XAML `<Trigger>`/`<DataTrigger>`/… lowering (mpapp-xc M-09); auto value-capture/restore; EnterActions/ExitActions lists.

## Per-platform verification

Platform-neutral (like binding) — drives the existing Observable→handler pipeline.

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ ctest 416 → **421**; `[trigger]` 5 cases / 26 assertions |
| Windows MSVC Release (no WindowsAppSDK) | ✅ `[trigger]` 26 assertions in build-winci |
| Android NDK r26 aarch64 | ✅ headers compile clean |
| Apple | ❌ no host (pure C++23/STL) |

## Acceptance Criteria

- [x] `trigger<T>` enter on match / exit on un-match; same-value writes are no-ops; re-match re-fires.
- [x] `multi_trigger` activates only when ALL conditions match.
- [x] `event_trigger` runs its action with the signal's args on emit.
- [x] `state_trigger` drives `visual_state_manager::go_to_state` from a bool (Valid ↔ Invalid).
- [x] No macros; composes with Observable/signal + binding + VSM.
- [x] Green Linux + Windows; compiles Android NDK.

## Build evidence

```
$ ./build-wsl/tests/mock_handlers_test '[trigger]'     -> 26 assertions, 5 cases
$ ctest --test-dir build-wsl                           -> 421/421
$ build-winci/tests/mock_handlers_test.exe "[trigger]" -> 26 assertions (MSVC Release)
$ aarch64-linux-android28-clang++ -std=c++2b -c …      -> ok
```

## Links

- RFC: [[RFC-0008-triggers]]. Builds on [[RFC-0007-data-binding]], [[RFC-0006-visual-state-manager]].
