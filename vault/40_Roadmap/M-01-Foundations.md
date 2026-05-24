---
type: milestone
id: M-01
title: Foundations — research, decisions, design doc
phase: P0
status: active
deliverables:
  - All Phase 0 ADRs locked
  - Comparative research notes complete for MAUI, Qt, Slint, JUCE, Avalonia, ImGui, Flutter, Sciter, wxWidgets
  - MAUI component inventory populated across all 55 controls
  - Three Phase 0 spikes complete (libclang attribute parsing — superseded, WinUI handler, JNI codegen, **plus template-wrapper-type spike**)
  - 20–30 page design doc
exitCriteria:
  - "Every 10_Architecture/* stub has substantive content"
  - "All 9 day-1 ADRs accepted; RFC-0001 and RFC-0002 closed"
  - "Three Phase 0 spikes pass"
  - "Controls Inventory has porting status for every row"
  - "Design doc reviewed"
tags:
  - type/milestone
  - phase/p0
  - status/active
---

# M-01 — Foundations

> [!success] Status
> **active** — kicked off 2026-05-12.

## Scope

Get the project to a state where we can start building real code with confidence. Research everything, lock in the decisions that frame the design, validate the riskiest pieces with focused spikes, and produce a design document that captures the architecture in one place.

## Exit Criteria

- [ ] Every `10_Architecture/*` stub has substantive content (not just placeholder paragraphs).
- [ ] All 9 day-1 ADRs accepted (ADR-0001 through ADR-0009 — done).
- [ ] [[RFC-0001-licensing-and-patent-strategy]] closed (decided → new ADR).
- [ ] [[RFC-0002-cross-compilation-toolchain]] closed (decided → new ADR).
- [ ] Three Phase 0 spikes pass:
  - [ ] [[T-0002-template-type-spike]] — `Observable<T>` / `Computed<...>` / `Command<>` on a sample VM.
  - [ ] [[T-0003-winui3-button-spike]] — WinUI 3 button handler.
  - [ ] [[T-0004-jni-codegen-spike]] — Android JNI typed wrapper generation.
- [ ] [[T-0005-inventory-maui-controls]] — every row of [[Controls Inventory]] has substantive content in `Components/<Name>.md`.
- [ ] [[T-0007-wslg-gtk4-hello]] — Linux GTK4 build from WSLg green.
- [ ] [[T-0008-mac-ios-test-harness-design]] — design (not implement) the human-free Apple test harness.
- [ ] Design doc reviewed and accepted.

## Risks

> [!warning] Top risks
> - Meta-compiler is the bus factor — but [[ADR-0009-public-api-template-wrappers-only]] removes most of that risk by avoiding a property-system meta-compiler.
> - C++26 reflection schedule outside our control — mitigated by [[ADR-0001-cpp-standard-baseline]] making it opt-in.
> - License contagion — [[RFC-0001-licensing-and-patent-strategy]] addresses this in P0.

## Tasks

Linked via [[_Bases/Tasks.base]] filtered by `milestone == "M-01"`.

## See in code

- All accepted ADRs in [`vault/20_ADRs/`](../20_ADRs/) (every one of ADR-0001 through ADR-0010 ships from this milestone).
- Closed RFCs in [`vault/30_RFCs/`](../30_RFCs/) (RFC-0001 → ADR-0010; RFC-0002 → ADR-0011).
- The three foundational spikes that closed this milestone, all archived:
  - [`vault/50_Tasks/_Archive/T-0002-template-type-spike/`](../50_Tasks/_Archive/T-0002-template-type-spike/) + the prototype at [`tests/template_type_spike/`](../../tests/template_type_spike/).
  - [`vault/50_Tasks/_Archive/T-0003-winui3-button-spike/`](../50_Tasks/_Archive/T-0003-winui3-button-spike/) + the example at [`examples/windows_button_spike/`](../../examples/windows_button_spike/).
  - [`vault/50_Tasks/_Archive/T-0004-jni-codegen-spike/`](../50_Tasks/_Archive/T-0004-jni-codegen-spike/) (abandoned in favor of fbjni; see archive notes).
- Component inventory: [[Controls Inventory]] populated from [`include/mpapp/`](../../include/mpapp/) headers in M-03.

## Related

- All ADRs in `20_ADRs/`
- All RFCs in `30_RFCs/`
- [[Controls Inventory]]
- [[CLAUDE]]
