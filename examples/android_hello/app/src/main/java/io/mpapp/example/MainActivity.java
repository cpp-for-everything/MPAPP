// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android example MainActivity.
//
// Minimal Activity that hands control to native C++ (mpapp::run<App>).
// All UI composition happens in the C++ on_launch override; the Activity
// is just the bootstrap that gives native code a Context to construct
// android.widget.* against.

package io.mpapp.example;

import android.app.Activity;
import android.os.Bundle;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("android_hello");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        nativeRegisterActivity(this);
        nativeLaunch();
    }

    // Implemented in src/handlers/android/jni_bridge.cpp via JNI_OnLoad +
    // the example's native_main.cpp.
    private native void nativeRegisterActivity(Activity activity);
    private native void nativeLaunch();
}
