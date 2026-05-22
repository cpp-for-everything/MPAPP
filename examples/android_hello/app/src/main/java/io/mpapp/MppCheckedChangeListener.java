// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Bridges android.widget.CompoundButton.OnCheckedChangeListener
// into the native handler so user-toggled Switch / CheckBox / RadioButton
// state reaches the corresponding C++ Observable.
//
// `kind` discriminates which native handler type the long pointer
// refers to: 1 = switch_, 2 = check_box, 3 = radio_button,
// 4 = switch_cell.

package io.mpapp;

import android.widget.CompoundButton;

public final class MppCheckedChangeListener implements CompoundButton.OnCheckedChangeListener {
    private final long handlerPtr;
    private final int  kind;

    public MppCheckedChangeListener(long handlerPtr, int kind) {
        this.handlerPtr = handlerPtr;
        this.kind       = kind;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        nativeDispatchCheckedChanged(handlerPtr, kind, isChecked);
    }

    private static native void nativeDispatchCheckedChanged(long handlerPtr,
                                                            int kind,
                                                            boolean checked);
}
