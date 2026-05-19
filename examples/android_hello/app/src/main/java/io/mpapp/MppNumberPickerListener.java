// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Bridges android.widget.NumberPicker.OnValueChangeListener
// into the native stepper handler.

package io.mpapp;

import android.widget.NumberPicker;

public final class MppNumberPickerListener implements NumberPicker.OnValueChangeListener {
    private final long handlerPtr;

    public MppNumberPickerListener(long handlerPtr) {
        this.handlerPtr = handlerPtr;
    }

    @Override
    public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
        nativeDispatchValue(handlerPtr, newVal);
    }

    private static native void nativeDispatchValue(long handlerPtr, int value);
}
