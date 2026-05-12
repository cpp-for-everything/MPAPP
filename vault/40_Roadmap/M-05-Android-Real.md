---
type: milestone
id: M-05
title: Android real platform — fbjni handlers
phase: P4
status: planned
deliverables:
  - fbjni-based handlers for every mocked control
  - mpapp-jni-gen Android codegen tool matured
  - Android emulator CI on self-hosted runner
  - Hot reload on Android emulator
exitCriteria:
  - "Every Controls Inventory row at mpappStatus: android-real"
  - "platformAndroid: true on every component"
  - "Android UI test suite green"
tags:
  - type/milestone
  - phase/p4
  - status/planned
  - platform/android
---

# M-05 — Android Real Platform

> [!info] Status
> **planned**. Starts after [[M-04-Windows-Real]] closes.

## Scope

Convert mock handlers to real fbjni handlers. Mature the `mpapp-jni-gen` codegen tool to produce typed wrappers for the Android API surface MPAPP touches.

## Exit Criteria

- [ ] Every component has a working `*_handler<platform::android>`.
- [ ] Every component's `platformAndroid: true`.
- [ ] `mpapp-jni-gen` generates typed wrappers (no raw `jobject` exposure).
- [ ] Android emulator UI tests on self-hosted runner.
- [ ] Hot reload working on Android emulator.

## Risks

> [!warning]
> - JNI local-ref leaks are easy. Every codegen output uses RAII `local_ref<T>` / `global_ref<T>` from fbjni — never raw `jobject`.
> - Android emulator performance on Windows host varies; self-hosted runner sizing matters.

## Tasks

Linked via [[_Bases/Tasks.base]] filtered by `milestone == "M-05"`.

## Related

- [[Platform Interop]]
- [[70_References/fbjni]]
- [[Hot Reload]]
