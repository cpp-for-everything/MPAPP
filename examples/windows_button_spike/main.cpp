// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — rewrite of the T-0003 WinUI 3 button spike
// against the app-shell abstraction.
//
// Goal: zero `winrt::`, `mux::`, `muxc::`, `Mdd*` tokens in this
// translation unit. The platform bootstrap (Application::Start, the
// `mux::ApplicationT<App>` subclass, MddBootstrap, apartment init,
// runtime DLL forwarder) lives inside the WinUI 3 handler set and is
// invoked by `mpapp::run<App>`. User code reads as a portable MPAPP
// program — the same source compiles unmodified once the GTK4 /
// AppKit / UIKit / Android handlers land.
//
// Cross-platform parts (Observable, signal, button.text, label.text,
// view-model wiring) are unchanged from the T-0003 spike. The diff
// between this and the old `main.cpp` is entirely about hiding
// platform-specific bootstrap behind `mpapp::application`,
// `mpapp::window`, `mpapp::stack_layout`, `mpapp::run<App>`.

#include <string>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/windows/button_handler.hpp>
#include <mpapp/handlers/windows/label_handler.hpp>
#include <mpapp/handlers/windows/stack_layout_handler.hpp>
#include <mpapp/handlers/windows/window_handler.hpp>

namespace {

// View-model for the spike — a single Observable<int> `count`.
struct view_model {
    mpapp::Observable<int> count{0};
};

class spike_app : public mpapp::application {
public:
    void on_launch() override {
        // 1. Wire handlers to widgets.
        btn_.set_handler(btn_handler_);
        lbl_.set_handler(lbl_handler_);
        layout_.set_handler(layout_handler_);

        // 2. Initial property values via the MPAPP surface.
        btn_.text = "Click me";
        lbl_.text = "Count: 0";

        // 3. Map text properties + click event onto native widgets.
        btn_handler_.map_text(btn_);
        btn_handler_.map_clicked(btn_);
        lbl_handler_.map_text(lbl_);

        // 4. VM ↔ view wiring through cross-platform signals only.
        btn_.clicked.subscribe(click_slot_, click_cb_);
        vm_.count.changed.subscribe(count_slot_, count_cb_);

        // 5. Compose the layout — orientation / spacing / padding /
        //    alignment all in cross-platform terms.
        layout_.stack_orientation    = mpapp::orientation::vertical;
        layout_.spacing              = 12.0;
        layout_.padding              = mpapp::thickness{24.0};
        layout_.horizontal_alignment = mpapp::h_align::center;
        layout_.vertical_alignment   = mpapp::v_align::center;
        layout_.add(lbl_);
        layout_.add(btn_);

        // Bind the layout handler — pushes the property values into
        // the native StackPanel and replays the child list.
        layout_handler_.bind(layout_);

        // 6. Window setup. `window.content` is a non-owning view*.
        window_.title    = "MPAPP T-0011 - app-shell rewrite";
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);

        // Assign content AFTER binding so the handler's content
        // mapper picks up the change-signal and routes the new value
        // into the native window's Content slot.
        window_.content = &layout_;

        // 7. Show the window.
        window_.show();
    }

private:
    struct click_cb_t {
        spike_app* self;
        void operator()() const {
            self->vm_.count.set(self->vm_.count.get() + 1);
        }
    };

    struct count_cb_t {
        spike_app* self;
        void operator()(int n) const {
            self->lbl_.text.set("Count: " + std::to_string(n));
        }
    };

    view_model              vm_{};
    mpapp::button           btn_{};
    mpapp::label            lbl_{};
    mpapp::stack_layout     layout_{};
    mpapp::window           window_{};

    mpapp::button_handler<mpapp::platform::windows>       btn_handler_{};
    mpapp::label_handler<mpapp::platform::windows>        lbl_handler_{};
    mpapp::stack_layout_handler<mpapp::platform::windows> layout_handler_{};
    mpapp::window_handler<mpapp::platform::windows>       window_handler_{};

    click_cb_t                       click_cb_{this};
    count_cb_t                       count_cb_{this};
    mpapp::signal_slot<>             click_slot_{};
    mpapp::signal_slot<const int&>   count_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<spike_app>(argc, argv);
}
