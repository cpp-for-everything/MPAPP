// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Bridges android.text.TextWatcher → native mpapp text
// handlers (entry / editor / entry_cell). `kind` discriminates which
// handler type the long pointer refers to: 1 = entry, 2 = editor,
// 3 = entry_cell.

package io.mpapp;

import android.text.Editable;
import android.text.TextWatcher;

public final class MppTextWatcher implements TextWatcher {
    private final long handlerPtr;
    private final int  kind;

    public MppTextWatcher(long handlerPtr, int kind) {
        this.handlerPtr = handlerPtr;
        this.kind       = kind;
    }

    @Override
    public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

    @Override
    public void onTextChanged(CharSequence s, int start, int before, int count) {}

    @Override
    public void afterTextChanged(Editable s) {
        nativeDispatchTextChanged(handlerPtr, kind, s == null ? null : s.toString());
    }

    private static native void nativeDispatchTextChanged(long handlerPtr, int kind, String text);
}
