# Android emulator end-to-end evidence

Real on-device verification of MPAPP, captured from the `coroute_test`
AVD (Android emulator) via `adb screencap`. Unlike the WSLg/msrdc
projection (which the DirectComposition wall blocks for PrintWindow —
see `T-0050/notes/wslg-dcomp-wall.md`), `adb screencap` reads the
device framebuffer directly, so Android GUI capture is clean and
scriptable. Harness: `tools/dev/android-e2e.ps1`.

## What the shots prove

`android_hello-running.png` — the `io.mpapp.example/.MainActivity`
running on the emulator, rendering the full MPAPP **real Android
handler** stack via JNI:

- title bar, a teal **BoxView**, a red **ShapeView/GraphicsView**
  ellipse (drawn through the Cairo backend, statically linked into the
  `.so`), a **Label** ("Count: 0 — hello, world"), an **Entry** ("Type
  your name"), a **Switch**, a **CheckBox**, a **Slider**, a **Button**
  ("Click me"), and two expandable **TableView**-style items
  (Account / Preferences).

`android_hello-after-tap.png` — after two `adb shell input tap`s on the
"Click me" button, the label reads **"Count: 2"**. This exercises the
complete live event pipeline on a real device:

```
native Android Button click
  → JNI MppClickListener
  → mpapp::signal<> clicked
  → view_model Observable<int> count.set(count+1)
  → Observable::changed
  → label_handler<platform::android> map_text
  → native TextView.setText
```

That `Observable → handler → native widget` path is exactly the one the
RFC-0007 **binding** engine drives, so this also transitively validates
the binding runtime on-device (the binding integration test pins the
binding→pipeline link; this pins the pipeline→native-widget link on a
real Android runtime).

## Reproduce

```
pwsh tools/dev/android-e2e.ps1 -Action boot
.\_build_android.bat                         # build the APK
pwsh tools/dev/android-e2e.ps1 -Action install
pwsh tools/dev/android-e2e.ps1 -Action launch
pwsh tools/dev/android-e2e.ps1 -Action screenshot -Out shot.png
pwsh tools/dev/android-e2e.ps1 -Action tap -X 540 -Y 893
```
