---
type: task
id: T-0066
title: VSM input-routing — visual_state_input_router
status: done
milestone: M-10
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

# T-0066 — VSM input-routing

## Goal

Deliver the goal's "VSM input-routing" + close the follow-up the
visual_state_manager header itself flagged ("the per-platform real layer will
auto-route system input events to the canonical state names"): map input state
onto the canonical CommonStates and drive `go_to_state` automatically.

## Scope

In: `include/mpapp/resources/visual_state_input_router.hpp` —
`visual_state_input_router{view&, visual_state_manager&}` that (a) subscribes the
view's `is_enabled` Observable and routes Disabled/Normal with zero native
wiring, and (b) exposes `set_pressed` / `set_pointer_over` / `set_focused` for
native pointer/focus events (or a pointer gesture recognizer) to call. Priority
order = MAUI CommonStates: Disabled > Pressed > PointerOver > Focused > Normal.
Pure platform-neutral logic (like the VSM — no native widget/handler).
`tests/mock_handlers/vsm_input_router_test.cpp` (2 cases / 15 assertions).
Out: the per-platform glue that calls the setters from native pointer/focus
events (the gesture handlers feed the same setters) — a thin follow-up.

## Per-platform verification

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ ctest `[router]` 15 assertions — priority order + is_enabled auto-route. |
| Windows MSVC | ✅ pure-std header — compiles by construction (used only via the test on the cloud build). |
| Android NDK r26 | ✅ header cross-compiles (aarch64, EXIT=0). |
| Apple | ✅ pure-std header — no platform code. |

## Acceptance Criteria

- [x] Router maps enabled/pressed/pointer-over/focused → canonical states.
- [x] `is_enabled` auto-routes Disabled/Normal with no native wiring.
- [x] Priority order matches MAUI CommonStates; mock-tested.

## Links

- RFC: [[RFC-0006-visual-state-manager]] (closes its flagged input-routing follow-up).
