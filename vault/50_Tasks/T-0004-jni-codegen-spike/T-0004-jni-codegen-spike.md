---
type: task
id: T-0004
title: Android JNI typed wrapper codegen spike
status: todo
milestone: M-01
owner: ""
area: handlers
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/handlers
  - platform/android
  - phase/p0
---

# T-0004 — Android JNI codegen spike

## Goal

Prove the fbjni-based interop strategy by:

1. Standing up fbjni in the build.
2. Writing a tiny `mpapp-jni-gen` prototype that emits a typed wrapper for `android.widget.Button` from a description input.
3. Using the generated wrapper to construct a Button and set its text from C++ running on the Android emulator.
4. Verifying RAII: `global_ref<T>` and `local_ref<T>` handle the JNI object lifecycle correctly, no leaks under repeated set/get.

## Acceptance Criteria

- [ ] fbjni integrated into CMake build (Android target).
- [ ] `tools/mpapp-jni-gen/` produces `src/handlers/android/generated/android_widget_button.hpp` from a YAML/JSON description.
- [ ] Generated wrapper exposes typed methods (e.g. `void set_text(jni::local_ref<jstring>)`).
- [ ] Spike program runs on Android emulator (via self-hosted runner from T-0006).
- [ ] Long-loop test (10000 iterations of set/get) shows no JNI ref leaks via `adb shell dumpsys meminfo`.
- [ ] Screen recording of emulator showing button text changing in `recordings/`.
- [ ] Logs in `logs/`.
- [ ] 100% coverage on the generated wrapper + the spike harness.

## Notes

This is the second-riskiest spike. JNI lifetime bugs are subtle — make the test harness as adversarial as possible.

## Links

- Milestone: [[M-01-Foundations]]
- Related: [[Handlers]], [[Platform Interop]], [[70_References/fbjni]]
