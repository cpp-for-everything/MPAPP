// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. RFC-0006 Visual State Manager — WinUI 3 demo.
//
// Identical structure to examples/gtk4_vsm_demo/main.cpp — same VSM
// + same widget composition, just compiled against the Windows
// handler set. Proves the mock surface is portable end-to-end.

#include <cstdlib>
#include <string>
#include <string_view>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/resources/visual_state_manager.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

// Set by `main` from argv[1] so on_launch can apply the
// requested initial state — lets a non-interactive screenshot
// harness drive each state from outside.
std::string initial_state_arg{};

class vsm_demo_app : public mpapp::application {
public:
    void on_launch() override {
        configure_vsm();

        target_.text = "Target — Normal";
        status_.text = "Current state: (no transitions yet)";

        normal_btn_.text   = "go_to_state Normal";
        pressed_btn_.text  = "go_to_state Pressed";
        disabled_btn_.text = "go_to_state Disabled";

        status_.set_handler(status_handler_);
        outer_.set_handler(outer_handler_);
        driver_row_.set_handler(driver_row_handler_);

        status_handler_.map_text(status_);

        normal_btn_.clicked.subscribe(normal_slot_,   normal_cb_);
        pressed_btn_.clicked.subscribe(pressed_slot_, pressed_cb_);
        disabled_btn_.clicked.subscribe(disabled_slot_, disabled_cb_);

        driver_row_.stack_orientation    = mpapp::orientation::horizontal;
        driver_row_.spacing              = 8.0;
        driver_row_.padding              = mpapp::thickness{};
        driver_row_.horizontal_alignment = mpapp::h_align::center;
        driver_row_.vertical_alignment   = mpapp::v_align::center;
        driver_row_.add(normal_btn_);
        driver_row_.add(pressed_btn_);
        driver_row_.add(disabled_btn_);
        driver_row_handler_.bind(driver_row_);

        outer_.stack_orientation    = mpapp::orientation::vertical;
        outer_.spacing              = 16.0;
        outer_.padding              = mpapp::thickness{24.0};
        outer_.horizontal_alignment = mpapp::h_align::center;
        outer_.vertical_alignment   = mpapp::v_align::center;
        outer_.add(target_);
        outer_.add(status_);
        outer_.add(driver_row_);
        outer_handler_.bind(outer_);

        window_.title  = "MPAPP RFC-0006 - Visual State Manager (WinUI 3)";
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &outer_;
        window_.show();

        if (!initial_state_arg.empty()) {
            vsm_.go_to_state(target_, std::string_view{ initial_state_arg });
        }
    }

private:
    void configure_vsm() {
        vsm_.groups.push_back(mpapp::visual_state_group{
            std::string{ "Common" }
        });
        auto& g = vsm_.groups.back();
        for (auto name : { mpapp::visual_states::normal,
                           mpapp::visual_states::pressed,
                           mpapp::visual_states::disabled }) {
            g.states.push_back(mpapp::visual_state{ std::string{ name } });
            const std::string state_name{ name };
            g.states.back().setters["target.text"] =
                [this, state_name](mpapp::view&) {
                    target_.text.set("Target - " + state_name);
                    status_.text.set("Current state: " + state_name);
                };
        }
    }

    struct normal_cb_t {
        vsm_demo_app* self;
        void operator()() const {
            self->vsm_.go_to_state(self->target_,
                                   mpapp::visual_states::normal);
        }
    };
    struct pressed_cb_t {
        vsm_demo_app* self;
        void operator()() const {
            self->vsm_.go_to_state(self->target_,
                                   mpapp::visual_states::pressed);
        }
    };
    struct disabled_cb_t {
        vsm_demo_app* self;
        void operator()() const {
            self->vsm_.go_to_state(self->target_,
                                   mpapp::visual_states::disabled);
        }
    };

    mpapp::visual_state_manager vsm_{};

    mpapp::button       target_{};
    mpapp::label        status_{};
    mpapp::button       normal_btn_{};
    mpapp::button       pressed_btn_{};
    mpapp::button       disabled_btn_{};
    mpapp::stack_layout driver_row_{};
    mpapp::stack_layout outer_{};
    mpapp::window       window_{};

    mpapp::label_handler<>        status_handler_{};
    mpapp::stack_layout_handler<> driver_row_handler_{};
    mpapp::stack_layout_handler<> outer_handler_{};
    mpapp::window_handler<>       window_handler_{};

    normal_cb_t                  normal_cb_{ this };
    pressed_cb_t                 pressed_cb_{ this };
    disabled_cb_t                disabled_cb_{ this };
    mpapp::signal_slot<>         normal_slot_{};
    mpapp::signal_slot<>         pressed_slot_{};
    mpapp::signal_slot<>         disabled_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    // Mirror the GTK4 demo's env-var convention so the same
    // capture harness works on both platforms (Windows doesn't
    // have GApplication's argv-as-files behaviour but the
    // consistent contract simplifies the screenshot tooling).
    if (const char* envv = std::getenv("MPAPP_VSM_INITIAL_STATE"); envv && *envv) {
        initial_state_arg = envv;
    }
    return mpapp::run<vsm_demo_app>(argc, argv);
}
