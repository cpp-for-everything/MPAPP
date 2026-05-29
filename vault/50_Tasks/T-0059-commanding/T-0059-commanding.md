---
type: task
id: T-0059
title: RFC-0014 commanding — relay_command (ICommand) mock surface
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
  - phase/p2
---

# T-0059 — Commanding

## Goal

Land [[RFC-0014-commanding]]: `command_base` (ICommand) + `relay_command` + `relay_command_of<T>` — the runtime command object a control binds to.

## Scope

In: `include/mpapp/binding/relay_command.hpp` + `tests/mock_handlers/relay_command_test.cpp` (6 cases / 20 assertions).
Out: a `command` property on controls (auto-invoke + is_enabled from can_execute); `{Binding}` Command XAML lowering; `async_relay_command`.

## Per-platform verification

Platform-neutral (`<functional>` + intrusive signal).

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ ctest 455 → **461**; `[command]` 6 cases / 20 assertions |
| Windows MSVC | ✅ via windows-native gate |
| Android NDK r26 aarch64 | ✅ compiles clean (`tests/android-command-smoke.cpp`) |
| Apple | ❌ no host |

## Acceptance Criteria

- [x] `relay_command` executes its action; `can_execute` gates it; `can_execute_changed` fires.
- [x] `relay_command_of<T>` passes the parameter to execute/can_execute + gates.
- [x] Polymorphic invoke via `command_base&`.
- [x] No macros; platform-neutral.

## Build evidence

```
$ ./build-wsl/tests/mock_handlers_test '[command]'  -> 20 assertions, 6 cases
$ ctest --test-dir build-wsl                        -> 461/461
$ aarch64-linux-android28-clang++ -std=c++2b -c android-command-smoke.cpp -> ok
```

## Links

- RFC: [[RFC-0014-commanding]]. Completes the MVVM trio with [[RFC-0007-data-binding]] + [[T-0058-value-converters]].
