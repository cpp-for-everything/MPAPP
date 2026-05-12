---
type: task
id: T-0007
title: GTK4 hello-window via WSLg on Windows host
status: blocked
milestone: M-02
owner: ""
area: platform-linux
blockedBy:
  - T-0001
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/blocked
  - area/build
  - platform/linux
  - phase/p1
---

# T-0007 — WSLg GTK4 hello window

## Goal

Prove the Linux dev loop from a Windows host using WSLg. Install Ubuntu 22.04 under WSL2, install GTK4 dev packages, cross-build a `mpapp::window` smoke target from CMake (Linux toolchain), and run it as a native window on the Windows 11 desktop via WSLg.

## Status

Source + CMake target landed; install + screenshot steps blocked on
the worker environment (WSL not installed, session non-elevated). See
[[wsl-blocker]] for the captured error output and recovery path. The
`examples/gtk4_hello/` target is gated on `UNIX AND NOT APPLE`, so the
change is inert for the Windows build.

## Acceptance Criteria

- [x] `examples/gtk4_hello/main.c` + `CMakeLists.txt` authored, gated
      on `UNIX AND NOT APPLE`.
- [x] `examples/CMakeLists.txt` wires the new subdirectory behind the
      same gate.
- [x] WSL2 + Ubuntu 22.04 install recipe documented in
      [[wslg-setup]].
- [x] `apt install build-essential libgtk-4-dev cmake ninja-build
      pkg-config` documented in [[wslg-setup]].
- [ ] `mpapp build --target linux-x64` from the Windows host produces a
      Linux x86_64 binary. *(blocked — see [[wsl-blocker]])*
- [ ] Running the binary inside WSL2 shows a GTK4 window on the
      Windows desktop (WSLg). *(blocked)*
- [ ] Screenshot of the GTK4 window rendering on Windows desktop in
      `screenshots/`. *(blocked)*
- [ ] Screen recording of the dev loop (edit, rebuild, run) in
      `recordings/`. *(blocked)*
- [x] Fallback documentation captured in [[wsl-blocker]] (covers the
      WSL-absent host case).

## Notes

WSLg auto-installs in Windows 11. On Windows 10 / older builds, fall back to Hyper-V Ubuntu VM.

## Links

- Setup: [[wslg-setup]]
- Blocker: [[wsl-blocker]]
- Source: `examples/gtk4_hello/main.c`, `examples/gtk4_hello/CMakeLists.txt`
- Milestone: [[M-02-Infrastructure]]
- Related: [[Build System]], [[70_References/WSLg]], [[70_References/GTK4]]
