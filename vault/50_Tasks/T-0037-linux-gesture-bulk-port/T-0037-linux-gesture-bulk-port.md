---
type: task
id: T-0037
title: Bulk-port linux_gestures::attach to every per-component Linux handler
status: in-progress
milestone: M-04b
owner: ""
area: handlers
blockedBy: []
coveragePercent: 95
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/handlers
  - phase/p4
  - platform/linux
---

> [!success] Plumbing complete on 2026-05-25 (commits b0a999d + 0d59562)
> Code + wrapper sweep landed via `tools/dev/sweep-gesture-map.py`:
> 238 hpp injections + 58 Linux .cpp implementations + 58 wrapper ctors
> refreshed with `embedded_handler_.map_gestures(*this);`. CI green
> (linux-native 54 s, 370/370 tests pass). The dual-branch
> `web_view_handler.cpp` + `hybrid_web_view_handler.cpp` got
> stub-branch `map_gestures` fixes so the link is clean regardless of
> webkitgtk-6.0 availability.
>
> Remaining work: Rule-11 closure requires an end-to-end recording of
> a gesture landing on a non-button widget via a real GTK4 display
> (WSLg or a Linux desktop). Task stays `in-progress` until that
> evidence lands; the `coveragePercent: 95` reflects the code path being
> 100% exercised through the mock contract (`view_handler<platform::mock>
> ::map_gestures` + `simulate_*` helpers) but not yet through a real
> input event.

# T-0037 — Bulk-port `linux_gestures::attach` to every per-component Linux handler

## Goal

Extend the [[T-0033-gesture-recognizers-tap-slice|RFC-0003]] per-component wire-up from `button_handler<platform::linux_>` (the pilot) to every other migrated component on Linux. After this lands, any `mpapp::<name>` wrapper that the user creates on Linux will get GtkGesture* controllers installed for every recognizer in its `gesture_recognizers` collection — no app code change required.

## Background

The first slice of RFC-0003 landed:

- The 5-recognizer mock surface (`tap`, `pan`, `pinch`, `swipe`, `pointer`) — commit `2604889`.
- The Linux `linux_gestures::attach(GtkWidget*, view&)` free function that walks `view::gesture_recognizers` and installs the matching GtkGesture* controller — landed alongside the canonical button wire-up.
- `button_handler<platform::linux_>::map_gestures(basic_button&)` calls `linux_gestures::attach(static_cast<GtkWidget*>(native_), b)`, and the `mpapp::button` wrapper's ctor calls `embedded_handler_.map_gestures(*this)`. To keep the call valid on platforms whose real implementation isn't ready yet, no-op `map_gestures` stubs landed on mock + Windows + Android + macOS + iOS button handlers.

What's missing is the analogous wire-up across the other 57 leaf components.

## Scope

For every migrated component listed in [[Controls Inventory]] (i.e. every header under `include/mpapp/internal/basic_*.hpp` except `basic_button.hpp` which is done):

1. **Per-platform handler header** — declare `void map_gestures(basic_<name>& b);` on every per-platform handler class:
   - `include/mpapp/handlers/linux/<name>_handler.hpp` — real declaration; implementation in the .cpp.
   - `include/mpapp/handlers/{windows,android,macos,ios}/<name>_handler.hpp` — inline no-op stub (`void map_gestures(basic_<name>& /*b*/) noexcept {}`) until the per-platform real-handler tasks land.
   - `include/mpapp/handlers/mock/<name>_handler.hpp` — inline no-op stub (the cross-cutting gesture-attach assertion already lives on `view_handler<platform::mock>::map_gestures`; the per-component mock doesn't need to re-record).
2. **Per-component Linux .cpp** — define the implementation:
   ```cpp
   void <name>_handler<platform::linux_>::map_gestures(basic_<name>& b) {
       if (native_ == nullptr) return;
       linux_gestures::attach(static_cast<GtkWidget*>(native_), b);
   }
   ```
   Plus `#include "mpapp/handlers/linux/gesture_attach.hpp"` at the top.
3. **Wrapper ctor** — call `embedded_handler_.map_gestures(*this);` after the existing `map_*` / `bind` calls. The cleanest way is to update `tools/dev/migrate-component.py`'s wrapper template to emit it unconditionally, then run `--regen-wrapper` against every component.

Out of scope:

- Per-platform real handlers for Windows / Android / macOS / iOS — separate tasks.
- Custom recognizer pre-flight logic (e.g. only attach pinch on widgets that support multi-touch). The recognizer set + filter logic lives in `linux_gestures::install_<kind>`; this task is pure plumbing.
- Layouts (stack_layout / grid_layout) using `bind(basic_<name>&)` style — they need their wrapper ctor extended too, but the script handles both `map_X` and `bind` style wrappers already.

## Acceptance Criteria

- [ ] Every per-platform handler hpp under `include/mpapp/handlers/{linux,windows,android,macos,ios,mock}/<name>_handler.hpp` declares `map_gestures`. Verified via `grep -L "map_gestures" include/mpapp/handlers/*/`<each name>`_handler.hpp` returning empty.
- [ ] Every per-component Linux .cpp file implements `map_gestures` by calling `linux_gestures::attach`.
- [ ] Every wrapper `include/mpapp/<name>.hpp` ctor body ends with `embedded_handler_.map_gestures(*this);` (or, for the no-handler exceptions `application` + `bindable_layout`, no change — they don't have wrappers).
- [ ] `ctest --test-dir build-wsl` still passes (no regressions). Gesture-specific mock tests remain at 19 (no new mock surface; just wiring).
- [ ] At least one example (e.g. `gtk4_hello`) is updated to attach a gesture recognizer to a non-button widget (a label or layout) + manually verified on a real GTK4 display that the gesture fires.
- [ ] Per CLAUDE Rule 11: this task closes only when an end-to-end recording / screenshot demonstrates a tap landing on a non-button widget via the wired-up linux_gestures.

## Recommended approach

Script-driven, three passes:

1. **Generator update**: modify `tools/dev/migrate-component.py`'s `WRAPPER_TEMPLATE` to append `embedded_handler_.map_gestures(*this);` as the last line of every wrapper ctor (regardless of `map_X` vs `bind` style).
2. **Handler sweep**: write `tools/dev/sweep-gesture-map.py` that for each migrated component:
   - Adds the `map_gestures` declaration to each per-platform handler hpp (idempotent — skip if already present).
   - Adds the linux .cpp implementation + the `gesture_attach.hpp` include (idempotent).
3. **Wrapper regen**: `python tools/dev/migrate-component.py --regen-wrapper --all-bucket-a` (plus the bucket-c + bucket-d lists) to rewrite every wrapper with the new template.

Verify after each pass: build the Linux mock_handlers_test + gtk4_hello to confirm no regression.

## Risks

> [!warning]
> - The sweep touches many files; double-check that the per-component `set_handler` name (some components use `set_cv_handler`, `set_sv_handler`, etc. per [[ADR-0024-wrapper-component-pattern]]) is not accidentally overwritten by the `--regen-wrapper` pass.
> - `bind`-style components (layouts) have a different ctor body shape — the template change must preserve that.

## Links

- Prior art: button slice in commit ahead-of-this (look for `button_handler<linux_>::map_gestures` definition).
- Source-of-truth design: [[RFC-0003-gesture-recognizers]].
- First slice: [[T-0033-gesture-recognizers-tap-slice]].
- Follow-up per-platform: T-0038 (Windows), T-0039 (Android), T-0040 (macOS), T-0041 (iOS) — to be opened.
