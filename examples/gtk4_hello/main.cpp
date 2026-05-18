// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 hello-window example, rewritten to use
// the MPAPP app-shell abstraction. This is the Linux counterpart of
// examples/windows_button_spike/main.cpp: the same view-model + UI
// composition code, swapped onto the GTK4 handler set.
//
// Goal: zero `gtk_*`, `GTK_*`, `GtkApplication`, `g_application_run`
// tokens in user-facing code. The platform bootstrap lives inside the
// GTK4 handler set and is invoked by `mpapp::run<App>`.

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

#include <mpapp/handlers/linux/button_handler.hpp>
#include <mpapp/handlers/linux/label_handler.hpp>
#include <mpapp/handlers/linux/stack_layout_handler.hpp>
#include <mpapp/handlers/linux/window_handler.hpp>

namespace {

struct view_model {
    mpapp::Observable<int> count{0};
};

class spike_app : public mpapp::application {
public:
    void on_launch() override {
        btn_.set_handler(btn_handler_);
        lbl_.set_handler(lbl_handler_);
        layout_.set_handler(layout_handler_);

        btn_.text = "Click me";
        lbl_.text = "Count: 0";

        btn_handler_.map_text(btn_);
        btn_handler_.map_clicked(btn_);
        lbl_handler_.map_text(lbl_);

        btn_.clicked.subscribe(click_slot_, click_cb_);
        vm_.count.changed.subscribe(count_slot_, count_cb_);

        layout_.stack_orientation    = mpapp::orientation::vertical;
        layout_.spacing              = 12.0;
        layout_.padding              = mpapp::thickness{24.0};
        layout_.horizontal_alignment = mpapp::h_align::center;
        layout_.vertical_alignment   = mpapp::v_align::center;
        layout_.add(lbl_);
        layout_.add(btn_);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0011 - GTK4 hello (cross-platform spike)";
        window_.width  = 480;
        window_.height = 240;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
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

    mpapp::button_handler<mpapp::platform::linux_>       btn_handler_{};
    mpapp::label_handler<mpapp::platform::linux_>        lbl_handler_{};
    mpapp::stack_layout_handler<mpapp::platform::linux_> layout_handler_{};
    mpapp::window_handler<mpapp::platform::linux_>       window_handler_{};

    click_cb_t                       click_cb_{this};
    count_cb_t                       count_cb_{this};
    mpapp::signal_slot<>             click_slot_{};
    mpapp::signal_slot<const int&>   count_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<spike_app>(argc, argv);
}
