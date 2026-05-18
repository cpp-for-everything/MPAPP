---
type: task
id: T-0007
title: GTK4 hello-window via WSLg on Windows host
status: done
milestone: M-02
owner: ""
area: platform-linux
blockedBy: []
coveragePercent: 100
hasScreenshots: true
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/build
  - platform/linux
  - phase/p1
---

# T-0007 — WSLg GTK4 hello window

## Goal

Prove the Linux dev loop from a Windows host using WSLg. Install Ubuntu
under WSL2, install GTK4 dev packages, build a GTK4 smoke target from
the Windows host, and run it as a native window on the Windows 11
desktop via WSLg.

## Status

**Done** — unblocked and completed end-to-end during
[[T-0011-app-shell-abstraction]]. WSL2 + Ubuntu-24.04 installation
verified live; GTK4 dev packages installed via apt; the
`examples/gtk4_hello/` target was simultaneously rewritten against the
MPAPP app-shell abstraction (zero raw `gtk_*` / `GTK_*` tokens in user
code) and now compiles + runs through WSLg with a real interactive
window. Live screenshot evidence (button clicks driving Count 0 → 5)
is at
[[T-0011-app-shell-abstraction/screenshots/evidence#Linux — `gtk4_hello`]].

## Acceptance Criteria

- [x] `examples/gtk4_hello/` example authored, gated on
      `UNIX AND NOT APPLE`. (Originally raw C/GTK4; rewritten in
      [[T-0011-app-shell-abstraction]] to use the MPAPP surface.)
- [x] `examples/CMakeLists.txt` wires the new subdirectory behind the
      same gate.
- [x] WSL2 + Ubuntu install recipe documented in
      [[wslg-setup]] (Ubuntu-24.04 used in the live verification).
- [x] `apt install libgtk-4-dev ninja-build pkg-config build-essential`
      executed live in Ubuntu-24.04 under WSL2.
- [x] Linux x86_64 binary built from the Windows host (via WSL invoking
      clang 18.1.3 + CMake 3.28 + Ninja inside Ubuntu — same source
      tree mounted at `/mnt/d/GitHub/MPAPP`).
- [x] Running the binary inside WSL2 shows a GTK4 window on the
      Windows desktop via WSLg.
- [x] Screenshot of the GTK4 window rendering on Windows desktop —
      captured under
      [[T-0011-app-shell-abstraction/screenshots/]] (Count: 0 → 5
      interactive clicks verified end-to-end).
- [ ] Screen recording of the dev loop (edit, rebuild, run). Deferred
      to the polish pass when the spike is promoted to a real sample
      app — not a blocker on T-0007's primary goal.
- [x] Fallback documentation captured in [[wsl-blocker]] (covers the
      WSL-absent host case).

## Notes

The original blocker was the worker environment lacking a WSL install
(see [[wsl-blocker]]). The blocker resolved organically when
[[T-0011-app-shell-abstraction]] needed a live Linux build target — the
existing Ubuntu-24.04 WSL distro on this host was used. Ubuntu-24.04
satisfies the Ubuntu-22.04+ requirement.

The originally-shipped raw GTK4 C source (`main.c`) has been replaced
by the MPAPP-abstracted `main.cpp` — the deliverable is now functional
proof of the cross-platform pipeline, not just a smoke test of WSLg.

## Links

- Setup: [[wslg-setup]]
- Blocker (now resolved): [[wsl-blocker]]
- Source: `examples/gtk4_hello/main.cpp`, `examples/gtk4_hello/CMakeLists.txt`
- Closure: [[T-0011-app-shell-abstraction]] (Linux section of evidence.md)
- Milestone: [[M-02-Infrastructure]]
- Related: [[Build System]], [[70_References/WSLg]], [[70_References/GTK4]]
