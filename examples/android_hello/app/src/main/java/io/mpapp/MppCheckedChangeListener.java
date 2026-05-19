// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Bridges android.widget.CompoundButton.OnCheckedChangeListener
// into the native handler so user-toggled Switch / CheckBox state reaches
// mpapp::switch_::is_on / mpapp::check_box::is_checked.

package io.mpapp;

import android.widget.CompoundButton;

public final class MppCheckedChangeListener implements CompoundButton.OnCheckedChangeListener {
    private final long handlerPtr;

    public MppCheckedChangeListener(long handlerPtr) {
        this.handlerPtr = handlerPtr;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        nativeDispatchCheckedChanged(handlerPtr, isChecked);
    }

    private static native void nativeDispatchCheckedChanged(long handlerPtr, boolean checked);
}
