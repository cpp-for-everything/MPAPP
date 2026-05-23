// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0021 — ShapeView demo (Linux/GTK4).
//
// Visible exercise of `mpapp::shape_view` — 2D vector primitive with
// kind / data / fill / stroke / stroke_thickness / opacity observables.
// One ShapeView per row, three rows demonstrating rectangle, ellipse,
// and path renderings through the GTK4 native handler.

#include <string>

#include <mpapp/application.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/shape_view.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/linux/label_handler.hpp>
#include <mpapp/handlers/linux/shape_view_handler.hpp>
#include <mpapp/handlers/linux/stack_layout_handler.hpp>
#include <mpapp/handlers/linux/window_handler.hpp>

namespace {

using lp = mpapp::platform::linux_;

class shapeview_demo_app : public mpapp::application {
public:
    void on_launch() override {
        // ---- Three shapes: rectangle, ellipse, path -----------------
        bind_shape(rect_,    rect_handler_,    mpapp::shape_kind::rectangle,
                   "",                              "#E63946", "#1D3557", 3.0);
        bind_shape(ellipse_, ellipse_handler_, mpapp::shape_kind::ellipse,
                   "",                              "#2A9D8F", "#1D3557", 3.0);
        bind_shape(path_,    path_handler_,    mpapp::shape_kind::path,
                   "M20 10 L60 10 L40 50 Z",        "#F4A261", "#1D3557", 3.0);

        bind_label(label_rect_,    label_rect_handler_,    "rectangle");
        bind_label(label_ellipse_, label_ellipse_handler_, "ellipse");
        bind_label(label_path_,    label_path_handler_,    "path (triangle)");

        layout_.set_handler(layout_handler_);
        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing           = 8.0;
        layout_.padding           = mpapp::thickness{16.0};
        layout_.add(label_rect_);
        layout_.add(rect_);
        layout_.add(label_ellipse_);
        layout_.add(ellipse_);
        layout_.add(label_path_);
        layout_.add(path_);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0021 - ShapeView Demo (GTK4)";
        window_.width  = 360;
        window_.height = 520;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    void bind_shape(mpapp::shape_view& s, mpapp::shape_view_handler<lp>& h,
                    mpapp::shape_kind k, const std::string& data,
                    const std::string& fill, const std::string& stroke,
                    double thickness) {
        s.set_sv_handler(h);
        h.map_kind(s);
        h.map_data(s);
        h.map_fill(s);
        h.map_stroke(s);
        h.map_stroke_thickness(s);
        h.map_opacity(s);
        s.kind             = k;
        s.data             = data;
        s.fill             = fill;
        s.stroke           = stroke;
        s.stroke_thickness = thickness;
        s.opacity          = 1.0;
    }
    void bind_label(mpapp::label& lbl, mpapp::label_handler<lp>& h, const std::string& text) {
        lbl.set_handler(h);
        h.map_text(lbl);
        lbl.text = text;
    }

    mpapp::shape_view                rect_{};
    mpapp::shape_view                ellipse_{};
    mpapp::shape_view                path_{};
    mpapp::shape_view_handler<lp>    rect_handler_{};
    mpapp::shape_view_handler<lp>    ellipse_handler_{};
    mpapp::shape_view_handler<lp>    path_handler_{};

    mpapp::label                     label_rect_{};
    mpapp::label                     label_ellipse_{};
    mpapp::label                     label_path_{};
    mpapp::label_handler<lp>         label_rect_handler_{};
    mpapp::label_handler<lp>         label_ellipse_handler_{};
    mpapp::label_handler<lp>         label_path_handler_{};

    mpapp::stack_layout              layout_{};
    mpapp::stack_layout_handler<lp>  layout_handler_{};
    mpapp::window                    window_{};
    mpapp::window_handler<lp>        window_handler_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<shapeview_demo_app>(argc, argv);
}
