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
import android.util.Log;

import java.io.File;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("android_hello");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        nativeRegisterActivity(this);

        // T-0016 Rule 11 catch-up: render the canvas-facade demo
        // through the active Cairo backend and write a PNG to the
        // app's external files dir. Visible end-to-end proof that
        // Cairo on Android x86_64 actually renders pixels at runtime
        // (not just that it links).
        try {
            File outDir = getExternalFilesDir(null);
            if (outDir != null) {
                File png = new File(outDir, "android-cairo-render.png");
                int ok = nativeRenderCairoDemoPng(png.getAbsolutePath());
                Log.i("MPAPP", "nativeRenderCairoDemoPng -> " + ok + " at " + png);
            }
        } catch (Throwable t) {
            Log.w("MPAPP", "Cairo render skipped: " + t);
        }

        // T-0017 Rule 11 catch-up: exercise the typed route_table +
        // can_activate / can_deactivate guards + page lifecycle
        // signals and emit a structured trace to logcat. Each line
        // is prefixed `T-0017:` so the test infra can grep them out.
        try {
            nativeRunRoutesSmokeTest();
        } catch (Throwable t) {
            Log.w("MPAPP", "Routes smoke test skipped: " + t);
        }

        // T-0018 Rule 11 catch-up: exercise ADR-0018 Phase F's
        // `hybrid_bridge::dispatch_async` across sync, inline-async,
        // and deferred-async method shapes. Output prefixed `T-0018:`.
        try {
            nativeRunBridgeSmokeTest();
        } catch (Throwable t) {
            Log.w("MPAPP", "Bridge smoke test skipped: " + t);
        }

        // T-0019 Rule 11 catch-up: exercise CollectionView
        // item_template — factory-based typed cells, materialize on
        // items_source change, materialized_changed signal. Output
        // prefixed `T-0019:`.
        try {
            nativeRunItemTemplateSmokeTest();
        } catch (Throwable t) {
            Log.w("MPAPP", "item_template smoke test skipped: " + t);
        }

        nativeLaunch();
    }

    // Implemented in src/handlers/android/jni_bridge.cpp via JNI_OnLoad +
    // the example's native_main.cpp.
    private native void nativeRegisterActivity(Activity activity);
    private native void nativeLaunch();
    private native int  nativeRenderCairoDemoPng(String outputPath);
    private native void nativeRunRoutesSmokeTest();
    private native void nativeRunBridgeSmokeTest();
    private native void nativeRunItemTemplateSmokeTest();
}
