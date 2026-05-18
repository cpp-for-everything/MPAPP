# T-0011 — application_handler in the handler taxonomy

The existing per-widget handler pattern is

```
control class        partial specialisation              role
─────────────────────────────────────────────────────────────────────────
mpapp::button   →  button_handler<platform::windows>   owns a `mux::Button`
mpapp::label    →  label_handler<platform::windows>    owns a `mux::TextBlock`
…
```

T-0011 extends it to **non-widget** types — types that don't render
themselves but own platform machinery:

```
mpapp::application →  application_handler<platform::windows>
                          owns a `mux::Application` subclass,
                          drives `mux::Application::Start`,
                          owns MddBootstrap lifetime.

mpapp::window      →  window_handler<platform::windows>
                          owns a `mux::Window`.
                          maps `title` → `Window::Title(…)`, etc.

mpapp::page        →  page_handler<platform::windows>
                          owns a `muxc::Page`-equivalent (in MAUI's
                          model — for the WinUI handler this is just a
                          `muxc::Frame` + content slot).

mpapp::stack_layout → stack_layout_handler<platform::windows>
                          owns a `muxc::StackPanel`.
                          maps `orientation` → `StackPanel::Orientation`,
                          etc.
```

This is the same partial-specialisation pattern, just applied to types
whose native counterpart is not a `FrameworkElement` widget.

## Lifetime model for `application_handler`

`mpapp::run<App>` semantics:

1. Construct `application_handler<platform::current>` on the stack (in
   `run<App>`'s frame).
2. The handler runs platform init (`MddBootstrap*` →
   `WindowsAppRuntime_EnsureIsLoaded` → `winrt::init_apartment`).
3. Handler calls `mux::Application::Start([this](auto&&){
     ::winrt::make<internal_app_t>(this); });` which is a *blocking*
   call that returns when the WinUI app exits.
4. `internal_app_t` is a private `mux::ApplicationT` subclass owned by
   the handler. In its `OnLaunched`:
     a. construct the user's `App` (heap-allocated, owned by the handler)
     b. call `App::on_launch()`
5. When the WinUI event loop exits, the handler tears down (Mdd shutdown)
   and `mux::Application::Start` returns. `run<App>` returns the exit
   code.

The user's `App` class is a `mpapp::application`-derived type the
*handler* allocates. The user never sees a `mux::ApplicationT`.

## Singleton vs multi-instance

- **Windows:** WinUI 3 enforces a single `mux::Application` per process.
  `application_handler<platform::windows>` therefore enforces a single
  `mpapp::application`-derived instance per process via a static
  guard. Attempting to construct a second one is a precondition
  violation (assert / throws).
- **Linux (GTK4):** `gtk_application_new` allows multiple application
  IDs per process technically, but the conventional model is one
  application per process. MPAPP enforces single-instance for parity.
- **macOS (AppKit):** `NSApplication.shared` is a singleton.
- **iOS (UIKit):** `UIApplication.shared` is a singleton.
- **Android (fbjni):** the Android `Application` Java class is a
  singleton per JVM. The MPAPP wrapper mirrors that.

Decision: `mpapp::application` is **single-instance everywhere**.
Captured in ADR-0012 (`status: draft`).

## `window.content` ownership

User code today writes:
```cpp
window_.content = layout_;
```
where `layout_` is stack-allocated in the user's `App`. The
`window_handler` must:
- Take a non-owning reference to the user's `view`-derived object.
- When the value of `content` changes, switch the platform-window's
  content slot (e.g. `mux::Window::Content(...)`).
- Tear down its own platform tree before the user's `App` destructor
  runs (so `mux::Window::Content(nullptr)` is called before the user's
  `layout_` member is destroyed).

The cleanup ordering is handled by the handler running its destructor
*before* the user's App fields run theirs — `application_handler`
guarantees this by tearing down the `mux::Window` in its own destructor,
which runs before its sibling user-owned widgets do.

## Cross-references

- [[ADR-0006-interop-parity]] — the user-facing surface must be
  identical on every platform; the spike was a violation T-0011 closes.
- [[ADR-0008-mock-first-implementation]] — mock handlers ship first;
  Windows-real lands second; Linux/macOS/iOS/Android follow per phase.
- [[Handlers]] — the cross-cutting handler architecture; T-0011 extends
  it without modifying the pattern.
