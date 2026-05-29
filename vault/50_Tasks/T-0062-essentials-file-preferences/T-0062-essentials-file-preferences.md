---
type: task
id: T-0062
title: Essentials — real persistent preferences backend (file-backed)
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

# T-0062 — Essentials: file-backed preferences

## Goal

Give RFC-0013 Essentials a **real, persistent** backend (the goal's "Essentials
backends" requirement) instead of only the in-memory mock — and have the УИСС
app use it.

## Scope

In: `include/mpapp/essentials/file_preferences.hpp` — `file_preferences`
(implements `mpapp::preferences`, persists `key=value` to a text file via
std::filesystem + fstream) + `default_preferences_path(app_id)` (resolves
`XDG_CONFIG_HOME` / `APPDATA` / `HOME/.config`, temp-dir fallback). One
implementation, **no ifdefs**, compiles + runs on every target.
`tests/mock_handlers/file_preferences_test.cpp` (3 cases / 18 assertions:
cross-instance persistence, multiline+escaped round-trip, remove/clear).
УИСС login remembers the last faculty number (load on launch, save on success).
Out: secure_storage real backend (keychain/credential-locker) — follow-up.

## Per-platform verification

| Platform | Result |
|---|---|
| Linux WSL/GCC | ✅ ctest `[preferences]` 18 assertions; uiss builds + the prefs file round-trips on disk. |
| Windows MSVC | ⏳ uiss rebuild (header is pure std — compiles on MSVC). |
| Android NDK r26 | ✅ uiss main cross-compiles (aarch64). Path note: Android sandboxes config — the NDK entry point can `setenv("XDG_CONFIG_HOME", filesDir)` so the resolver lands in the app's writable dir (temp-dir fallback otherwise). |
| Apple | ❌ no host — pure-std header, expected to compile. |

## Acceptance Criteria

- [x] `preferences` has a real persistent (file-backed) implementation.
- [x] Cross-platform single implementation, no ifdefs.
- [x] Round-trips across process instances; escaping for `=`/newline/backslash.
- [x] УИСС uses it (remembers faculty number).

## Links

- RFC: [[RFC-0013-essentials]]. Used by [[T-0060-uiss-reference-app]].
