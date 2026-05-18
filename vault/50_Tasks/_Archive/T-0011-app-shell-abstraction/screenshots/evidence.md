# T-0011 — Live cross-platform verification evidence

Captured 2026-05-18 via the Claude Code computer-use MCP, screenshots
are embedded in the spawning session's conversation transcript. The
text below records what was actually exercised; the raw PNGs can be
re-captured at any time by re-running the spikes against the same
machine.

## Windows — `windows_button_spike.exe`

**Source**: `examples/windows_button_spike/main.cpp` (zero `winrt::`,
`mux::`, `muxc::`, `Mdd*` tokens in user-facing code; verified via
grep).

**Handler set**: `mpapp/handlers/windows/{application,window,stack_layout,button,label}`
backed by:

- `winrt::Microsoft::UI::Xaml::Application` (`Application::Start` +
  `mux::ApplicationT<App>` subclass, both hidden inside
  `application_handler.cpp`).
- `winrt::Microsoft::UI::Xaml::Window`
- `winrt::Microsoft::UI::Xaml::Controls::StackPanel`
- `winrt::Microsoft::UI::Xaml::Controls::Button`
- `winrt::Microsoft::UI::Xaml::Controls::TextBlock`

**Verified live**:

1. `mpapp::run<spike_app>(argc, argv)` invocation drove the full
   `MddBootstrapInitialize2` → `WindowsAppRuntime_EnsureIsLoaded` →
   `winrt::init_apartment` → `Application::Start` → custom
   `mpapp_winui_app::OnLaunched` chain.
2. The user's `spike_app::on_launch()` ran and built the UI tree via
   the MPAPP surface (no WinRT names in user code).
3. Title bar rendered as **"MPAPP T-0011 - app-shell rewrite"**.
4. Body rendered the cross-platform `mpapp::label` (text:
   `"Count: 0"`) on top of `mpapp::button` (text: `"Click me"`).
5. **7 clicks** on the button drove the cross-platform `clicked`
   signal → `Observable<int>::set` on `count` → `count.changed`
   signal → label `text` reset → native `TextBlock` re-render.
   Final label showed **"Count: 7"**.

**Bug fix discovered during this run**: the first build threw HRESULT
`0x800710DD` from `mux::Window::Close()` when the window had never
been activated. `apply_is_visible(false)` was syncing initial state
at bind time, hitting that path. Fix: gate the `Close()` call behind
a `was_activated_` flag in `window_handler<platform::windows>`. After
the fix, the spike runs to interactivity and stays alive.

## Linux — `gtk4_hello`

**Source**: `examples/gtk4_hello/main.cpp` (originally a raw C/GTK4
file; rewritten in this batch to use the same MPAPP surface as the
Windows spike — only the handler template arguments differ:
`platform::linux_` instead of `platform::windows`). Zero `gtk_*`,
`GTK_*`, `GtkApplication`, `g_application_run` tokens in user-facing
code.

**Handler set**: `mpapp/handlers/linux/{application,window,stack_layout,button,label}`
backed by:

- `GtkApplication` (`gtk_application_new` + `g_application_run`, both
  hidden inside `application_handler.cpp`).
- `GtkApplicationWindow`
- `GtkBox`
- `GtkButton`
- `GtkLabel`

**Build**: Inside WSL2 Ubuntu-24.04, clang 18.1.3, GTK4 4.14.5
(`libgtk-4-dev` installed during this batch), CMake 3.28+, Ninja.
The same root CMake project compiles `mpapp-core` + the new
`mpapp-handlers-linux` static library + the `gtk4_hello` exe.

**Verified live via WSLg**:

1. `mpapp::run<spike_app>(argc, argv)` from the user's `main`
   reached the GTK4 `activate` callback, where the App was
   constructed and `on_launch()` fired.
2. Window rendered with title bar
   **"MPAPP T-0011 - GTK4 hello (cross-platform spi..."**
   (truncated by GTK header bar).
3. `mpapp::stack_layout` rendered as a vertical `GtkBox` with
   `Count: 0` label on top, `Click me` button below, centered and
   padded per the user code.
4. **5 clicks** drove the cross-platform signal chain end-to-end:
   the GTK4 button's `clicked` signal → `mpapp::button::clicked.emit`
   → user-side handler → `Observable<int>::set` →
   `count.changed.emit` → `label.text.set` → GTK4 `gtk_label_set_text`.
   Final label showed **"Count: 5"**.

(A second WSLg session at the start of the verification showed
**"Count: 10"** after 3 inadvertent clicks during taskbar interaction
plus 7 additional clicks; both runs are independent confirmations
of the same end-to-end pipeline.)

## What this proves

The same `main.cpp` body — a `class spike_app : public mpapp::application`
that builds button + label + stack_layout + window in `on_launch` —
compiles unchanged onto **three** completely different platforms
(WinUI 3 on Windows, GTK4 on Linux, Java widgets via JNI on
Android) and produces visually + behaviourally equivalent native
windows on all three. The only diffs between
`examples/windows_button_spike/main.cpp`, `examples/gtk4_hello/main.cpp`,
and `examples/android_hello/app/src/main/cpp/native_main.cpp` are:

1. The `#include <mpapp/handlers/{platform}/...>` lines name a
   different platform directory.
2. The `mpapp::button_handler<mpapp::platform::{platform}>` template
   arguments name a different `platform::` tag.

Everything else — the view-model, the property bindings, the signal
wiring, the layout composition, the entry point — is identical and
truly cross-platform. ADR-0006 (interop parity) is satisfied above
the widget layer.

## macOS / iOS

Code-complete (`include/mpapp/handlers/macos/`, `include/mpapp/handlers/ios/`,
`src/handlers/macos/*.mm`, `src/handlers/ios/*.mm`). Not verifiable
on this Windows host — no Apple toolchain available. Compilation
will be exercised once a self-hosted macOS runner comes online (per
[[T-0008-mac-ios-test-harness-design]]).

## Android — fully verified live on emulator

Verified on the existing `D:\android-sdk` install (Android SDK 34,
NDK r26.1.10909125, AVD `coroute_test` on a Pixel-class device
profile, system-image `android-34/google_apis`).

**Source**: `examples/android_hello/app/src/main/cpp/native_main.cpp`
+ Java `MainActivity` + `MppClickRouter` + manifest + Gradle.
The C++ body is the same view-model + UI composition as the Windows
and Linux spikes; only the handler template arguments differ
(`platform::android`).

**Handler set**: `mpapp/handlers/android/{application,window,stack_layout,
button,label}_handler` backed by real Java widget jobjects:

- `android.app.Activity` (registered from `MainActivity.onCreate`
  via `nativeRegisterActivity(this)`).
- `android.widget.LinearLayout` (for `mpapp::stack_layout`).
- `android.widget.Button` (for `mpapp::button`).
- `android.widget.TextView` (for `mpapp::label`).
- `View.setOnClickListener(new io.mpapp.MppClickRouter(buttonPtr))`
  bridges Java clicks back into `mpapp::button::clicked.emit()` via
  the registered `Java_io_mpapp_MppClickRouter_nativeDispatchClick`
  JNI thunk.

**Build**: Gradle 8.10 + AGP 8.5.2 + OpenJDK 21.0.8 + NDK r26.1.
Required two project-wide CMake fixes uncovered by this build:

- `CMAKE_CXX_SCAN_FOR_MODULES OFF` globally — NDK clang has no
  `clang-scan-deps`.
- Every `std::formatter` specialisation gated behind
  `__has_include(<format>) && !defined(__ANDROID__)` — NDK r26
  libc++ doesn't ship `<format>` yet. The formatters are only used
  by mock-handler tests, so excluding them from Android is safe.

**Verified live via adb**:

1. APK packages cleanly (`app-debug.apk` ~3 MB).
2. `adb install -r` succeeds (`pm list packages` shows
   `package:io.mpapp.example`).
3. `adb shell am start -n io.mpapp.example/.MainActivity` launches
   the Activity. `MainActivity.onCreate` calls
   `nativeRegisterActivity(this)` → `nativeLaunch()` →
   `mpapp::run<spike_app>(...)`.
4. The Activity renders with title bar **"MPAPP T-0011 - Android hello"**.
5. The native `setContentView` mounts the `LinearLayout` (from
   `mpapp::stack_layout`) holding the `TextView` (`mpapp::label`)
   showing **"Count: 0"** above the `Button` (`mpapp::button`)
   labelled **"Click me"**. `uiautomator dump` confirms the button
   bounds as `[24,1268][1056,1394]`.
6. **`adb shell input tap 540 1331`** × 7 → label updates to
   **"Count: 7"**. Cross-platform signal pipeline confirmed
   end-to-end on Android:

   ```
   Java Button click
     -> View.OnClickListener (MppClickRouter)
     -> JNI nativeDispatchClick(buttonPtr)
     -> mpapp::android_button_dispatch_click(button*)
     -> button.clicked.emit()
     -> user's click_cb → vm.count.set(count + 1)
     -> count.changed.emit
     -> user's count_cb → label.text.set("Count: ...")
     -> label.text.changed.emit
     -> label_handler<android>::apply_text via callback
     -> JNI CallVoidMethod TextView.setText
     -> visible counter update
   ```

**Bug fixes discovered during this verification** (all in the
final-shipped source):

1. **`map_clicked` was a stub** — the OnClickListener was never
   installed on the Java Button. Fix: `button_handler<android>::map_clicked`
   now instantiates `io.mpapp.MppClickRouter(buttonPtr)` via JNI and
   calls `View.setOnClickListener(router)` so taps route back into
   the cross-platform `mpapp::button::clicked` signal.
2. **ART `CheckJNI` aborts on pending exceptions**. Bionic's checked
   JNI mode calls `abort()` if any JNI call is made while an
   exception is pending. `setContentView(null)` (the initial-state
   sync at bind time when `window.content` is `nullptr`)
   legitimately throws `IllegalArgumentException` — leaving an
   exception pending that nukes the next call. Fix: every
   `apply_*` / helper now opens with `if (env->ExceptionCheck())
   env->ExceptionClear();` and clears after each `CallVoidMethod`.
   Applied across `window_handler`, `button_handler`,
   `label_handler`, and `stack_layout_handler`.
3. **`GetObjectClass(activity)` was the abort site under CheckJNI**.
   Swapped to `FindClass("android/app/Activity")` /
   `FindClass("android/view/ViewGroup")` /
   `FindClass("android/widget/LinearLayout")` — looking up the
   class by name avoids whatever ART CheckJNI quirk was tripping
   on the global ref. Side benefit: `ViewGroup.addView` is the
   right level for the type, since `LinearLayout` inherits it.

Screenshots committed alongside this file:

- `android-emulator-app-installed.png` — the launcher with the
  package installed (early state, before fixing the click chain).
- `android-mpapp-running.png` — the MPAPP window rendered, but
  the click handler hadn't been implemented yet (Count stays 0).
- `android-mpapp-count7.png` — after wiring `map_clicked` + the
  `ExceptionClear` pattern, 7 button taps drive Count: 0 → 7.
- `android-mpapp-final-count3.png` — after stripping the
  diagnostic logging and re-running, 3 taps → Count: 3.
