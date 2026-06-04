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

## Session 2 — MVVM/binding/animation + per-platform backends (verified)

A second autonomous run (25 + 5 + 4 agents across three workflows) added the
framework-richness layer and the per-platform backends, this time with a
**full cross-platform verification harness** discovered mid-session (see
[[host-test-harness]] in agent memory): WSL Ubuntu-24.04 (g++ 14.2 + GTK4 4.14.5
+ WebKitGTK-6.0 + GIO — the project's real CI compiler), the Windows MinGW
toolchain, and the real Android NDK 27.2 clang (arm64 + x86_64). macOS/iOS remain
the only host-gated targets.

**The WSL gold-standard `ctest` (g++ 14.2) caught 3 real undefined-behaviour bugs
the MinGW static-link harness silently tolerated** — all fixed + committed:
`and_multi_trigger` dangling callback (by-value vector reallocation), and
`app_actions`/`email` returning `std::optional` by value while tests bound
`const auto& = *getter()` (dangling into a destroyed temporary). Full mock suite
is **1476 tests / 100%** under g++ 14.2.

- **MVVM / CommunityToolkit.Mvvm:** `observable_object`, `messenger`,
  `observable_validator`, `async_relay_command`.
- **Binding:** fallback/target-null/string-format resolver, typed `property_path`,
  CTK converters (null/bool, value-map, string/color, multi-value + chaining).
- **Animation:** full MAUI easing set, composite/child timeline, repeat/auto-reverse,
  `color_to` + generic Observable animation, fluent extensions + cancel registry.
- **Behaviors/Triggers:** event-to-command, text-validation, compare-state trigger.
- **Real cross-platform backends** (no-ifdef): `real_file_system`,
  `real_version_tracking`, `real_app_info`, `real_main_thread`.
- **Windows Win32 backends** (compile+link under MinGW): clipboard, battery,
  launcher/browser, device_display.
- **Linux GIO/GTK4 backends** (compile-verified in WSL): launcher/browser
  (`g_app_info`), connectivity (`GNetworkMonitor`), clipboard (`GdkClipboard`).
- **Android JNI backends** (cross-compiled arm64+x86_64): clipboard
  (`ClipboardManager`), vibration (`Vibrator`).
- **Real layout handlers** — AbsoluteLayout/FlexLayout were declaration-only headers;
  implemented for **Linux** (`GtkFixed`/`GtkBox`, links into `mpapp-handlers-linux`)
  and **Android** (`FrameLayout`/`LinearLayout` v1, cross-compiled both ABIs).

## What's next (follow-ups)

1. **Windows real layout handlers** (mux::Canvas/custom) — needs MSVC + WinUI
   (MinGW can't build C++/WinRT); plus macOS/iOS layout handlers (Apple host).
   FlexLayout needs a faithful flexbox solver on every platform (current = v1 map).
2. **On-device / runtime** verification — current per-platform gate is compile/link,
   not runtime (headless WSL has no display/DBus; Android/Windows backends not yet
   run on a device). GTK clipboard async reads + Android Context plumbing are runtime TODOs.
3. **Per-platform real backends** for the remaining Essentials APIs (sensors,
   geolocation, battery on Linux via UPower, etc.) + license review (Rule 9) for native deps.
4. **CMake wiring** — the new `src/essentials/{windows,linux,android}/` backends compile
   standalone but are not yet added to a build target; the layout handler `.cpp`s already
   build (globbed). Umbrella (`mpapp.hpp`) still excludes the new layout wrappers.
5. **XAML lowering** of the new surface (brushes, AppThemeBinding, templating, dialogs)
   into `mpapp-xc` — M-09 tooling scope.

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
