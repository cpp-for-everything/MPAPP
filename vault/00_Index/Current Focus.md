---
type: moc
tags:
  - type/moc
---

# Current Focus

> [!important] Status — 2026-W20
> **Phase P2 active. Milestones [[M-02-Infrastructure]] and [[M-03-Mock-Surface]] both in flight.**
>
> Just-completed (batch 3, merged 2026-05-18): mock layout + input handler groups (16 components, ~60 tests), async-executor skeleton, mpapp CLI real `build` + `xaml-compile`, Zig toolchain files for 5 targets, T-0009 cross-comp matrix validated (4/6 from Windows host), [[ADR-0010-licensing-and-patent-strategy]] + [[ADR-0011-cross-compilation-toolchain]] accepted. 109/109 tests green.
>
> Newly identified gap: the **app-shell layer** is unabstracted — `examples/windows_button_spike/main.cpp` carries ~25 raw `winrt::`/`mux::`/`muxc::`/`Mdd*` tokens. [[T-0011-app-shell-abstraction]] (filed 2026-05-18) closes this; [[ADR-0012-application-window-handler-abstraction]] captures the pattern (proposed).
>
> Next: T-0011 spike (Application / Window / Page / StackLayout / Grid handlers) → rewritten spike with zero WinRT tokens in user-facing code.

## This week (2026-W20)

Updates here weekly. Don't add dates further out than this week.

- [ ] Spike [[T-0011-app-shell-abstraction]] mock surface — `application`, `window`, `page`, `stack_layout`, `grid_layout` mock handlers + lifecycle tests.
- [ ] WinUI 3 real handlers for `application` / `window` / `stack_layout`.
- [ ] Rewrite `examples/windows_button_spike/main.cpp` against the new surface — zero `winrt::`/`mux::`/`muxc::`/`Mdd*` tokens in user-facing code.
- [ ] Promote [[ADR-0012-application-window-handler-abstraction]] proposed → accepted once the rewritten spike builds + runs.
- [ ] Resolve [[T-0007-wslg-gtk4-hello]] WSL-install blocker (and re-run the matrix on a host with WSL2).

## Active milestone

[[M-02-Infrastructure]] (closing) and [[M-03-Mock-Surface]] (active) — exit criteria, risks, and linked tasks live there.

## Recently accepted / proposed

- [[ADR-0012-application-window-handler-abstraction]] (2026-05-18, **proposed**) — extends the widget-handler pattern to Application / Window / Page / Layout; gates [[T-0011-app-shell-abstraction]].
- [[ADR-0011-cross-compilation-toolchain]] (2026-05-12, accepted) — Zig (`zig cc`) is the primary cross-compilation toolchain; closes [[RFC-0002-cross-compilation-toolchain]].
- [[ADR-0010-licensing-and-patent-strategy]] (2026-05-12, accepted) — Apache 2.0 + commercial dual license, Apache-style CLA, LGPL-dynamic-only deps, deferred patent filing; closes [[RFC-0001-licensing-and-patent-strategy]].
- [[ADR-0009-public-api-template-wrappers-only]] — template wrapper types only; option B (attributes) rejected.
- [[ADR-0008-mock-first-implementation]] — mock-first strategy.
- (See [[Decision Log]] for the full day-1 burst.)

## Pinned reading

- [[CLAUDE]] — vault rules.
- [[Type System]] — the template-wrapper-type design.
- [[Build System]] — cross-compilation matrix.
- [[dotnet-maui-deep-dive]] — the spec MPAPP mirrors.
