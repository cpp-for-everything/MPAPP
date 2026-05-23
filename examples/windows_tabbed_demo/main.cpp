// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0023 — TabbedPage demo (Windows/WinUI 3).

#include <string>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/page.hpp>
#include <mpapp/run.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/tabbed_page.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/windows/button_handler.hpp>
#include <mpapp/handlers/windows/label_handler.hpp>
#include <mpapp/handlers/windows/page_handler.hpp>
#include <mpapp/handlers/windows/stack_layout_handler.hpp>
#include <mpapp/handlers/windows/tabbed_page_handler.hpp>
#include <mpapp/handlers/windows/window_handler.hpp>

namespace {

using wp = mpapp::platform::windows;

struct tab_page {
    mpapp::page                      page_{};
    mpapp::stack_layout              layout_{};
    mpapp::label                     label_{};
    mpapp::button                    button_{};

    mpapp::page_handler<wp>          page_handler_{};
    mpapp::stack_layout_handler<wp>  layout_handler_{};
    mpapp::label_handler<wp>         label_handler_{};
    mpapp::button_handler<wp>        button_handler_{};

    void build(const std::string& title, const std::string& body,
               const std::string& btn_text) {
        page_.title = title;
        label_.text = body;
        button_.text = btn_text;

        label_.set_handler(label_handler_);
        button_.set_handler(button_handler_);
        layout_.set_handler(layout_handler_);
        page_.set_handler(page_handler_);

        label_handler_.map_text(label_);
        button_handler_.map_text(button_);
        button_handler_.map_clicked(button_);

        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing = 12.0;
        layout_.padding = mpapp::thickness{16.0};
        layout_.add(label_);
        layout_.add(button_);
        layout_handler_.bind(layout_);

        page_handler_.map_title(page_);
        page_handler_.map_content(page_);
        page_handler_.bind_content(page_, layout_);
    }
};

class tabbed_demo_app : public mpapp::application {
public:
    void on_launch() override {
        home_.build("Home",   "Welcome to the home tab.", "Click me (Home)");
        about_.build("About", "About this app.",          "Click me (About)");
        settings_.build("Settings", "App settings live here.", "Click me (Settings)");

        tp_.set_tp_handler(tp_handler_);
        tp_handler_.map_children(tp_);
        tp_handler_.map_selected_index(tp_);
        tp_.add_tab(&home_.page_);
        tp_.add_tab(&about_.page_);
        tp_.add_tab(&settings_.page_);

        window_.title  = "MPAPP T-0023 - TabbedPage Demo (WinUI 3)";
        window_.width  = 560;
        window_.height = 380;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &tp_;
        window_.show();
    }

private:
    tab_page home_{};
    tab_page about_{};
    tab_page settings_{};

    mpapp::tabbed_page                tp_{};
    mpapp::tabbed_page_handler<wp>    tp_handler_{};

    mpapp::window                     window_{};
    mpapp::window_handler<wp>         window_handler_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<tabbed_demo_app>(argc, argv);
}
