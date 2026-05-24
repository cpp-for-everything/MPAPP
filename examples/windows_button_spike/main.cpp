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

#include <cctype>
#include <string>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/entry.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/switch_.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/button_handler.hpp>
#include <mpapp/handlers/entry_handler.hpp>
#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/switch_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

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
        name_.set_handler(name_handler_);
        shout_.set_handler(shout_handler_);
        layout_.set_handler(layout_handler_);

        // 2. Initial property values via the MPAPP surface.
        btn_.text         = "Click me";
        lbl_.text         = "Count: 0 — hello, world";
        name_.placeholder = "Type your name";
        shout_.is_on      = false;

        // 3. Map properties + events onto native widgets.
        btn_handler_.map_text(btn_);
        btn_handler_.map_clicked(btn_);
        lbl_handler_.map_text(lbl_);
        name_handler_.map_text(name_);
        name_handler_.map_placeholder(name_);
        shout_handler_.map_is_on(shout_);

        // 4. VM ↔ view wiring through cross-platform signals only.
        btn_.clicked.subscribe(click_slot_, click_cb_);
        vm_.count.changed.subscribe(count_slot_, count_cb_);
        name_.text.changed.subscribe(name_slot_, name_cb_);
        shout_.is_on.changed.subscribe(shout_slot_, shout_cb_);

        // 5. Compose the layout — orientation / spacing / padding /
        //    alignment all in cross-platform terms.
        layout_.stack_orientation    = mpapp::orientation::vertical;
        layout_.spacing              = 12.0;
        layout_.padding              = mpapp::thickness{24.0};
        layout_.horizontal_alignment = mpapp::h_align::center;
        layout_.vertical_alignment   = mpapp::v_align::center;
        layout_.add(lbl_);
        layout_.add(name_);
        layout_.add(shout_);
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

    static std::string greeting(const std::string& name, bool shout) {
        const std::string who = name.empty() ? std::string{"world"} : name;
        std::string g = "hello, " + who;
        if (shout) {
            for (auto& c : g) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            g += "!!!";
        }
        return g;
    }

    void render_label() {
        const int n = vm_.count.get();
        lbl_.text.set("Count: " + std::to_string(n) + " — "
                      + greeting(name_.text.get(), shout_.is_on.get()));
    }

    struct count_cb_t {
        spike_app* self;
        void operator()(int) const { self->render_label(); }
    };

    struct name_cb_t {
        spike_app* self;
        void operator()(const std::string&) const { self->render_label(); }
    };

    struct shout_cb_t {
        spike_app* self;
        void operator()(bool) const { self->render_label(); }
    };

    view_model              vm_{};
    mpapp::button           btn_{};
    mpapp::label            lbl_{};
    mpapp::entry            name_{};
    mpapp::switch_          shout_{};
    mpapp::stack_layout     layout_{};
    mpapp::window           window_{};

    mpapp::button_handler<mpapp::platform::current>       btn_handler_{};
    mpapp::label_handler<mpapp::platform::current>        lbl_handler_{};
    mpapp::entry_handler<mpapp::platform::current>        name_handler_{};
    mpapp::switch_handler<mpapp::platform::current>       shout_handler_{};
    mpapp::stack_layout_handler<mpapp::platform::current> layout_handler_{};
    mpapp::window_handler<mpapp::platform::current>       window_handler_{};

    click_cb_t                             click_cb_{this};
    count_cb_t                             count_cb_{this};
    name_cb_t                              name_cb_{this};
    shout_cb_t                             shout_cb_{this};
    mpapp::signal_slot<>                   click_slot_{};
    mpapp::signal_slot<const int&>         count_slot_{};
    mpapp::signal_slot<const std::string&> name_slot_{};
    mpapp::signal_slot<const bool&>        shout_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<spike_app>(argc, argv);
}
