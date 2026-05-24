// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0021 — ShapeView demo (Windows/WinUI 3).

#include <string>

#include <mpapp/application.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/shape_view.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/shape_view_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

using wp = mpapp::platform::current;

class shapeview_demo_app : public mpapp::application {
public:
    void on_launch() override {
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

        window_.title  = "MPAPP T-0021 - ShapeView Demo (WinUI 3)";
        window_.width  = 400;
        window_.height = 540;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    void bind_shape(mpapp::shape_view& s, mpapp::shape_view_handler<wp>& h,
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
    void bind_label(mpapp::label& lbl, mpapp::label_handler<wp>& h, const std::string& text) {
        lbl.set_handler(h);
        h.map_text(lbl);
        lbl.text = text;
    }

    mpapp::shape_view                rect_{};
    mpapp::shape_view                ellipse_{};
    mpapp::shape_view                path_{};
    mpapp::shape_view_handler<wp>    rect_handler_{};
    mpapp::shape_view_handler<wp>    ellipse_handler_{};
    mpapp::shape_view_handler<wp>    path_handler_{};

    mpapp::label                     label_rect_{};
    mpapp::label                     label_ellipse_{};
    mpapp::label                     label_path_{};
    mpapp::label_handler<wp>         label_rect_handler_{};
    mpapp::label_handler<wp>         label_ellipse_handler_{};
    mpapp::label_handler<wp>         label_path_handler_{};

    mpapp::stack_layout              layout_{};
    mpapp::stack_layout_handler<wp>  layout_handler_{};
    mpapp::window                    window_{};
    mpapp::window_handler<wp>        window_handler_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<shapeview_demo_app>(argc, argv);
}
