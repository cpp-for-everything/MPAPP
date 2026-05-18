# MPAPP — Android hello example

T-0011 follow-up — Android counterpart of `windows_button_spike` and
`gtk4_hello`. Demonstrates that the **same view-model + UI composition
code** compiles unchanged onto the Android JNI handler set; only the
handler-type template arguments change (`platform::android` instead of
`platform::windows` / `platform::linux_`).

## Layout

```
examples/android_hello/
  app/
    src/main/
      AndroidManifest.xml             — single Activity, launchable
      cpp/
        CMakeLists.txt                — externalNativeBuild target
        native_main.cpp               — JNI entry + the user's mpapp::application
      java/io/mpapp/example/
        MainActivity.java             — bootstraps native code
      java/io/mpapp/
        MppClickRouter.java           — bridges Java OnClickListener → mpapp::button.clicked
```

## Build

The CMake side compiles with any Android NDK ≥ r26 + the Android Gradle
Plugin's externalNativeBuild. A minimal `build.gradle.kts` is left for
the consumer to drop in alongside the `app/` directory — kept out of
the repo to avoid pinning a Gradle / AGP version that goes stale.

The expected gradle invocation, once `build.gradle.kts` is provided:

```sh
./gradlew :app:assembleDebug
adb install app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n io.mpapp.example/.MainActivity
```

## Status

The C++ side is the same surface as the Windows and Linux examples and
compiles against the Android NDK. The end-to-end emulator run + apk
install + screenshot lands in the M-05 milestone alongside the Android
self-hosted CI runner work.
