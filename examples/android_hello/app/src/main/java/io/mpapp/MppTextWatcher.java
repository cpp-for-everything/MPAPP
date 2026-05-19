// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 follow-up — Android text watcher.
//
// Bridges android.text.TextWatcher → native mpapp::entry::text.set.
// The native entry handler instantiates one of these via JNI and
// installs it on its EditText.

package io.mpapp;

import android.text.Editable;
import android.text.TextWatcher;

public final class MppTextWatcher implements TextWatcher {
    private final long handlerPtr;

    public MppTextWatcher(long handlerPtr) {
        this.handlerPtr = handlerPtr;
    }

    @Override
    public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

    @Override
    public void onTextChanged(CharSequence s, int start, int before, int count) {}

    @Override
    public void afterTextChanged(Editable s) {
        nativeDispatchTextChanged(handlerPtr, s == null ? null : s.toString());
    }

    private static native void nativeDispatchTextChanged(long handlerPtr, String text);
}
