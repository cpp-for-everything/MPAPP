---
type: task
id: T-0039
title: Android real gesture wire-up — android_gestures::attach over JNI + android.view.GestureDetector
status: todo
milestone: M-05
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

- [ ] `include/mpapp/handlers/android/gesture_attach.hpp` + `src/handlers/android/gesture_attach.cpp` implement the C++ side of all 5 recognizers.
- [ ] Java glue: `examples/android_hello/app/src/main/java/io/mpapp/MppGestureRouter.java` — mirrors `MppClickRouter`'s shape per [[ADR-0022-android-kind-discriminated-routers]].
- [ ] Per-component `map_gestures` stubs replaced with calls into `android_gestures::attach`.
- [ ] Rule-11 closure: recording of a tap on a non-button widget on a real Android device or emulator (the self-hosted `mpapp-windows-self` runner already has an Android emulator slot — see [[CI Strategy]]).

## Notes

The kind-discriminated router pattern from ADR-0022 is the right home for this — `MppGestureRouter` is one new router in the family alongside `MppClickRouter`, `MppItemClickRouter`, `MppTextWatcher`, etc. Gradle build wires + `META-INF` jar packaging follow the existing convention.

## Links

- RFC: [[RFC-0003-gesture-recognizers]] §Detailed Design.
- ADR governing the Java glue shape: [[ADR-0022-android-kind-discriminated-routers]].
- Linux precedent: [[T-0037-linux-gesture-bulk-port]].
