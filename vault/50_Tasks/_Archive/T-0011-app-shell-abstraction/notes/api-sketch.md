# T-0011 — API sketch for the app-shell abstraction

This is the **proposed** public surface. It deliberately mirrors the
widget-handler pattern that already works for `mpapp::button` and friends,
so there are no new mechanisms — only new types that use them.

## Goal of the rewrite

The current spike's user-facing code (`examples/windows_button_spike/main.cpp`)
must go from this:

```cpp
// 25 raw WinRT tokens in this file today
namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

struct App : mux::ApplicationT<App> {
    void OnLaunched(mux::LaunchActivatedEventArgs const&) {
        muxc::StackPanel panel{};
        panel.Orientation(muxc::Orientation::Vertical);
        panel.Padding(mux::Thickness{24, 24, 24, 24});
        panel.Children().Append(state.lbl_handler.native());
        // …
        mux::Window window{};
        window.Title(L"…");
        window.Content(panel);
        window.Activate();
    }
};

int __stdcall wWinMain(...) {
    ::MddBootstrapInitialize2(...);
    ::WindowsAppRuntime_EnsureIsLoaded();
    ::winrt::init_apartment(...);
    mux::Application::Start(...);
}
```

…to this:

```cpp
// Zero winrt/mux/muxc/Mdd tokens
#include <mpapp/application.hpp>
#include <mpapp/window.hpp>
#include <mpapp/stack_layout.hpp>

#include <mpapp/button.hpp>
#include <mpapp/label.hpp>

struct view_model {
    mpapp::Observable<int> count{0};
};

class my_app : public mpapp::application {
public:
    void on_launch() override {
        // VM
        vm_.count.changed.subscribe(count_slot_,
            [this](int n) { lbl_.text = "Count: " + std::to_string(n); });

        // UI
        btn_.text = "Click me";
        lbl_.text = "Count: 0";
        btn_.clicked.subscribe(click_slot_,
            [this]() { vm_.count.set(vm_.count.get() + 1); });

        layout_.orientation     = mpapp::orientation::vertical;
        layout_.spacing         = 12;
        layout_.padding         = mpapp::thickness{24};
        layout_.horizontal_alignment = mpapp::h_align::center;
        layout_.vertical_alignment   = mpapp::v_align::center;
        layout_.children().add(lbl_);
        layout_.children().add(btn_);

        window_.title   = "MPAPP T-0011 — app-shell rewrite";
        window_.content = layout_;
        window_.show();
    }

private:
    view_model            vm_{};
    mpapp::button         btn_{};
    mpapp::label          lbl_{};
    mpapp::stack_layout   layout_{};
    mpapp::window         window_{};
    mpapp::signal_slot<>           click_slot_{};
    mpapp::signal_slot<const int&> count_slot_{};
};

int main(int argc, char** argv) {
    return mpapp::run<my_app>(argc, argv);
}
```

That's the contract this task ships against.

## Proposed types

### `mpapp::application`

```cpp
namespace mpapp {

template <class Platform> class application_handler;

class application {
public:
    application();
    virtual ~application();

    // User overrides — called by the platform handler on the UI thread
    // after platform init is complete.
    virtual void on_launch()    = 0;
    virtual void on_suspend()   {}
    virtual void on_resume()    {}
    virtual void on_terminate() {}

    // Exposed for handlers and entry-point machinery; users almost never
    // touch these directly.
    application_handler<platform::current>&       handler() noexcept;
    const application_handler<platform::current>& handler() const noexcept;

    // The framework dispatcher (per [[Async Executor and Event Loops]]).
    static executor& main_dispatcher() noexcept;

protected:
    application(const application&)            = delete;
    application& operator=(const application&) = delete;
};

// Entry-point helper. Constructs the App, owns the platform handler,
// runs the event loop. Returns when the app exits. No public-API macro
// (Rule 1).
template <class App>
int run(int argc, char** argv);

} // namespace mpapp
```

The Windows handler internally does what `wWinMain` does today:
`MddBootstrapInitialize2` → `WindowsAppRuntime_EnsureIsLoaded` →
`winrt::init_apartment` → `Application::Start` → constructs the user's
`App` and calls `on_launch()` from `OnLaunched`. None of those tokens
appear in the public header — they live in
`src/handlers/windows/application_handler.cpp`.

### `mpapp::window`

```cpp
namespace mpapp {

template <class Platform> class window_handler;

class window : public control<window> {
public:
    Observable<std::string> title{""};
    // `content` is a non-owning ref to any view-derived widget. The user
    // owns the lifetime; window only stores the pointer.
    Observable<view*>       content{nullptr};
    Observable<int>         width{0};
    Observable<int>         height{0};

    void show();
    void close();

    signal<>                closed;

    window_handler<platform::current>&       handler() noexcept;
    void set_handler(window_handler<platform::current>& h) noexcept;
};

} // namespace mpapp
```

### `mpapp::page`

Matches MAUI's `ContentPage`: a navigation host wrapping a single
content `view`. Carries a `title`, optional `back_button_visible`,
etc. Handler pattern identical to `window`.

### `mpapp::stack_layout`, `mpapp::grid_layout`, ancillary types

```cpp
namespace mpapp {

enum class orientation : std::uint8_t { vertical, horizontal };

enum class h_align : std::uint8_t { start, center, end, stretch };
enum class v_align : std::uint8_t { start, center, end, stretch };

struct thickness {
    double left{0}, top{0}, right{0}, bottom{0};
    constexpr thickness() = default;
    constexpr explicit thickness(double uniform)
        : left(uniform), top(uniform), right(uniform), bottom(uniform) {}
    constexpr thickness(double l, double t, double r, double b)
        : left(l), top(t), right(r), bottom(b) {}
};

template <class Platform> class stack_layout_handler;

class stack_layout : public layout {
public:
    Observable<orientation> orientation{orientation::vertical};
    Observable<double>      spacing{0};
    Observable<thickness>   padding{};
    Observable<h_align>     horizontal_alignment{h_align::stretch};
    Observable<v_align>     vertical_alignment{v_align::stretch};

    // children: shared with the layout base; stack_layout adds no
    // children-management API of its own — it just composes them.

    stack_layout_handler<platform::current>&       handler() noexcept;
    void set_handler(stack_layout_handler<platform::current>& h) noexcept;
};

class grid_layout : public layout {
    // rows, columns, row_definitions, column_definitions, …
};

} // namespace mpapp
```

`orientation`, `h_align`, `v_align`, `thickness` are framework-owned
types so user code never names `muxc::Orientation`, `mux::Thickness`,
etc. Each platform handler maps them to its native equivalent.

## Handler taxonomy

The existing handler pattern is

```
include/mpapp/handlers/<platform>/<widget>_handler.hpp
   └── partial specialisation of  <widget>_handler<platform::<name>>
```

T-0011 just adds four new widget types to that grid:

```
include/mpapp/handlers/windows/application_handler.hpp
include/mpapp/handlers/windows/window_handler.hpp
include/mpapp/handlers/windows/page_handler.hpp
include/mpapp/handlers/windows/stack_layout_handler.hpp
include/mpapp/handlers/windows/grid_layout_handler.hpp

include/mpapp/handlers/mock/application_handler.hpp
include/mpapp/handlers/mock/window_handler.hpp
include/mpapp/handlers/mock/page_handler.hpp
include/mpapp/handlers/mock/stack_layout_handler.hpp
include/mpapp/handlers/mock/grid_layout_handler.hpp

include/mpapp/handlers/linux/  (stubs, gated UNIX AND NOT APPLE)
include/mpapp/handlers/macos/  (stubs)
include/mpapp/handlers/ios/    (stubs)
include/mpapp/handlers/android/ (stubs)
```

The `<platform>` directories grow new entries; no existing files move.

## Why this is its own task, not a hot-fix

- The Windows handler implementation is non-trivial: WinUI 3 requires
  `Application::Start` to be the *outermost* call on the main thread, so
  `mpapp::run<App>` cannot just spawn a thread and call the user's
  `on_launch()` — the App object must subclass `mux::ApplicationT<App>`
  in-place. The handler hides that, but designing the hide is real work.
- `application` is a singleton on Windows (only one `mux::Application`
  per process) but MAY become multi-instance on Linux/macOS — that's an
  ADR-0012 decision, not an in-line refactor.
- The `view*` reference in `window.content` raises an ownership question
  the existing widget headers don't have to answer (currently every
  control is stack-allocated by the user); `window_handler<windows>` has
  to keep the WinUI tree alive across user-code rebinds.

ADR-0012 captures these decisions before the implementation lands.
