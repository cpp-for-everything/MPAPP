// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0026 — WebView demo (Windows/WebView2).

#include <string>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/web_view.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/button_handler.hpp>
#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/web_view_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

using wp = mpapp::platform::current;

constexpr const char* kPageOne =
    "<!doctype html><html><body style='font-family:sans-serif;padding:24px;background:#F4A261;color:#1D3557'>"
    "<h1>MPAPP T-0026 &mdash; WebView demo</h1>"
    "<p>This page is loaded via <code>html_source</code> on the cross-platform <code>mpapp::web_view</code> surface.</p>"
    "<p>The platform handler binds to WebKitGTK 6.x on Linux, WebView2 on Windows, and <code>android.webkit.WebView</code> on Android.</p>"
    "</body></html>";

constexpr const char* kPageTwo =
    "<!doctype html><html><body style='font-family:sans-serif;padding:24px;background:#2A9D8F;color:#FFFFFF'>"
    "<h1>Page two</h1>"
    "<p>html_source.changed fired &mdash; the handler reloaded the content.</p>"
    "<ul><li>Same WebView control</li><li>New page content</li><li>Native widget never recreated</li></ul>"
    "</body></html>";

class webview_demo_app : public mpapp::application {
public:
    void on_launch() override {
        wv_.set_wv_handler(wv_handler_);
        wv_handler_.map_url(wv_);
        wv_handler_.map_html(wv_);
        wv_.html_source = kPageOne;

        wv_.navigated.subscribe(nav_slot_, nav_cb_);
        wv_.is_loading.changed.subscribe(loading_slot_, loading_cb_);

        toggle_btn_.set_handler(toggle_btn_handler_);
        toggle_btn_handler_.map_text(toggle_btn_);
        toggle_btn_handler_.map_clicked(toggle_btn_);
        toggle_btn_.text = "Toggle content";
        toggle_btn_.clicked.subscribe(toggle_slot_, toggle_cb_);

        status_label_.set_handler(status_label_handler_);
        status_label_handler_.map_text(status_label_);
        refresh_status();

        layout_.set_handler(layout_handler_);
        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing = 8.0;
        layout_.padding = mpapp::thickness{16.0};
        layout_.add(status_label_);
        layout_.add(toggle_btn_);
        layout_.add(wv_);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0026 - WebView Demo (WebView2)";
        window_.width  = 760;
        window_.height = 560;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    void refresh_status() {
        status_label_.text =
            std::string{"is_loading: "} + (wv_.is_loading.get() ? "true " : "false") +
            "   last_nav_url: " + last_nav_url_ +
            "   page: " + (showing_page_two_ ? "two" : "one");
    }

    void toggle() {
        showing_page_two_ = !showing_page_two_;
        wv_.html_source = (showing_page_two_ ? kPageTwo : kPageOne);
    }

    struct toggle_cb_t {
        webview_demo_app* self;
        void operator()() const { self->toggle(); }
    };
    struct nav_cb_t {
        webview_demo_app* self;
        void operator()(const std::string& url, bool) const {
            self->last_nav_url_ = url;
            self->refresh_status();
        }
    };
    struct loading_cb_t {
        webview_demo_app* self;
        void operator()(bool) const { self->refresh_status(); }
    };

    bool showing_page_two_ = false;
    std::string last_nav_url_ = "(none)";

    mpapp::web_view                  wv_{};
    mpapp::web_view_handler<wp>      wv_handler_{};

    mpapp::button                    toggle_btn_{};
    mpapp::button_handler<wp>        toggle_btn_handler_{};

    mpapp::label                     status_label_{};
    mpapp::label_handler<wp>         status_label_handler_{};

    mpapp::stack_layout              layout_{};
    mpapp::stack_layout_handler<wp>  layout_handler_{};
    mpapp::window                    window_{};
    mpapp::window_handler<wp>        window_handler_{};

    toggle_cb_t                                       toggle_cb_{this};
    nav_cb_t                                          nav_cb_{this};
    loading_cb_t                                      loading_cb_{this};
    mpapp::signal_slot<>                              toggle_slot_{};
    mpapp::signal_slot<const std::string&, bool>      nav_slot_{};
    mpapp::signal_slot<const bool&>                   loading_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<webview_demo_app>(argc, argv);
}
