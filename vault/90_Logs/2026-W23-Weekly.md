---
type: log
week: 2026-W23
date: 2026-06-04
tags:
  - type/log
---

# 2026-W23 — Weekly log

## What happened — Tier-2/3 MAUI-parity brute-force push

A single autonomous multi-agent run (34 agents, two waves) closed the bulk of the
**Tier-2 (specific missing primitives)** and **Tier-3 (breadth)** gaps identified in
the MAUI-parity audit. Everything landed **mock-first** ([[ADR-0008-mock-first-implementation]],
Rule 6) and was verified by compiling **and running** the Catch2 tests on the host
harness (see [[host-test-harness|host test harness]] note in agent memory).

### Verification

- Per-agent: each agent compiled + ran its own `*_test.cpp` green before returning.
- Integration: all **36 new test files compiled together into one binary** and ran
  — **683 test cases / 2016 assertions, all passed** — proving no ODR / symbol /
  duplicate-name clashes across the new surface.
- Self-containment: every new header compiles standalone (one fix — `sms.hpp` was
  missing `<stdexcept>`).
- Regression: a broad sample of existing component tests (button / label / box_view /
  border / grid_layout / content_view / page / application / navigation_page) still
  compiles clean after the shared-header edits.
- Portability fix: `src/hot_reload/windows.cpp` had an unconditional `#define NOMINMAX`
  → guarded with `#ifndef` (MinGW `-Werror` clash; MSVC unaffected).

### Tier 2 — specific primitives

- **[[Components/AbsoluteLayout|AbsoluteLayout]]** — full grid-mirrored component
  (surface + wrapper + mock + Win/Linux/Android blind handlers + 13 tests). `layout_bounds`
  rect + `layout_flags` proportional bitmask attached store.
- **[[Components/FlexLayout|FlexLayout]]** — full grid-mirrored component (11 tests).
  Container flow props + per-child order/grow/shrink/basis/align_self attached store.
- **Gradient brushes** — `solid_color_brush` / `linear_gradient_brush` /
  `radial_gradient_brush` / `gradient_stop` (`include/mpapp/brushes/brush.hpp`),
  integrated into the `view` surface as `background_brush`, plus `shadow` and `clip`.
- **Page dialog services** — `display_alert` / `display_action_sheet` / `display_prompt`
  (`include/mpapp/dialogs.hpp`; deterministic recorded-request + programmable-response mock).
- **Modal navigation** — `push_modal` / `pop_modal` stack engine
  (`include/mpapp/detail/modal_stack.hpp`) mirroring the page-stack ([[ADR-0014-page-navigation-stack]]).
- **AppThemeBinding + Application.RequestedTheme** — light/dark theming
  (`include/mpapp/theme/app_theme_binding.hpp`; `app_theme` enum from Essentials `app_info`).
- **Templating** — `data_template_selector`, `control_template`, `content_presenter`
  (`include/mpapp/templates/`).

### Tier 3 — Essentials device APIs (4 → ~37)

New `include/mpapp/essentials/` headers, each interface + test-drivable in-memory mock,
following the RFC-0013 pattern: **sensors** (accelerometer, gyroscope, magnetometer,
barometer, compass, orientation_sensor + `sensors_common`), **battery**, **device_display**,
**app_info** (+`app_theme`), **version_tracking**, **main_thread**, **clipboard**,
**flashlight**, **vibration**, **haptic_feedback**, **share**, **launcher**, **browser**,
**email**, **sms**, **phone_dialer**, **file_picker**, **media_picker**, **file_system**,
**text_to_speech**, **permissions**, **geolocation**, **geocoding**, **contacts**,
**screenshot**, **web_authenticator**, **app_actions**.

## What's next (follow-ups this push created)

1. **Per-platform real handlers** for AbsoluteLayout / FlexLayout — the Win/Linux/Android
   handlers are written blind and need verification on each host; FlexLayout additionally
   needs a faithful flexbox arrange engine. macOS/iOS unwritten (pending Apple host).
2. **Per-platform real backends** for the new Essentials APIs (the original 4 are also
   mostly in-memory; real backends are per-platform follow-ups + license review per Rule 9
   for any new native deps).
3. **Umbrella wiring** — deliberately NOT added to `include/mpapp/mpapp.hpp` yet, to avoid
   coupling the unverified per-platform layout handlers into the default include. Wire in
   once a host verifies them.
4. **XAML lowering** of the new surface (brushes, AppThemeBinding, templating, dialogs)
   into `mpapp-xc` — M-09 tooling scope.
5. **Coverage + per-component docs** for AbsoluteLayout/FlexLayout reach Rule-11 closure
   only after real handlers + screenshots on a host.

## Notes

- Parallelism model: disjoint-file ownership (new files are glob-collected by
  `tests/CMakeLists.txt` and `cmake/MpappHandlers.cmake`; Essentials are header-only with
  no registration), so 31 Wave-1 agents touched disjoint files with zero merge conflicts;
  the 3 shared headers (`view.hpp`, `page.hpp`/`application.hpp`, theme) were each edited by
  exactly one Wave-2 owner. No worktrees needed.

## Related

- [[00_Index/Current Focus]]
- [[10_Architecture/Controls Inventory]]
- [[ADR-0008-mock-first-implementation]] · [[ADR-0024-wrapper-component-pattern]]
- [[30_RFCs/RFC-0013-essentials]]
