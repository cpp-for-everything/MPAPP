// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android wiring for the RFC-0003 gesture-recognizer family.
//
// Per-component Android handlers call `attach(view_obj, view)` after creating
// their native `android.view.View` so each recognizer in
// `view.gesture_recognizers` gets the matching native listener installed.
// Tap is wired via `View.setOnClickListener` + the Java `io.mpapp
// .MppGestureRouter` shim, which JNI-dispatches back to the recognizer
// (mirrors the MppClickRouter pattern used by the button handler). Pan /
// pinch / swipe / pointer (OnTouchListener + ScaleGestureDetector) are a
// follow-up. Mirrors the GTK4 `linux_gestures::attach` contract.

#ifndef MPAPP_HANDLERS_ANDROID_GESTURE_ATTACH_HPP
#define MPAPP_HANDLERS_ANDROID_GESTURE_ATTACH_HPP

#include "../../platform.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {
class view;
} // namespace mpapp

namespace mpapp::internal::android_gestures {

// Walk `v.gesture_recognizers` and install the matching native listener on
// `view_obj` (a global/local ref to an android.view.View). Invoke once per
// view setup.
void attach(jobject view_obj, view& v);

} // namespace mpapp::internal::android_gestures

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_GESTURE_ATTACH_HPP
