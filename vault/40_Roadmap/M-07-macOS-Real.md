---
type: milestone
id: M-07
title: macOS real platform — AppKit handlers (when MacBook self-hosted runner is online)
phase: P6
status: planned
deliverables:
  - AppKit (NOT Catalyst) handlers for every mocked control
  - Self-hosted macOS runner registered
  - Automated UI tests run human-free (designed in M-02 via T-0008)
  - Hot reload on macOS desktop
exitCriteria:
  - "Every Controls Inventory row at mpappStatus: macos-real"
  - "platformMacos: true on every component"
  - "macOS UI test suite green without human intervention"
tags:
  - type/milestone
  - phase/p6
  - status/planned
  - platform/macos
---

# M-07 — macOS Real Platform

> [!info] Status
> **planned**. Blocked on user providing MacBook Pro as self-hosted CI runner.

## Scope

AppKit handlers per [[ADR-0005-ios-macos-separate-interop]] — no Mac Catalyst. The pre-built human-free test harness from M-02 (T-0008 design) is what enables Claude to iterate on this milestone without manual UI clicking.

## Exit Criteria

- [ ] Every component has a working `*_handler<platform::macos>` against AppKit.
- [ ] Every component's `platformMacos: true`.
- [ ] Self-hosted macOS runner registered with GitHub Actions; cloud `macos-latest` jobs migrated to it.
- [ ] AppleScript / Accessibility-API based UI tests run on every PR.
- [ ] Hot reload working on macOS desktop.

## Readiness checklist

The seed `.mm` files under `src/handlers/macos/` already follow the post-[[ADR-0024-wrapper-component-pattern]] structure: every `map_<property>` takes `internal::basic_<name>&` (the surface), and the per-platform handler lives in `mpapp::internal::`. So the wrapper-pattern migration is **not** a blocker — what is missing is purely the Apple host that can compile + run + UI-test the bodies.

Concrete entry conditions, in order:

1. **Mac host available.** Either the user's MacBook Pro or a temporary cloud Mac (`macos-latest`) to validate the existing seed files build at all under Xcode Clang. Today only `release.yml`'s tag-gated `macos-native` job exercises this, and even that does not run automatically per PR.
2. **Self-hosted `mpapp-macos-self` runner registered.** Replaces the cloud `macos-latest` minutes in `release.yml`; once online, a `macos-native` job is added to `pr.yml` (see [[CI Strategy]] §Apple platform timeline).
3. **T-0008 UI-test harness wired in.** AppleScript + accessibility-API recipe must run without human intervention; cost of UI-test failure must be visible in the GitHub Checks UI like any other test.
4. **Component fill-in** — start with the 4 seed components (application, button, label, window) the `.mm` files already have, then bulk-port the rest of the inventory the same way the Bulk Handler Port (M-04b) did for Linux + Android. After [[ADR-0024-wrapper-component-pattern]] this is a 60-component sweep of "extend the existing `.mm` files with the remaining `map_<prop>` methods + add the surface-class casts to `::mpapp::internal::basic_<name>*` in dispatch functions."
5. **Hot reload** — the LLVM-based hot-reload daemon from [[Hot Reload]] needs an AppKit-specific code-injection path.

## Risks

## Reinstate trigger

This milestone moves from `planned` → `active` when **either** an Apple-host self-hosted runner is online **or** the project lead has a Mac in front of them to validate the existing seed `.mm` files. The Cloud `macos-latest` cost (~15 min wall-clock per release tag) is acceptable as a temporary fallback during the validation phase.

## Risks

> [!warning]
> - macOS HIG is stricter than other platforms — sidebars, toolbars, menus require care.
> - Without the human-free test harness, iteration speed collapses. T-0008 from M-02 must be done.

## Tasks

Linked via [[_Bases/Tasks.base]] filtered by `milestone == "M-07"`.

## See in code

- Seed AppKit handlers (Objective-C++ `.mm`): [`src/handlers/macos/`](../../src/handlers/macos/) — `application_handler.mm`, `button_handler.mm`, `label_handler.mm`, `window_handler.mm`. App-shell-only today; per-component fill-in pending an Apple host.
- Distinct from iOS per [[ADR-0005-ios-macos-separate-interop]]: macOS uses AppKit (`NSWindow`, `NSButton`), iOS uses UIKit (`UIWindow`, `UIButton`). No shared "Apple" handler tree.
- Apple-target toolchain files: [`cmake/toolchains/macos-arm64.cmake`](../../cmake/toolchains/macos-arm64.cmake).
- Apple-target cross-compile T-0009 row remains `in-progress` until an Apple host is online — see [`vault/50_Tasks/T-0009-cross-compilation-matrix/`](../50_Tasks/T-0009-cross-compilation-matrix/).
- Test-harness scaffolding (human-free Apple UI tests per T-0008) lands as part of M-02 closure; references gathered in [[Test Harness]].

## Related

- [[ADR-0005-ios-macos-separate-interop]]
- [[Platform Interop]]
- [[70_References/Objective-C++]]
- [[Test Harness]]
- [[Hot Reload]]
