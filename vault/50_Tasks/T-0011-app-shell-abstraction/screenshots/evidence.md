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
compiles unchanged onto two completely different platforms and
produces visually + behaviourally equivalent native windows. The only
diffs between `examples/windows_button_spike/main.cpp` and
`examples/gtk4_hello/main.cpp` are:

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

## Android — code-complete, APK builds + installs; UI render hits JNI lifetime bug

Verified on the existing `D:\android-sdk` install (Android SDK 34,
NDK r26.1.10909125, AVD `coroute_test` on a Pixel-class device
profile, system-image `android-34/google_apis`):

1. **Gradle 8.10** downloaded and pinned at `D:\gradle-8.10` (user-
   authorised). AGP 8.5.2 + OpenJDK 21.0.8 (already on this host
   from prior Android Studio install).
2. **Native build via NDK** — `examples/android_hello/app/src/main/cpp/CMakeLists.txt`
   compiles the full app-shell handler set + `mpapp-core` sources
   into a single `libandroid_hello.so` for `x86_64`. Required two
   project-wide CMake fixes uncovered by this build:
     - `CMAKE_CXX_SCAN_FOR_MODULES OFF` globally (NDK clang has no
       `clang-scan-deps`).
     - Gate every `std::formatter` specialisation behind
       `__has_include(<format>) && !defined(__ANDROID__)`. The NDK's
       libc++ in r26 doesn't ship `<format>` yet; the formatters are
       only used by mock tests, so excluding them from the Android
       build is safe.
3. **APK packaged** — `app-debug.apk` produced under
   `examples/android_hello/app/build/outputs/apk/debug/`.
4. **Installed** via `adb install -r` on the running emulator
   (`pm list packages` confirms `package:io.mpapp.example`).
5. **Launched** via `adb shell am start -n io.mpapp.example/.MainActivity`.
   `MainActivity.onCreate` runs, calls `nativeRegisterActivity(this)`
   then `nativeLaunch()` → `mpapp::run<spike_app>(...)`.
   `spike_app::on_launch()` enters, builds the UI tree via the
   cross-platform surface.
6. **Crash** at `window_handler<android>::apply_content` →
   `env->GetObjectClass(native_)` → SIGABRT in the bionic JNI
   checker (stack frame `#10 GetObjectClass+35`, called from
   `apply_content+127`). The `content.changed` signal pipeline
   fires correctly through `Observable<view*>::set` →
   `signal::emit` → `content_cb_t::operator()` → `apply_content` —
   all four MPAPP layers traverse exactly as on Windows and Linux.
   The abort is purely in the last step's JNI call.

**Hypothesis (most likely)**: the activity `jobject` registered in
`nativeRegisterActivity` is `NewGlobalRef`'d in `set_activity` and
then `NewGlobalRef`'d *again* in `window_handler<android>::window_handler()`
to populate `native_`. The double-global-ref may not be the issue per
se — both should remain valid — but the second `GetObjectClass` call
on the latter ref aborts. The bionic abort signature (`SIGABRT` /
`SI_QUEUE` / `signal 6`) is consistent with ART's `CheckJNI` finding
a stale or null ref.

**Fix attempted, did not resolve**: dropped the `native_` member from
`window_handler<android>` and have `apply_*` look up the activity via
`detail::get_activity()` directly each call (committed in the same
batch — the source after this change is the as-shipped one). The
clean-rebuilt APK reproduces the exact same crash signature
(`GetObjectClass+35` → SIGABRT), confirming the issue is not in the
double-NewGlobalRef layer. The crash needs deeper debugging with
either CheckJNI extended diagnostics or a debug native build with
breakpoints; tracked as M-05 follow-up.

**What this still proves**:

- The same `examples/android_hello/app/src/main/cpp/native_main.cpp`
  body — identical to the Windows + Linux `main.cpp` except for the
  handler template arguments — **compiles and links cleanly against
  the Android NDK**.
- The Java ↔ JNI ↔ C++ ↔ mpapp::run<App> ↔ on_launch chain works
  end-to-end on the emulator (the crash is *inside* on_launch, deep
  in the cross-platform pipeline that did fire correctly).
- The full APK build pipeline — Gradle → CMake (NDK clang) →
  `.so` packaging → install → launch — is wired and verified.

Screenshot of the emulator immediately after the APK install
(showing the launcher) is at `android-emulator-app-installed.png`
next to this file. The `io.mpapp.example` package is installed
(verified via `adb shell pm list packages`); it crashes on launch
before its UI renders.
