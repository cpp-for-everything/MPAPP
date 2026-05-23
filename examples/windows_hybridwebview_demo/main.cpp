// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0027 — HybridWebView end-to-end demo (Windows/WebView2).
//
// Mirror of gtk4_hybridwebview_demo. See that file's header for the
// full surface description.

#include <functional>
#include <string>
#include <utility>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/hybrid_bridge.hpp>
#include <mpapp/hybrid_web_view.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/windows/button_handler.hpp>
#include <mpapp/handlers/windows/hybrid_web_view_handler.hpp>
#include <mpapp/handlers/windows/label_handler.hpp>
#include <mpapp/handlers/windows/stack_layout_handler.hpp>
#include <mpapp/handlers/windows/window_handler.hpp>

namespace {

using wp = mpapp::platform::windows;

constexpr const char* kIndexHtml = R"(<!doctype html>
<html><body style="font-family:sans-serif;padding:24px;background:#264653;color:#F4A261">
<h1>T-0027 HybridWebView demo</h1>
<p id="reply">(waiting for C++ to send a message)</p>
<p style="opacity:.7">The page below auto-calls <code>window.mpapp.call('notify',...)</code> on load.</p>
<script>
  function init() {
    if (!window.mpapp) {
      setTimeout(init, 50);
      return;
    }
    window.mpapp.register('show', function(msg) {
      document.getElementById('reply').textContent = 'from C++: ' + msg;
      return 'ok';
    });
    window.mpapp.call('notify', 'page loaded ' + new Date().toISOString());
  }
  init();
</script>
</body></html>
)";

class notify_bridge : public mpapp::hybrid_bridge {
public:
    using callback_t = std::function<void(const std::string&)>;

    explicit notify_bridge(callback_t cb) : on_event_(std::move(cb)) {
        register_method("notify", &notify_bridge::notify);
    }

    std::string notify(const std::string& msg) {
        if (on_event_) on_event_(msg);
        return std::string{"ack:"} + msg;
    }

private:
    callback_t on_event_;
};

class hybridwv_demo_app : public mpapp::application {
public:
    void on_launch() override {
        hwv_.set_hwv_handler(hwv_handler_);
        hwv_handler_.map_messages(hwv_);
        hwv_handler_.map_html_source(hwv_);

        hwv_.set_bridge<notify_bridge>([this](const std::string& msg) {
            ++this->bridge_calls_;
            this->last_event_ = msg;
            this->refresh_status();
        });
        hwv_.html_source = kIndexHtml;

        send_btn_.set_handler(send_btn_handler_);
        send_btn_handler_.map_text(send_btn_);
        send_btn_handler_.map_clicked(send_btn_);
        send_btn_.text = "Send 'hello from C++' to JS";
        send_btn_.clicked.subscribe(send_slot_, send_cb_);

        status_label_.set_handler(status_label_handler_);
        status_label_handler_.map_text(status_label_);
        refresh_status();

        layout_.set_handler(layout_handler_);
        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing = 8.0;
        layout_.padding = mpapp::thickness{16.0};
        layout_.add(status_label_);
        layout_.add(send_btn_);
        layout_.add(hwv_);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0027 - HybridWebView Demo (WebView2)";
        window_.width  = 800;
        window_.height = 600;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    void refresh_status() {
        status_label_.text =
            "bridge_calls: " + std::to_string(bridge_calls_) +
            "   last_js_event: " + last_event_ +
            "   cpp_to_js_sends: " + std::to_string(cpp_to_js_sends_);
    }

    void send_to_js() {
        ++cpp_to_js_sends_;
        hwv_.invoke_js("show",
                       std::string{"hello from C++ ("} +
                       std::to_string(cpp_to_js_sends_) + ")");
        refresh_status();
    }

    struct send_cb_t {
        hybridwv_demo_app* self;
        void operator()() const { self->send_to_js(); }
    };

    int bridge_calls_     = 0;
    int cpp_to_js_sends_  = 0;
    std::string last_event_ = "(none)";

    mpapp::hybrid_web_view                hwv_{};
    mpapp::hybrid_web_view_handler<wp>    hwv_handler_{};

    mpapp::button                         send_btn_{};
    mpapp::button_handler<wp>             send_btn_handler_{};

    mpapp::label                          status_label_{};
    mpapp::label_handler<wp>              status_label_handler_{};

    mpapp::stack_layout                   layout_{};
    mpapp::stack_layout_handler<wp>       layout_handler_{};
    mpapp::window                         window_{};
    mpapp::window_handler<wp>             window_handler_{};

    send_cb_t                              send_cb_{this};
    mpapp::signal_slot<>                   send_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<hybridwv_demo_app>(argc, argv);
}
