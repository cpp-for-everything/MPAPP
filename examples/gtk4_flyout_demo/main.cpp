// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0024 — FlyoutPage demo (Linux/GTK4).
//
// `mpapp::flyout_page` — page-level master/detail container with a
// `flyout` Page (the pane), a `detail` Page (the main area), and an
// `is_presented` Observable toggling the flyout open/closed.
//
// Demo wires two pages + a "Toggle flyout" button on the detail page
// that flips fp.is_presented.

#include <string>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/flyout_page.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/page.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/button_handler.hpp>
#include <mpapp/handlers/flyout_page_handler.hpp>
#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/page_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

struct simple_page {
    mpapp::page                      page_{};
    mpapp::stack_layout              layout_{};
    mpapp::label                     label_{};

    mpapp::page_handler<>          page_handler_{};
    mpapp::stack_layout_handler<>  layout_handler_{};
    mpapp::label_handler<>         label_handler_{};

    void build(const std::string& title, const std::string& body) {
        page_.title = title;
        label_.text = body;

        label_.set_handler(label_handler_);
        layout_.set_handler(layout_handler_);
        page_.set_handler(page_handler_);

        label_handler_.map_text(label_);

        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing = 12.0;
        layout_.padding = mpapp::thickness{16.0};
        layout_.add(label_);
        layout_handler_.bind(layout_);

        page_handler_.map_title(page_);
        page_handler_.map_content(page_);
        page_handler_.bind_content(page_, layout_);
    }
};

struct detail_page_with_button {
    mpapp::page                      page_{};
    mpapp::stack_layout              layout_{};
    mpapp::label                     label_{};
    mpapp::button                    toggle_btn_{};

    mpapp::page_handler<>          page_handler_{};
    mpapp::stack_layout_handler<>  layout_handler_{};
    mpapp::label_handler<>         label_handler_{};
    mpapp::button_handler<>        toggle_btn_handler_{};

    void build(const std::string& title, const std::string& body,
               const std::string& btn_text) {
        page_.title = title;
        label_.text = body;
        toggle_btn_.text = btn_text;

        label_.set_handler(label_handler_);
        toggle_btn_.set_handler(toggle_btn_handler_);
        layout_.set_handler(layout_handler_);
        page_.set_handler(page_handler_);

        label_handler_.map_text(label_);
        toggle_btn_handler_.map_text(toggle_btn_);
        toggle_btn_handler_.map_clicked(toggle_btn_);

        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing = 12.0;
        layout_.padding = mpapp::thickness{16.0};
        layout_.add(label_);
        layout_.add(toggle_btn_);
        layout_handler_.bind(layout_);

        page_handler_.map_title(page_);
        page_handler_.map_content(page_);
        page_handler_.bind_content(page_, layout_);
    }
};

class flyout_demo_app : public mpapp::application {
public:
    void on_launch() override {
        flyout_.build("Menu", "(flyout pane)");
        detail_.build("Detail", "Detail content here. Click the button to toggle the flyout.",
                      "Toggle flyout");

        detail_.toggle_btn_.clicked.subscribe(toggle_slot_, toggle_cb_);

        fp_.set_fp_handler(fp_handler_);
        fp_handler_.map_flyout(fp_);
        fp_handler_.map_detail(fp_);
        fp_handler_.map_is_presented(fp_);
        fp_.flyout       = &flyout_.page_;
        fp_.detail       = &detail_.page_;
        fp_.is_presented = true;   // start with flyout visible

        window_.title  = "MPAPP T-0024 - FlyoutPage Demo (GTK4)";
        window_.width  = 640;
        window_.height = 360;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &fp_;
        window_.show();
    }

private:
    struct toggle_cb_t {
        flyout_demo_app* self;
        void operator()() const { self->fp_.toggle(); }
    };

    simple_page                  flyout_{};
    detail_page_with_button      detail_{};

    mpapp::flyout_page                fp_{};
    mpapp::flyout_page_handler<>    fp_handler_{};

    mpapp::window                     window_{};
    mpapp::window_handler<>         window_handler_{};

    toggle_cb_t                        toggle_cb_{this};
    mpapp::signal_slot<>              toggle_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<flyout_demo_app>(argc, argv);
}
