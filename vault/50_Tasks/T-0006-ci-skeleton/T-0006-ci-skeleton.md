---
type: task
id: T-0006
title: GitHub Actions skeleton with budget-aware sharding and self-hosted runner
status: in-progress
milestone: M-02
owner: ""
area: tooling
blockedBy:
  - T-0001
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/tooling
  - area/build
  - phase/p1
---

# T-0006 — CI skeleton

## Goal

Stand up the GitHub Actions matrix described in [[CI Strategy]]. Per-PR builds shard aggressively; self-hosted runner registered for long Android emulator jobs; macOS jobs deferred to tagged releases until the MacBook is online.

## Acceptance Criteria

- [ ] `.github/workflows/pr.yml` runs on every PR:
  - [ ] `windows-native` job builds Windows-x64 via MSVC + Clang.
  - [ ] `windows-cross` job cross-builds linux-x64 + android-arm64.
  - [ ] `linux-native` job builds linux-x64.
  - [ ] `linux-cross` job cross-builds windows-x64 + android-arm64.
- [ ] `.github/workflows/release.yml` runs on tagged releases:
  - [ ] All PR jobs.
  - [ ] `macos-native` job builds macos-arm64 + ios-arm64 (Simulator).
- [ ] Self-hosted runner labeled `mpapp-windows-self` registered on the user's Windows machine.
- [ ] `android-emulator` workflow targets `mpapp-windows-self` for Android emulator runs.
- [ ] ccache integrated; cache hit rate ≥ 50% on second run.
- [ ] Smoke test from T-0001 runs in every job.
- [ ] Documentation in `notes/` describes how to add/maintain self-hosted runners.

## Notes

Action minutes budget is in [[CI Strategy]]. If a job grows past its allocation, shard it more aggressively or move it to self-hosted.

## Links

- Milestone: [[M-02-Infrastructure]]
- Related: [[CI Strategy]], [[Build System]], [[CLAUDE]] rule 8
- Workflows: [`.github/workflows/pr.yml`](../../../.github/workflows/pr.yml), [`.github/workflows/release.yml`](../../../.github/workflows/release.yml)
- Runner setup: [[runner-setup]]
- Zig install: [[zig-install]]
