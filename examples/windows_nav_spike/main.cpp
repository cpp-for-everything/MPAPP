// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Navigation spike — drives the ADR-0014 page_stack
// engine + ADR-0019 push_async/pop_async wrappers in a real WinUI 3
// window.
//
// Two pages: home + details. Pressing "Go to details" on the home page
// calls `co_await nav.push_async(&details)`; pressing "Back" on the
// details page calls `co_await nav.pop_async()`. The navigation bar
// auto-shows its back button when the stack depth crosses 2.

#include <string>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/executor.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/navigation_page.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/page.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/windows/button_handler.hpp>
#include <mpapp/handlers/windows/label_handler.hpp>
#include <mpapp/handlers/windows/navigation_page_handler.hpp>
#include <mpapp/handlers/windows/page_handler.hpp>
#include <mpapp/handlers/windows/stack_layout_handler.hpp>
#include <mpapp/handlers/windows/window_handler.hpp>

namespace {

// Tiny page bundling a stack_layout with a title label + button. Wired
// up the same way the button spike wires its single layout.
struct screen {
    mpapp::page                                              page_{};
    mpapp::stack_layout                                      layout_{};
    mpapp::label                                             title_{};
    mpapp::button                                            action_{};

    mpapp::page_handler<mpapp::platform::windows>            page_handler_{};
    mpapp::stack_layout_handler<mpapp::platform::windows>    layout_handler_{};
    mpapp::label_handler<mpapp::platform::windows>           title_handler_{};
    mpapp::button_handler<mpapp::platform::windows>          action_handler_{};

    void build(const std::string& page_title,
               const std::string& body,
               const std::string& button_text) {
        page_.title  = page_title;
        title_.text  = body;
        action_.text = button_text;

        title_.set_handler(title_handler_);
        action_.set_handler(action_handler_);
        layout_.set_handler(layout_handler_);
        page_.set_handler(page_handler_);

        title_handler_.map_text(title_);
        action_handler_.map_text(action_);
        action_handler_.map_clicked(action_);

        layout_.stack_orientation    = mpapp::orientation::vertical;
        layout_.spacing              = 12.0;
        layout_.padding              = mpapp::thickness{24.0};
        layout_.horizontal_alignment = mpapp::h_align::center;
        layout_.vertical_alignment   = mpapp::v_align::center;
        layout_.add(title_);
        layout_.add(action_);
        layout_handler_.bind(layout_);

        page_handler_.map_title(page_);
        page_handler_.map_content(page_);
        page_handler_.bind_content(page_, layout_);
    }
};

class nav_app : public mpapp::application {
public:
    void on_launch() override {
        home_.build("Home", "You are on the root page.", "Go to details →");
        details_.build("Details", "Pushed onto the navigation stack.", "Back");

        // Wire navigation: home button pushes details, details button pops.
        home_.action_.clicked.subscribe(home_click_slot_, home_click_cb_);
        details_.action_.clicked.subscribe(details_click_slot_, details_click_cb_);

        // Build the NavigationPage with home as the root.
        nav_.set_np_handler(nav_handler_);
        nav_handler_.map_stack(nav_);
        nav_.push(&home_.page_);

        // Window with the NavigationPage as content.
        window_.title = "MPAPP NavigationPage spike (ADR-0014 + ADR-0019)";
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &nav_;
        window_.show();
    }

private:
    // Use a lambda + fire-and-forget approach for async navigation.
    // The eager-start task<T> runs synchronously to completion in the
    // current mock build (per ADR-0019's "wrapper around sync push").
    struct home_click_cb_t {
        nav_app* self;
        void operator()() const {
            // Discard the returned task — body completes synchronously.
            (void)self->nav_.push_async(&self->details_.page_);
        }
    };

    struct details_click_cb_t {
        nav_app* self;
        void operator()() const {
            (void)self->nav_.pop_async();
        }
    };

    screen                                                       home_{};
    screen                                                       details_{};

    mpapp::navigation_page                                       nav_{};
    mpapp::navigation_page_handler<mpapp::platform::windows>     nav_handler_{};

    mpapp::window                                                window_{};
    mpapp::window_handler<mpapp::platform::windows>              window_handler_{};

    home_click_cb_t                                              home_click_cb_{this};
    details_click_cb_t                                           details_click_cb_{this};
    mpapp::signal_slot<>                                         home_click_slot_{};
    mpapp::signal_slot<>                                         details_click_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<nav_app>(argc, argv);
}
