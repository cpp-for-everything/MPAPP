// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. RFC-0003 — Android tap gesture router.
//
// Bridges android.view.View.OnClickListener back into the native
// mpapp::tap_gesture_recognizer.tapped signal. The android gesture
// attach helper makes the View clickable and installs an instance of
// this class as the listener, passing the jlong native recognizer
// pointer as the routing key. Mirrors MppClickRouter, but routes to a
// recognizer rather than a button so any view with a tap recognizer
// (label, image, box, ...) participates without a dedicated handler.

package io.mpapp;

import android.view.View;

public final class MppGestureRouter implements View.OnClickListener {
    private final long recognizerPtr;

    public MppGestureRouter(long recognizerPtr) {
        this.recognizerPtr = recognizerPtr;
    }

    @Override
    public void onClick(View v) {
        nativeDispatchTap(recognizerPtr);
    }

    private static native void nativeDispatchTap(long recognizerPtr);
}
