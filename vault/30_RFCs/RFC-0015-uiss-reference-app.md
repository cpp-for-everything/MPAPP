---
type: rfc
id: RFC-0015
title: УИСС — cross-platform reference application (TU-Sofia "Е-Студент")
status: draft
author: Alex Tsvetanov
created: 2026-05-29
area: widgets
relatedADRs:
  - ADR-0024
  - ADR-0012
  - ADR-0014
---

# RFC-0015 — УИСС reference application

> [!info] Status
> **draft** — first vertical slice landed under [[T-0060-uiss-reference-app]]: `examples/uiss` **builds on all three targets** (Linux/GTK4 builds + runs; Windows/WinUI 3 `uiss.exe` linked; Android NDK cross-compiles both ABIs). On-screen run/screenshots deferred (host PC in use). This is the framework's first *real application*, not a single-widget spike.

## Problem

Every MPAPP example so far is a single-widget or single-subsystem spike (`gtk4_flyout_demo`, `windows_vsm_demo`, …). Nothing exercises the whole stack the way a real product does: multi-page navigation, a login gate, a dozen data-bound screens, scrolling tables, all composed from **one ifdef-free source tree** that compiles to every target. Without such an app we cannot honestly claim MAUI parity, and we have no grounded signal for *which* remaining gaps actually matter to app authors.

The chosen target is the **TU-Sofia "Е-Студент" student portal** (УИСС — Университетска Информационна Система за Студенти). Its real screens are captured under `examples/УИСС/*.mhtml` and serve as the design spec.

## Proposal

`examples/uiss/` — a single `mpapp::application` replicating the portal:

- **Login** (faculty number + ЕГН/national-id) gates the app.
- A **`flyout_page`** root: the ≡ flyout pane is the nav menu (the 10 portal sections + logout, built once); the `detail` slot swaps between the login page and the 10 section pages.
- **Ten section pages** — Информация, Здравно осигуряване, Заверки и оценки, Спорт, Стипендии, Общежития, Плащания, Идентификация, История на влизанията, Помощна информация — each a scrollable `page` bound to one in-memory `uiss::student` record transcribed from the references.

Single codebase, no `#ifdef`. `mpapp::run<uiss_app>` binds the platform handler set (WinUI 3 / GTK4 / Android NDK) at link time.

## Detailed design

Files (`examples/uiss/`):

- `uiss/data.hpp` — value structs (`attribute`, `grade_group`, `login_row`, `help_topic`, `student`) + `seed()` with the real portal data. Pure data; drives all three targets.
- `uiss/support.hpp` — composition helpers that stay 100% on the public surface:
  - `non_owning(view&)` → a non-owning aliasing `shared_ptr<view>` for `ScrollView.content`.
  - `box` — bundles `basic_stack_layout` + its handler; `add()` children then `done()` binds the native container once (works around the wrapper auto-bind running at ctor time when `child_count()==0`).
  - `label_list` — owns heap `mpapp::label`s for data-driven rows (leaf wrappers auto-bind).
  - `click_button` — a `button` + `signal_slot` + a `std::function` action.
- `uiss/pages.hpp` — reusable `section_page` (header bar with a ≡ toggle + title over a scrollable column), the `login_page`, and the ten per-section content builders.
- `uiss/app.hpp` — `uiss_app : application`; owns the window, flyout, login, nav menu and the 10 section pages; wires login/navigate/logout.
- `main.cpp` — `mpapp::run<uiss::uiss_app>`.

Navigation is just Observable mutation: `fp_.detail = &sections_[i].page` live-swaps the detail (the flyout handler's `map_detail` subscribes to the change). Login flips `is_presented` to reveal the menu.

### Composition pattern confirmed

- **Leaf wrappers** (`label`/`button`/`entry`/`scroll_view`/`page`/`flyout_page`/`window`) auto-bind their handler in the ctor → app code is `x.prop = …`.
- **Layouts** need an explicit handler + `bind()` *after* `add()` (children are added post-construction) — encapsulated in `box`.
- Cyrillic literals require UTF-8 source: `-finput-charset/-fexec-charset=UTF-8` (GCC/Clang) and `/utf-8` (MSVC).

## Gaps surfaced (this app is a gap detector)

Building a real screen immediately exposed framework limits worth their own tickets:

1. **`label` has no font/color/weight** — surface is `text`-only. Headings, the blue portal chrome, bold keys all render plain. → motivates a styled-text surface + RFC-0012 fonts wiring.
2. **No table/grid-of-data widget ergonomics** — the grades/history/attribute tables are rendered as flat label rows; `CollectionView`/`TableView` typed-item templates would be the idiomatic path (and exercise virtualization).
3. **Image loading** — the TU logo needs a real file/URI image loader (RFC-0004 per-platform loaders, T-0045+).
4. **`picker`** — the Плащания payment-kind dropdown wants a real `picker` (currently listed as text).
5. **Styling/resources** — RFC-0005 styles aren't applied for app-wide theming yet.

These become the prioritized backlog: the app tells us what to harden next.

## Alternatives

- **TabbedPage instead of FlyoutPage.** Rejected for the root — the portal's ≡ hamburger is a flyout drawer; `flyout_page` matches and is responsive (split on desktop, drawer on mobile).
- **One giant page with sections.** Rejected — defeats the navigation exercise; the portal is genuinely multi-page.
- **Cyrillic directory name `examples/УИСС` for the build target.** Rejected — kept ASCII `examples/uiss` for CMake target + cross-toolchain path safety (Android Gradle, Windows). The Cyrillic folder holds the `.mhtml` references only.

## Open Questions

> [!todo] Open
> - [ ] Windows/WinUI + Android/NDK run verification (build green is the first gate).
> - [ ] Upgrade the data tables to `CollectionView` typed items once that path is ergonomic.
> - [ ] Apply RFC-0005 styles for the portal's blue chrome once `label` gains font/color.
> - [ ] TU logo via the RFC-0004 image loaders.

## Migration / Compatibility

Pure addition under `examples/`. No framework surface changed. Platform-gated in `examples/CMakeLists.txt` (GTK4 on Linux, WinUI 3 on Windows; Android via its own NDK scaffolding).

## References

- `examples/УИСС/*.mhtml` — the design spec (saved portal screens).
- [[ADR-0024-wrapper-component-pattern]] (the wrapper/surface split the app composes on), [[ADR-0012-application-window-handler-abstraction]], [[ADR-0014-page-navigation-stack]].
- [[RFC-0004-image-source-family]], [[RFC-0005-resource-dictionaries-and-styling]], [[RFC-0012-fonts]] — the gaps this app motivates hardening.
