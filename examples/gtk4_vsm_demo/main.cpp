// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. RFC-0006 Visual State Manager — GTK4 demo.
//
// Renders a target button + status label, and 3 driver buttons that
// explicitly transition the VSM through Normal / Pressed / Disabled.
// Each state's setters mutate the target button's text + the status
// label's text — providing a visible, screenshottable end-to-end
// proof of the mock-surface VSM driving real GTK widgets.
//
// Why explicit driver buttons rather than wiring Pressed automatically
// to the real button's press event? The per-platform auto-route of
// system input events to VSM states is captured as RFC-0006 follow-up
// work — the mock surface deliberately ships state transitions as a
// public API users / handlers drive directly.

#include <cstdio>
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

// Set by `main` from argv[1] so the on_launch path can apply the
// requested initial state — lets a screenshot harness drive each
// state from outside without input automation.
std::string initial_state_arg{};

class vsm_demo_app : public mpapp::application {
public:
    void on_launch() override {
        // ---- VSM setup ----
        configure_vsm();

        // ---- Widget initial values + handler wiring ----
        target_.text = "Target — Normal";
        status_.text = "Current state: (no transitions yet)";

        normal_btn_.text   = "go_to_state Normal";
        pressed_btn_.text  = "go_to_state Pressed";
        disabled_btn_.text = "go_to_state Disabled";

        status_.set_handler(status_handler_);
        outer_.set_handler(outer_handler_);
        driver_row_.set_handler(driver_row_handler_);

        status_handler_.map_text(status_);

        // ---- Subscribe driver buttons to state transitions ----
        normal_btn_.clicked.subscribe(normal_slot_,   normal_cb_);
        pressed_btn_.clicked.subscribe(pressed_slot_, pressed_cb_);
        disabled_btn_.clicked.subscribe(disabled_slot_, disabled_cb_);

        // ---- Layout: vertical column with the target + status on top,
        //      the three driver buttons in a horizontal row below. ----
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

        // ---- Window ----
        window_.title  = "MPAPP RFC-0006 - Visual State Manager (GTK4)";
        window_.width  = 560;
        window_.height = 260;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &outer_;
        window_.show();

        // Optional: if argv supplied an initial state, transition
        // immediately so a non-interactive screenshot run captures
        // the post-transition window without needing input
        // automation.
        if (!initial_state_arg.empty()) {
            const int transitioned =
                vsm_.go_to_state(target_, std::string_view{ initial_state_arg });
            // Textual proof of the transition — picked up by the
            // capture harness when WSLg's DComp wall blocks BitBlt
            // (msrdc COPY MODE on Win11 makes PrintWindow return a
            // black bitmap; the textual line below is the closure
            // evidence in that case).
            std::fprintf(stderr,
                         "VSM-SMOKE: state=%s transitioned=%d target.text=%s status.text=%s\n",
                         initial_state_arg.c_str(),
                         transitioned,
                         target_.text.get().c_str(),
                         status_.text.get().c_str());
            std::fflush(stderr);
        }
    }

private:
    // Build the VSM: 1 group ("Common") with 3 states. Each state's
    // setter mutates the target button's text + the status label's
    // text so the transition is visible.
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
            // We capture `this` so the setter can update the status
            // label too — pure RFC-0006 "function<void(view&)>" shape.
            g.states.back().setters["target.text"] =
                [this, state_name](mpapp::view&) {
                    target_.text.set("Target — " + state_name);
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

    mpapp::button       target_{};         // wrapper-component — owns its handler
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
    // Use an environment variable rather than argv[1] — GTK4's
    // GApplication consumes leftover argv as file-open paths and
    // exits early when it can't open them. Env-var routing keeps
    // the demo runnable from a screenshot harness without
    // poisoning the GTK arg parser.
    if (const char* envv = std::getenv("MPAPP_VSM_INITIAL_STATE"); envv && *envv) {
        initial_state_arg = envv;
    }
    return mpapp::run<vsm_demo_app>(argc, argv);
}
