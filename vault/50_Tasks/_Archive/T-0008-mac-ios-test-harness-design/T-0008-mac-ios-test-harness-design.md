---
type: task
id: T-0008
title: Design human-free macOS / iOS UI test harness
status: done
milestone: M-02
owner: ""
area: tooling
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
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
- Design: [[design|notes/design.md]]

## Closure notes

- **Closed:** 2026-05-12
- **Merged commits:** `f681645` (design doc on branch `batch/test-harness-design`), `cd38a07` (merge into main).
- **Delivered:** 286-line `notes/design.md` covering the human-free macOS / iOS test-harness design — `xcrun simctl` + AppleScript/Accessibility API + XCUITest stack, full build → boot → install → launch → test → screenshot → teardown workflow, stuck-simulator recovery, self-hosted runner setup (security, sign-in, code-signing), screenshot capture strategy with PR-artifact attachment, failure-mode coverage, implementation-effort breakdown, and risks. Implementation deferred to M-07 per the task spec.
- **Coverage:** design-only task — `100%` per the task gate.
