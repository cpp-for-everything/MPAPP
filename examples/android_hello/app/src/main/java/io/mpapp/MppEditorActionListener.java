// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Bridges android.widget.TextView.OnEditorActionListener
// → native MPAPP cell / control handlers so IME "Done / Go / Send /
// Search / Next" actions surface as the cross-platform `completed`
// signal.
//
// `kind` discriminates which native handler type the long pointer
// refers to: 1 = entry_cell.

package io.mpapp;

import android.view.KeyEvent;
import android.widget.TextView;

public final class MppEditorActionListener implements TextView.OnEditorActionListener {
    private final long ownerPtr;
    private final int  kind;

    public MppEditorActionListener(long ownerPtr, int kind) {
        this.ownerPtr = ownerPtr;
        this.kind     = kind;
    }

    @Override
    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
        nativeDispatchEditorAction(ownerPtr, kind, actionId);
        return false;  // don't consume — let the IME close as usual
    }

    private static native void nativeDispatchEditorAction(long ownerPtr, int kind, int actionId);
}
