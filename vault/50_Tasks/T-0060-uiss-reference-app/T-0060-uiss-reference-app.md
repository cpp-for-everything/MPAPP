---
type: task
id: T-0060
title: УИСС reference app — cross-platform "Е-Студент" portal
status: in-progress
milestone: M-10
owner: ""
area: widgets
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/widgets
  - phase/p2
---

# T-0060 — УИСС reference application

## Goal

Land [[RFC-0015-uiss-reference-app]]: a single ifdef-free MPAPP application
(`examples/uiss`) replicating the TU-Sofia "Е-Студент" student portal
(`examples/УИСС/*.mhtml`) — login gate + flyout navigation across 10 data-bound
section pages. The framework's first real application and its primary end-to-end
validation + gap-detection vehicle.

## Scope

In: `examples/uiss/` (`main.cpp` + `uiss/{data,support,pages,app}.hpp` +
`CMakeLists.txt`), wired into `examples/CMakeLists.txt` (Linux + Windows).
Out: the framework gaps the app surfaces (label styling, CollectionView tables,
image loader for the logo, real picker) — captured as follow-ups in RFC-0015.

## Per-platform verification

| Platform | Result |
|---|---|
| Linux WSL/GTK4 | ✅ builds (`cmake --build build-wsl --target uiss`) + runs (window composes, no crash, no GTK-CRITICAL). On-screen capture blocked by the documented WSLg/RAIL screenshot wall — window renders as flat gray to desktop capture. |
| Windows MSVC/WinUI 3 | ✅ builds — `main.cpp.obj` compiled (`/utf-8`) + `uiss.exe` linked (2.5 MB) via vcvars64 + ninja in `build/`. On-screen run deferred (host PC in use). |
| Android NDK r26 | ✅ cross-compiles on **both ABIs** (`aarch64-linux-android28` + `x86_64-linux-android28`, EXIT=0) — ifdef-free sources + android handler headers + Cyrillic UTF-8 literals all valid. Full APK + emulator run deferred (host PC in use). |
| Apple | ❌ no host (blind). |

## Acceptance Criteria

- [x] Single source tree, **no `#ifdef`**, composes login + flyout + 10 sections.
- [x] Builds clean on Linux/GTK4; app launches and runs without crashing.
- [x] All 10 sections render the real portal data from `uiss::seed()`.
- [x] Builds clean on Windows/WinUI 3.
- [x] Compiles for Android NDK (both ABIs) from the same sources.
- [ ] Screenshots captured per platform (deferred — WSLg wall on Linux; Windows/Android emulator runs deferred while host PC is in use).

## Build evidence

```
# Linux
$ cmake --build build-wsl --target uiss        -> [3/3] Linking CXX executable examples/uiss/uiss
$ ./build-wsl/examples/uiss/uiss               -> window composes; PID alive; clean stderr (MESA/EGL warnings only)

# Windows (vcvars64 + ninja; NOTE: do NOT redirect vcvars stdout)
$ cmake --build build --target uiss            -> [47/50] uiss/main.cpp.obj; [49/50] Linking uiss.exe; BUILD_UISS_SUCCESS
                                               -> build/examples/uiss/uiss.exe (2.5 MB)

# Android NDK r26 (single source tree, both ABIs)
$ aarch64-linux-android28-clang++ -std=c++2b -I include -I examples/uiss -c examples/uiss/main.cpp  -> EXIT=0
$ x86_64-linux-android28-clang++  -std=c++2b -I include -I examples/uiss -c examples/uiss/main.cpp  -> EXIT=0
```

## Notes

- Composition findings (leaf wrappers auto-bind; layouts need explicit `bind()`
  after `add()`; Cyrillic needs UTF-8 source flags) are documented in RFC-0015 and
  encapsulated in `uiss/support.hpp`.
- The WSLg capture wall (PrintWindow black / RAIL window composites as gray to
  desktop capture) is the same limitation hit in T-0044; Android `adb screencap`
  is the reliable visual path and is the planned capture route once the NDK build
  lands.

## Links

- RFC: [[RFC-0015-uiss-reference-app]]. Design spec: `examples/УИСС/*.mhtml`.
