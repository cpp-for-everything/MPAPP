// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Custom WebViewClient that routes onPageStarted /
// onPageFinished into the native web_view_handler. Mirrors the
// MppActionRouter / MppCheckedChangeListener pattern for native-event
// → C++ Observable propagation.

package io.mpapp;

import android.graphics.Bitmap;
import android.webkit.WebView;
import android.webkit.WebViewClient;

public final class MppWebViewClient extends WebViewClient {
    private final long handlerPtr;

    public MppWebViewClient(long handlerPtr) {
        this.handlerPtr = handlerPtr;
    }

    @Override
    public void onPageStarted(WebView view, String url, Bitmap favicon) {
        nativeDispatchPageStarted(handlerPtr, url);
    }

    @Override
    public void onPageFinished(WebView view, String url) {
        // WebViewClient.onPageFinished doesn't carry a success flag — Android
        // surfaces errors through onReceivedError. For the v1 surface we
        // mark every finish as success and let onReceivedError flip the
        // narrative later.
        nativeDispatchPageFinished(handlerPtr, url, true);
    }

    private static native void nativeDispatchPageStarted(long handlerPtr, String url);
    private static native void nativeDispatchPageFinished(long handlerPtr, String url, boolean success);
}
