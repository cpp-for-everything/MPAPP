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

## Android

Code-complete (`include/mpapp/handlers/android/`,
`src/handlers/android/*.cpp`, `examples/android_hello/` with
MainActivity + JNI bridge + MppClickRouter). Pending: Gradle harness
+ emulator install on this host (in flight after the Windows + Linux
verification).
