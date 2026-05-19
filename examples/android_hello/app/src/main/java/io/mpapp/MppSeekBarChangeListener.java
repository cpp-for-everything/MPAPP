// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Bridges android.widget.SeekBar.OnSeekBarChangeListener
// into the native slider handler.
//
// The Android SeekBar exposes only an integer progress in [0, max].
// The native handler does the int → double remap into the cross-platform
// [minimum, maximum] surface so user code stays in doubles.

package io.mpapp;

import android.widget.SeekBar;

public final class MppSeekBarChangeListener implements SeekBar.OnSeekBarChangeListener {
    private final long handlerPtr;

    public MppSeekBarChangeListener(long handlerPtr) {
        this.handlerPtr = handlerPtr;
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        // Always forward, including programmatic changes. The native
        // side has its own suppress_echo_ flag to break the
        // apply-→native→callback loop for programmatic writes.
        nativeDispatchProgress(handlerPtr, progress, seekBar.getMax());
    }

    @Override public void onStartTrackingTouch(SeekBar seekBar) {}
    @Override public void onStopTrackingTouch(SeekBar seekBar) {}

    private static native void nativeDispatchProgress(long handlerPtr, int progress, int max);
}
