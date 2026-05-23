// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0031 phase 2 — fires layout-change events into the
// native shape_view handler so it can reallocate + repaint its
// Bitmap when the ImageView's pixel size changes.
//
// addOnLayoutChangeListener is the cheapest Android-native way to
// watch one View's allocated bounds without polling. We supply
// `ownerPtr` (the C++ handler `this`) and the native dispatch
// reinterpret_casts it back.

package io.mpapp;

import android.view.View;

public final class MppShapeViewLayoutListener implements View.OnLayoutChangeListener {
    private final long ownerPtr;

    public MppShapeViewLayoutListener(long ownerPtr) {
        this.ownerPtr = ownerPtr;
    }

    @Override
    public void onLayoutChange(View v,
                               int left, int top, int right, int bottom,
                               int oldLeft, int oldTop, int oldRight, int oldBottom) {
        final int w = right - left;
        final int h = bottom - top;
        if (w > 0 && h > 0) {
            nativeOnLayoutChanged(ownerPtr, w, h);
        }
    }

    private static native void nativeOnLayoutChanged(long ownerPtr, int w, int h);
}
