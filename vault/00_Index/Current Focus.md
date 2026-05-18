---
type: moc
tags:
  - type/moc
---

# Current Focus

> [!important] Status — 2026-W20 (late)
> **Phases P3 / P4 / P5 active in parallel.** [[M-02-Infrastructure]] closed; [[M-03-Mock-Surface]] effectively done; M-04+ work running across three platforms simultaneously.
>
> **Just-completed**: [[_Archive/T-0011-app-shell-abstraction|T-0011]] app-shell abstraction landed and **live-verified on three platforms** — Windows (WinUI 3, Count: 0 → 7), Linux (GTK4 via WSLg, Count: 0 → 5), and Android (NDK r26 emulator, Count: 0 → 7). Same C++ source on all three; only the handler template arguments differ. macOS + iOS handlers code-complete pending an Apple host. [[_Archive/T-0007-wslg-gtk4-hello|T-0007]] (WSLg) closed in the same batch since it was unblocked organically during the live Linux verification. [[ADR-0012-application-window-handler-abstraction]] promoted to **accepted**.
>
> 126/126 mock-handler tests passing. Five components (Application / Window / StackLayout / Button / Label) now ship as `android-real` (Windows + Linux + Android verified live).

## This week (2026-W20)

- [x] Spike [[_Archive/T-0011-app-shell-abstraction|T-0011]] mock surface — `application`, `window`, `page`, `stack_layout`, `grid_layout` mock handlers + lifecycle tests.
- [x] WinUI 3 real handlers for `application` / `window` / `stack_layout`.
- [x] Rewrite `examples/windows_button_spike/main.cpp` against the new surface — zero `winrt::`/`mux::`/`muxc::`/`Mdd*` tokens in user-facing code.
- [x] Promote [[ADR-0012-application-window-handler-abstraction]] proposed → accepted.
- [x] Resolve [[_Archive/T-0007-wslg-gtk4-hello|T-0007]] WSL-install blocker (Ubuntu-24.04 on this host; GTK4 dev packages installed; cross-platform `gtk4_hello` rewrite verified end-to-end via WSLg).
- [x] **GTK4 (Linux) real handlers** — application / window / stack_layout / button / label.
- [x] **Android (JNI) real handlers** — Activity bridge + LinearLayout / Button / TextView via JNI, click routed back via `MppClickRouter`.
- [x] **AppKit + UIKit handlers** — code-complete (Objective-C++ `.mm`, no Apple host to verify yet).

## Next up (P3 / P4 / P5 expansion)

The app-shell layer proves the handler pattern works across five platforms. The next batches port more of MAUI's control surface to the same three-platform-real bar:

- **CollectionView, Entry, Editor, Switch, Slider, Stepper, CheckBox, RadioButton** — already have mock handlers (Unit 8 in batch 3). Promote each to `windows-real` + `linux-real` + `android-real` per the T-0011 template.
- **Image, ImageButton** — mock surface first, then real on three platforms.
- **Page navigation** — mock `Page` exists; needs the real handlers on Windows/Linux/Android. WinUI 3 will use `muxc::Frame`; GTK4 will use `GtkStack`; Android will use `Fragment`.
- **Grid layout track definitions** — `grid_layout` mock has row/col counts; needs the full star/auto/abs sizing surface + per-child placement.
- **Hot reload on Linux + Android** — [[_Archive/T-0010-hot-reload-spike|T-0010]] proved the LLVM hot-reload approach on Windows; same shape on Linux (`dlopen`/`dlclose` of `.so`) and Android (Activity `dlopen` of bundled `.so`).

## Active milestone

[[M-03-Mock-Surface]] (effectively complete for the app-shell + 16 layout/input controls) and **M-04** (Windows real platform expansion) + **M-05** (Android real platform expansion) running in parallel. [[M-06-macOS]] gated on a self-hosted Mac runner.

## Recently accepted

- [[ADR-0012-application-window-handler-abstraction]] (2026-05-18, **accepted**) — extends the widget-handler pattern to Application / Window / Page / Layout. Proved live on 3 platforms by [[_Archive/T-0011-app-shell-abstraction|T-0011]].
- [[ADR-0011-cross-compilation-toolchain]] (2026-05-12, accepted) — Zig (`zig cc`) is the primary cross-compilation toolchain; closes [[RFC-0002-cross-compilation-toolchain]].
- [[ADR-0010-licensing-and-patent-strategy]] (2026-05-12, accepted) — Apache 2.0 + commercial dual license, Apache-style CLA, LGPL-dynamic-only deps, deferred patent filing; closes [[RFC-0001-licensing-and-patent-strategy]].
- [[ADR-0009-public-api-template-wrappers-only]] — template wrapper types only; option B (attributes) rejected.
- [[ADR-0008-mock-first-implementation]] — mock-first strategy.
- (See [[Decision Log]] for the full day-1 burst.)

## Active tasks

- [[T-0004-jni-codegen-spike]] — Android JNI codegen with `fbjni`. T-0011 verified the *manual* JNI-bridge pattern works end-to-end; the codegen tool now has a known-good target to generate against.
- [[T-0009-cross-compilation-matrix]] — Windows host validation still at 4/6 (Android + iOS blocked by Zig 0.13 SDK gaps). Linux + Android targets are NOW provably buildable via the alternate paths (WSL native + Android NDK), which informs the matrix decision.

## Pinned reading

- [[CLAUDE]] — vault rules.
- [[Type System]] — the template-wrapper-type design.
- [[Build System]] — cross-compilation matrix.
- [[dotnet-maui-deep-dive]] — the spec MPAPP mirrors.
- [[_Archive/T-0011-app-shell-abstraction/screenshots/evidence|T-0011 live-verification evidence]] — the canonical reference for what "3-platform-real" looks like in practice.
