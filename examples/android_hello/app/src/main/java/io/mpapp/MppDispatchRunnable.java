// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android main-thread dispatcher Runnable shim.
//
// Wraps a native std::function<void()> pointer (the `token`) as a
// java.lang.Runnable. The native looper_dispatcher posts an instance of this
// onto a Handler bound to the main Looper (Handler.post / postDelayed); when
// the Looper runs it, run() JNI-dispatches nativeRun(token), which invokes
// and frees the native closure. Same kind-discriminated-router pattern as
// MppClickRouter / MppGestureRouter.

package io.mpapp;

public final class MppDispatchRunnable implements Runnable {
    private final long token;

    public MppDispatchRunnable(long token) {
        this.token = token;
    }

    @Override
    public void run() {
        nativeRun(token);
    }

    private static native void nativeRun(long token);
}
