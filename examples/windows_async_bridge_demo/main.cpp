// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0018 — async bridge dispatch demo (Windows/WinUI 3).
//
// Sibling of gtk4_async_bridge_demo. Same demo_bridge + UI shape +
// envelope text; different per-platform handler set.

#include <functional>
#include <string>
#include <utility>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/hybrid_bridge.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

class demo_bridge : public mpapp::hybrid_bridge {
public:
    demo_bridge() {
        register_method("add_sync",        &demo_bridge::add_sync);
        register_async_method<int>("add_async_inline",
                                   &demo_bridge::add_async_inline);
        register_async_method<int>("defer_add",
                                   &demo_bridge::defer_add);
    }

    int add_sync(int a, int b) { return a + b; }

    void add_async_inline(int a, int b, std::function<void(int)> respond) {
        respond(a + b);
    }

    void defer_add(int a, int b, std::function<void(int)> respond) {
        pending_a_ = a;
        pending_b_ = b;
        pending_respond_ = std::move(respond);
    }

    void resolve_pending() {
        if (!pending_respond_) return;
        auto cb = std::move(pending_respond_);
        pending_respond_ = nullptr;
        cb(pending_a_ + pending_b_);
    }

    bool has_pending() const { return static_cast<bool>(pending_respond_); }

private:
    int                          pending_a_ = 0;
    int                          pending_b_ = 0;
    std::function<void(int)>     pending_respond_;
};

class async_bridge_demo_app : public mpapp::application {
public:
    void on_launch() override {
        bind_label(last_request_label_,  last_request_label_handler_,
                   "last_request: (none)");
        bind_label(last_response_label_, last_response_label_handler_,
                   "last_response: (none)");
        bind_label(pending_label_,       pending_label_handler_,
                   "pending_count: 0");

        bind_button(btn_sync_,
                    "dispatch  add_sync(2, 3)",
                    btn_sync_slot_, btn_sync_cb_);
        bind_button(btn_async_inline_,
                    "dispatch_async  add_async_inline(10, 20)",
                    btn_async_inline_slot_, btn_async_inline_cb_);
        bind_button(btn_async_defer_,
                    "dispatch_async  defer_add(7, 8)",
                    btn_async_defer_slot_, btn_async_defer_cb_);
        bind_button(btn_resolve_,
                    "resolve pending  defer_add",
                    btn_resolve_slot_, btn_resolve_cb_);

        layout_.set_handler(layout_handler_);
        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing           = 8.0;
        layout_.padding           = mpapp::thickness{16.0};
        layout_.add(last_request_label_);
        layout_.add(last_response_label_);
        layout_.add(pending_label_);
        layout_.add(btn_sync_);
        layout_.add(btn_async_inline_);
        layout_.add(btn_async_defer_);
        layout_.add(btn_resolve_);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0018 - Async Bridge Demo (WinUI 3)";
        window_.width  = 620;
        window_.height = 360;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    using wp = mpapp::platform::current;

    void bind_label(mpapp::label& lbl, mpapp::label_handler<wp>& h,
                    const std::string& text) {
        lbl.set_handler(h);
        h.map_text(lbl);
        lbl.text = text;
    }
    template <class Cb>
    void bind_button(mpapp::button& btn,
                     const std::string& text,
                     mpapp::signal_slot<>& slot, Cb& cb) {
        // `mpapp::button` auto-binds its embedded handler in its ctor;
        // this helper just sets the label + click subscription.
        btn.text = text;
        btn.clicked.subscribe(slot, cb);
    }

    void set_request(const std::string& env) {
        last_request_label_.text = "last_request:  " + env;
    }
    void set_response(const std::string& env) {
        last_response_label_.text = "last_response: " + env;
    }
    void set_pending_count(int n) {
        pending_label_.text = "pending_count: " + std::to_string(n);
    }

    struct btn_sync_cb_t {
        async_bridge_demo_app* self;
        void operator()() const {
            const std::string envelope =
                R"({"id":1,"method":"add_sync","args":[2,3]})";
            self->set_request(envelope);
            std::string out;
            self->bridge_.dispatch(envelope, out);
            self->set_response(out);
        }
    };

    struct btn_async_inline_cb_t {
        async_bridge_demo_app* self;
        void operator()() const {
            const std::string envelope =
                R"({"id":2,"method":"add_async_inline","args":[10,20]})";
            self->set_request(envelope);
            self->bridge_.dispatch_async(envelope,
                                         [self = self->capture()](std::string r) {
                                             self->set_response(r);
                                         });
        }
    };

    struct btn_async_defer_cb_t {
        async_bridge_demo_app* self;
        void operator()() const {
            const std::string envelope =
                R"({"id":3,"method":"defer_add","args":[7,8]})";
            self->set_request(envelope);
            self->set_response("(pending - click 'resolve' to fire)");
            self->bridge_.dispatch_async(envelope,
                                         [self = self->capture()](std::string r) {
                                             self->set_response(r);
                                             self->set_pending_count(0);
                                         });
            self->set_pending_count(self->bridge_.has_pending() ? 1 : 0);
        }
    };

    struct btn_resolve_cb_t {
        async_bridge_demo_app* self;
        void operator()() const {
            if (!self->bridge_.has_pending()) {
                self->set_response("(nothing pending - click defer first)");
                return;
            }
            self->bridge_.resolve_pending();
        }
    };

    async_bridge_demo_app* capture() { return this; }

    demo_bridge bridge_{};

    mpapp::label last_request_label_{};
    mpapp::label last_response_label_{};
    mpapp::label pending_label_{};

    mpapp::button btn_sync_{};
    mpapp::button btn_async_inline_{};
    mpapp::button btn_async_defer_{};
    mpapp::button btn_resolve_{};

    mpapp::stack_layout layout_{};
    mpapp::window       window_{};

    mpapp::label_handler<wp>        last_request_label_handler_{};
    mpapp::label_handler<wp>        last_response_label_handler_{};
    mpapp::label_handler<wp>        pending_label_handler_{};
    mpapp::stack_layout_handler<wp> layout_handler_{};
    mpapp::window_handler<wp>       window_handler_{};

    btn_sync_cb_t          btn_sync_cb_{this};
    btn_async_inline_cb_t  btn_async_inline_cb_{this};
    btn_async_defer_cb_t   btn_async_defer_cb_{this};
    btn_resolve_cb_t       btn_resolve_cb_{this};

    mpapp::signal_slot<> btn_sync_slot_{};
    mpapp::signal_slot<> btn_async_inline_slot_{};
    mpapp::signal_slot<> btn_async_defer_slot_{};
    mpapp::signal_slot<> btn_resolve_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<async_bridge_demo_app>(argc, argv);
}
