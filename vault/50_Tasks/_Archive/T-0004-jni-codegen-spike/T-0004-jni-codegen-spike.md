---
type: task
id: T-0004
title: Android JNI typed wrapper codegen spike
status: abandoned
milestone: M-01
owner: ""
area: handlers
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/abandoned
  - area/handlers
  - platform/android
  - phase/p0
---

# T-0004 — Android JNI codegen spike

> [!warning] Abandoned (2026-05-24)
> The premise — "prove the fbjni-based interop strategy works" — was
> bypassed implicitly during the M-04 / M-04b / M-04c handler push. Every
> Android handler that shipped (button, label, list_view, collection_view,
> table_view, web_view, hybrid_web_view, shape_view, graphics_view, the
> cell tree, page family, Shell, layouts, the ADR-0022 router family,
> etc.) uses **raw JNI directly**, with hand-rolled per-class helper
> functions in `src/handlers/android/`. No fbjni dependency was added,
> no `mpapp-jni-gen` was written, no generated wrappers exist.
>
> The research question this spike would have answered ("can raw JNI
> be ergonomic enough without a codegen layer?") was answered "yes" by
> the shipped surface. Closing as abandoned rather than done so the
> outcome is unambiguous — none of the original acceptance criteria
> were met.
>
> If a future maintainer decides the raw-JNI helper pattern has become
> too repetitive and a codegen pass would pay for itself, the right
> action is a **new** task (T-NNNN), not resurrecting this one.

## Original goal

Prove the fbjni-based interop strategy by:

1. Standing up fbjni in the build.
2. Writing a tiny `mpapp-jni-gen` prototype that emits a typed wrapper for `android.widget.Button` from a description input.
3. Using the generated wrapper to construct a Button and set its text from C++ running on the Android emulator.
4. Verifying RAII: `global_ref<T>` and `local_ref<T>` handle the JNI object lifecycle correctly, no leaks under repeated set/get.

## Original acceptance criteria

(All unmet — see abandonment callout above.)

- [ ] fbjni integrated into CMake build (Android target).
- [ ] `tools/mpapp-jni-gen/` produces `src/handlers/android/generated/android_widget_button.hpp` from a YAML/JSON description.
- [ ] Generated wrapper exposes typed methods (e.g. `void set_text(jni::local_ref<jstring>)`).
- [ ] Spike program runs on Android emulator (via self-hosted runner from T-0006).
- [ ] Long-loop test (10000 iterations of set/get) shows no JNI ref leaks via `adb shell dumpsys meminfo`.
- [ ] Screen recording of emulator showing button text changing in `recordings/`.
- [ ] Logs in `logs/`.
- [ ] 100% coverage on the generated wrapper + the spike harness.

## Notes

This was originally the second-riskiest spike (JNI lifetime). In practice the risk was managed cell-by-cell as each handler shipped — every `make_object` / `attach_current_thread` / global-ref / local-ref pattern lives in plain C++ and is reviewed per-PR rather than centralized through a codegen pipeline. The handler files themselves are the de-facto JNI reference.

## Links

- Milestone: [[M-01-Foundations]]
- Related: [[Handlers]], [[Platform Interop]], [[70_References/fbjni]]
- Living JNI reference (in lieu of codegen): every file under `src/handlers/android/`.
