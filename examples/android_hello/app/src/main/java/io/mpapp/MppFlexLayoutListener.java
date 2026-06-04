// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Fires layout-change events into the native flex_layout
// handler so it can re-run the mpapp::flex_arrange solver against the
// FrameLayout's real pixel size when its allocated bounds change.
//
// addOnLayoutChangeListener is the cheapest Android-native way to watch one
// View's allocated bounds without polling. We supply `ownerPtr` (the C++
// handler `this`) and the native dispatch reinterpret_casts it back. Mirrors
// io.mpapp.MppShapeViewLayoutListener.

package io.mpapp;

import android.view.View;

public final class MppFlexLayoutListener implements View.OnLayoutChangeListener {
    private final long ownerPtr;

    public MppFlexLayoutListener(long ownerPtr) {
        this.ownerPtr = ownerPtr;
    }

    @Override
    public void onLayoutChange(View v,
                               int left, int top, int right, int bottom,
                               int oldLeft, int oldTop, int oldRight, int oldBottom) {
        final int w = right - left;
        final int h = bottom - top;
        if (w > 0 && h > 0) {
            nativeOnFlexLayoutChanged(ownerPtr, w, h);
        }
    }

    private static native void nativeOnFlexLayoutChanged(long ownerPtr, int w, int h);
}
