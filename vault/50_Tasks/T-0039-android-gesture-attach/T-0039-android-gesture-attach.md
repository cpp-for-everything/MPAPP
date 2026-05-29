---
type: task
id: T-0039
title: Android real gesture wire-up — android_gestures::attach over JNI + android.view.GestureDetector
status: in-progress
milestone: M-05
owner: ""
area: handlers
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/handlers
  - phase/p5
  - platform/android
---

# T-0039 — Android real gesture wire-up

## Goal

Android analog of `linux_gestures::attach`: install JNI-bridged native gesture listeners on the jobject that the per-component handler owns. Replaces the no-op `map_gestures` stubs landed in commit `b0a999d`.

## Per-recognizer wire-up table

| Recognizer | Android API | JNI shape |
|---|---|---|
| `tap` | `android.view.GestureDetector.SimpleOnGestureListener::onSingleTapConfirmed` / `onDoubleTap` | Java-side `MppGestureRouter` (analog of `MppClickRouter`) instantiated with `long buttonPtr`, calls `nativeDispatchTap(long, double, double, int)`. |
| `pan` | `View.OnTouchListener` + manual delta tracking from `MotionEvent.ACTION_DOWN/MOVE/UP` | `MppGestureRouter::nativeDispatchPan(long, int phase, int gestureId, double tx, double ty)`. |
| `pinch` | `android.view.ScaleGestureDetector.SimpleOnScaleGestureListener::onScale` | `nativeDispatchPinch(long, int phase, double scale, double ox, double oy)`. |
| `swipe` | `GestureDetector.SimpleOnGestureListener::onFling` — derive direction from velocityX/Y. | `nativeDispatchSwipe(long, int dirBits)`. |
| `pointer` | `View.OnHoverListener` for enter/exit/move on API 14+ devices with pointer support; touch events on touch-only devices. | `nativeDispatchPointer*` family — 5 functions, one per signal. |

## Acceptance Criteria

- [x] `include/mpapp/handlers/android/gesture_attach.hpp` + `src/handlers/android/gesture_attach.cpp` implement the C++ side of **tap** (commit `0486df9`). Pan / pinch / swipe / pointer remain follow-up.
- [x] Java glue: `examples/android_hello/app/src/main/java/io/mpapp/MppGestureRouter.java` — mirrors `MppClickRouter`'s shape per [[ADR-0022-android-kind-discriminated-routers]].
- [x] Per-component `map_gestures` stubs replaced with calls into `android_gestures::attach` (label + button).
- [ ] Rule-11 closure: recording of a tap on a non-button widget on a real Android device or emulator (the self-hosted `mpapp-windows-self` runner already has an Android emulator slot — see [[CI Strategy]]).

## Implemented (tap — commit `0486df9`)

`android_gestures::attach(jobject, view&)` walks `v.gesture_recognizers`; for each `gesture_kind::tap` it calls `View.setClickable(true)` and installs an `io.mpapp.MppGestureRouter` (a `View.OnClickListener`) constructed with `reinterpret_cast<jlong>(&tap)`. The router's `onClick` JNI-dispatches `Java_io_mpapp_MppGestureRouter_nativeDispatchTap(JNIEnv*, jclass, jlong)`, which `reinterpret_cast`s the recognizer pointer back and `emit`s its `tapped` signal. Same kind-discriminated-router pattern as `MppClickRouter` (ADR-0022). The Java class is resolved via `FindClass("io/mpapp/MppGestureRouter")` at runtime, so the native TU cross-compiles without it. Auto-globbed into the Android native build (`examples/android_hello/.../cpp/CMakeLists.txt` `GLOB src/handlers/android/*.cpp`).

**Verification:** all four touched TUs (new `gesture_attach.cpp` + modified `button_handler.cpp` / `label_handler.cpp`) syntax-clean on both NDK r27 ABIs (`aarch64-linux-android28` + `x86_64-linux-android28`, `-std=c++2b`). On-device tap recording (Rule-11 gate) deferred — host PC in use; emulator route via `tools/dev/android-e2e.ps1`. This closes the **tap** half; the full 5-recognizer port stays the ticket's open scope.

## Notes

The kind-discriminated router pattern from ADR-0022 is the right home for this — `MppGestureRouter` is one new router in the family alongside `MppClickRouter`, `MppItemClickRouter`, `MppTextWatcher`, etc. Gradle build wires + `META-INF` jar packaging follow the existing convention.

## Links

- RFC: [[RFC-0003-gesture-recognizers]] §Detailed Design.
- ADR governing the Java glue shape: [[ADR-0022-android-kind-discriminated-routers]].
- Linux precedent: [[T-0037-linux-gesture-bulk-port]].
