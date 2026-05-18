// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android click router.
//
// Bridges android.widget.Button.OnClickListener back into the native
// mpapp::button.clicked signal. The native button handler installs an
// instance of this class as the listener on its underlying Button,
// passing the jlong native button pointer as the routing key.

package io.mpapp;

import android.view.View;

public final class MppClickRouter implements View.OnClickListener {
    private final long buttonPtr;

    public MppClickRouter(long buttonPtr) {
        this.buttonPtr = buttonPtr;
    }

    @Override
    public void onClick(View v) {
        nativeDispatchClick(buttonPtr);
    }

    private static native void nativeDispatchClick(long buttonPtr);
}
