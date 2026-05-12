---
type: task
id: T-0007
title: GTK4 hello-window via WSLg on Windows host
status: todo
milestone: M-02
owner: ""
area: platform-linux
blockedBy:
  - T-0001
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/build
  - platform/linux
  - phase/p1
---

# T-0007 — WSLg GTK4 hello window

## Goal

Prove the Linux dev loop from a Windows host using WSLg. Install Ubuntu 22.04 under WSL2, install GTK4 dev packages, cross-build a `mpapp::window` smoke target from CMake (Linux toolchain), and run it as a native window on the Windows 11 desktop via WSLg.

## Acceptance Criteria

- [ ] WSL2 + Ubuntu 22.04 installed and documented in `notes/wslg-setup.md`.
- [ ] `apt install build-essential libgtk-4-dev clang cmake ninja-build` succeeds.
- [ ] `mpapp build --target linux-x64` from the Windows host produces a Linux x86_64 binary.
- [ ] Running the binary inside WSL2 shows a GTK4 window on the Windows desktop (WSLg).
- [ ] Screenshot of the GTK4 window rendering on Windows desktop in `screenshots/`.
- [ ] Screen recording of the dev loop (edit, rebuild, run) in `recordings/`.
- [ ] Fallback documentation in `notes/hyper-v-fallback.md` for what to do if WSLg has GTK4 issues.

## Notes

WSLg auto-installs in Windows 11. On Windows 10 / older builds, fall back to Hyper-V Ubuntu VM.

## Links

- Milestone: [[M-02-Infrastructure]]
- Related: [[Build System]], [[70_References/WSLg]], [[70_References/GTK4]]
