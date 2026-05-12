---
type: task
id: T-0003
title: WinUI 3 button handler spike
status: todo
milestone: M-01
owner: ""
area: handlers
blockedBy:
  - T-0002
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/handlers
  - platform/windows
  - phase/p0
---

# T-0003 — WinUI 3 button handler spike

## Goal

Prove the CRTP handler architecture by implementing a real `button_handler<platform::windows>` against C++/WinRT and WinUI 3. Single button, single property (`text`), single event (`clicked`). Click in a desktop window → command fires → observable count increments → label updates.

## Acceptance Criteria

- [ ] `src/handlers/windows/button_handler.cpp` implements the handler.
- [ ] `examples/windows_button_spike/main.cpp` opens a WinUI 3 window with a `mpapp::button` bound to a `todo_view_model`.
- [ ] Click increments `count`, label re-renders with new value.
- [ ] Build succeeds on Windows host (MSVC).
- [ ] Screenshots of the working window in `screenshots/`.
- [ ] Screen recording showing click → count increment in `recordings/`.
- [ ] 100% coverage on the new handler.

## Notes

Document any C++/WinRT pitfalls (IInspectable refcounting, generic boxed types) in `notes/`.

## Links

- Milestone: [[M-01-Foundations]]
- Related: [[Handlers]], [[Platform Interop]], [[Components/Button]], [[70_References/CppWinRT]]
