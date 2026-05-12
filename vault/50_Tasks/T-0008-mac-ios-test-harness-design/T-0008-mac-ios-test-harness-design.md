---
type: task
id: T-0008
title: Design human-free macOS / iOS UI test harness
status: todo
milestone: M-02
owner: ""
area: tooling
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/tooling
  - platform/macos
  - platform/ios
  - phase/p1
---

# T-0008 — Design human-free macOS / iOS UI test harness

## Goal

Design (not implement) the test harness that will let Claude iterate on macOS and iOS handlers **without human intervention** once the user's MacBook Pro comes online as a self-hosted runner.

The design document goes in `notes/design.md` (or `notes/design/`). Implementation lands when M-07 starts.

## Acceptance Criteria

- [ ] `notes/design.md` covers:
  - [ ] Tooling stack: `xcrun simctl` for iOS Simulator; AppleScript + Accessibility API for macOS; `XCUITest` for any deeper instrumentation.
  - [ ] Workflow: app build → boot simulator → install → launch → run UI tests → capture screenshots → tear down → report.
  - [ ] Recovery: stuck simulators get programmatic reset (`xcrun simctl shutdown all`, `xcrun simctl erase all`).
  - [ ] Self-hosted runner setup (security, sign-in flow, code-signing).
  - [ ] Screenshot capture strategy (auto-attached to PR artifacts).
  - [ ] Failure-mode coverage: timeout handling, simulator boot failure, code-signing failure, accessibility-permission grant.
- [ ] Estimated implementation effort breakdown.
- [ ] Risks documented.
- [ ] No coverage required (this is a design task) — set `coveragePercent: 100` once the design doc is reviewed.

## Notes

This task exists in M-02 — well before any macOS work — specifically so the harness is ready when the MacBook arrives. Otherwise iteration time on Apple platforms is gated on human availability, which kills throughput.

## Links

- Milestone: [[M-02-Infrastructure]]
- Related: [[Test Harness]], [[M-07-macOS-Real]], [[M-08-iOS-Real]]
