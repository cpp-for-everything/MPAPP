// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Generic Android OnClickListener that dispatches back
// into native C++ with an opaque (ownerPtr, kind, payload) triple. The
// native side switches on `kind` to decide which widget handler should
// receive the event.
//
// Kinds defined today (kept in sync with src/handlers/android/action_router.cpp):
//   0 — NavigationPage back button     (ownerPtr = navigation_page*, payload unused)
//   1 — Shell tab strip button         (ownerPtr = shell*, payload = tab index)
//
// Adding a new event source: pick the next free kind, wire it on both
// sides, and use this single listener class.

package io.mpapp;

import android.view.View;

public final class MppActionRouter implements View.OnClickListener {
    private final long ownerPtr;
    private final int  kind;
    private final int  payload;

    public MppActionRouter(long ownerPtr, int kind, int payload) {
        this.ownerPtr = ownerPtr;
        this.kind     = kind;
        this.payload  = payload;
    }

    @Override
    public void onClick(View v) {
        nativeDispatchAction(ownerPtr, kind, payload);
    }

    private static native void nativeDispatchAction(long ownerPtr, int kind, int payload);
}
