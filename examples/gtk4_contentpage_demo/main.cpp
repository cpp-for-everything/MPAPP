// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0025 — ContentPage demo (Linux/GTK4).
//
// Visible exercise of `mpapp::content_page` — title + content
// Observable<shared_ptr<view>> + thickness padding. The page hosts
// a stack_layout containing a label + button. Set as window.content.

#include <memory>
#include <string>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/content_page.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/linux/button_handler.hpp>
#include <mpapp/handlers/linux/content_page_handler.hpp>
#include <mpapp/handlers/linux/label_handler.hpp>
#include <mpapp/handlers/linux/stack_layout_handler.hpp>
#include <mpapp/handlers/linux/window_handler.hpp>

namespace {

using lp = mpapp::platform::linux_;

class contentpage_demo_app : public mpapp::application {
public:
    void on_launch() override {
        // ---- Build the inner content: label + button ----------------
        label_.text = "This page is hosted in an mpapp::content_page";
        button_.text = "Click me";

        label_.set_handler(label_handler_);
        button_.set_handler(button_handler_);
        layout_.set_handler(layout_handler_);

        label_handler_.map_text(label_);
        button_handler_.map_text(button_);
        button_handler_.map_clicked(button_);

        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing = 12.0;
        layout_.padding = mpapp::thickness{0.0};   // outer padding lives on the content_page
        layout_.add(label_);
        layout_.add(button_);
        layout_handler_.bind(layout_);

        // ---- ContentPage --------------------------------------------
        cp_.set_handler(cp_handler_);
        cp_handler_.map_title(cp_);
        cp_handler_.map_content(cp_);
        cp_handler_.map_padding(cp_);
        cp_.title   = "Content Page";
        cp_.padding = mpapp::thickness{24.0};   // honored by the handler
        cp_.content = std::shared_ptr<mpapp::view>(&layout_, [](mpapp::view*){});

        window_.title  = "MPAPP T-0025 - ContentPage Demo (GTK4)";
        window_.width  = 480;
        window_.height = 300;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &cp_;
        window_.show();
    }

private:
    mpapp::label                       label_{};
    mpapp::button                      button_{};
    mpapp::stack_layout                layout_{};

    mpapp::label_handler<lp>           label_handler_{};
    mpapp::button_handler<lp>          button_handler_{};
    mpapp::stack_layout_handler<lp>    layout_handler_{};

    mpapp::content_page                cp_{};
    mpapp::content_page_handler<lp>    cp_handler_{};

    mpapp::window                      window_{};
    mpapp::window_handler<lp>          window_handler_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<contentpage_demo_app>(argc, argv);
}
