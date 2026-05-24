// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0022 — GraphicsView demo (Linux/GTK4).
//
// Exercises `mpapp::graphics_view` — Skia-style canvas surface with
// width/height observables, an invalidate() trigger that bumps the
// `draw_count` Observable, and a `draw_requested` signal apps subscribe
// to for issuing draw commands. v1 handler just maps width/height to
// the platform drawing area's content size; real Cairo/Direct2D draws
// are the v2 follow-up tied to the canvas facade migration.

#include <string>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/graphics_view.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/graphics_view_handler.hpp>
#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

class graphicsview_demo_app : public mpapp::application {
public:
    void on_launch() override {
        // ---- GraphicsView + handler --------------------------------
        gv_.set_gv_handler(gv_handler_);
        gv_handler_.map_size(gv_);
        gv_handler_.map_draw_count(gv_);
        gv_.width  = 280;
        gv_.height = 120;

        // Subscribe to draw_requested + draw_count so the status label
        // reflects pulse counts.
        gv_.draw_requested.subscribe(req_slot_, req_cb_);
        gv_.draw_count.changed.subscribe(count_slot_, count_cb_);

        invalidate_btn_.text = "gv.invalidate()";
        invalidate_btn_.clicked.subscribe(invalidate_slot_, invalidate_cb_);

        status_label_.set_handler(status_label_handler_);
        status_label_handler_.map_text(status_label_);
        refresh_status();

        layout_.set_handler(layout_handler_);
        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing           = 8.0;
        layout_.padding           = mpapp::thickness{16.0};
        layout_.add(status_label_);
        layout_.add(invalidate_btn_);
        layout_.add(gv_);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0022 - GraphicsView Demo (GTK4)";
        window_.width  = 380;
        window_.height = 320;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    void refresh_status() {
        status_label_.text =
            "size: " + std::to_string(gv_.width.get()) + "x" + std::to_string(gv_.height.get()) +
            "   draw_count: " + std::to_string(gv_.draw_count.get()) +
            "   draw_requested fires: " + std::to_string(req_fires_);
    }

    struct invalidate_cb_t {
        graphicsview_demo_app* self;
        void operator()() const { self->gv_.invalidate(); }
    };
    struct req_cb_t {
        graphicsview_demo_app* self;
        void operator()() const { ++self->req_fires_; self->refresh_status(); }
    };
    struct count_cb_t {
        graphicsview_demo_app* self;
        void operator()(std::size_t /*n*/) const { self->refresh_status(); }
    };

    int req_fires_ = 0;

    mpapp::graphics_view                 gv_{};
    mpapp::graphics_view_handler<>     gv_handler_{};

    mpapp::button                        invalidate_btn_{};

    mpapp::label                         status_label_{};
    mpapp::label_handler<>             status_label_handler_{};

    mpapp::stack_layout                  layout_{};
    mpapp::stack_layout_handler<>      layout_handler_{};
    mpapp::window                        window_{};
    mpapp::window_handler<>            window_handler_{};

    invalidate_cb_t                       invalidate_cb_{this};
    req_cb_t                              req_cb_{this};
    count_cb_t                            count_cb_{this};
    mpapp::signal_slot<>                  invalidate_slot_{};
    mpapp::signal_slot<>                  req_slot_{};
    mpapp::signal_slot<const std::size_t&> count_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<graphicsview_demo_app>(argc, argv);
}
