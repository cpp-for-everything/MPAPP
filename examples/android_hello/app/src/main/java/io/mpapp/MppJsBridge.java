// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. JavaScript-interface bridge that routes
// window.mpapp.send(payload) calls into the native
// hybrid_web_view_handler via a JNI trampoline.
//
// IMPORTANT: methods exposed via @JavascriptInterface run on the
// WebView's internal worker thread; the native dispatcher does not
// assume the UI thread.

package io.mpapp;

import android.webkit.JavascriptInterface;

public final class MppJsBridge {
    private final long handlerPtr;

    public MppJsBridge(long handlerPtr) {
        this.handlerPtr = handlerPtr;
    }

    @JavascriptInterface
    public void send(String payload) {
        nativeDispatchInbound(handlerPtr, payload);
    }

    private static native void nativeDispatchInbound(long handlerPtr, String payload);
}
