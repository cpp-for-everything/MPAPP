// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0028 closure evidence — GTK4 demo showing all
// four collection_layout values side by side. One screenshot
// captures the full matrix; mirrors the Windows demo of the same
// name.

#include <string>
#include <vector>

#include <mpapp/application.hpp>
#include <mpapp/collection_view.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/run.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/collection_view_handler.hpp>
#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

using lp = mpapp::platform::current;

struct cv_section {
    mpapp::label                       header{};
    mpapp::label_handler<lp>           header_handler{};
    mpapp::collection_view             cv{};
    mpapp::collection_view_handler<lp> cv_handler{};
};

void wire_section(cv_section& s,
                  const std::string& title,
                  const std::vector<std::string>& items,
                  mpapp::collection_layout layout) {
    s.header.set_handler(s.header_handler);
    s.header_handler.map_text(s.header);
    s.header.text = title;

    s.cv.items_source   = items;
    s.cv.layout         = layout;
    s.cv.selection_mode = mpapp::collection_selection_mode::single;
    s.cv.set_cv_handler(s.cv_handler);
    s.cv_handler.map_items_source(s.cv);
    s.cv_handler.map_selected_index(s.cv);
    s.cv_handler.map_selection_mode(s.cv);
    s.cv_handler.map_layout(s.cv);
}

class layout_demo_app : public mpapp::application {
public:
    void on_launch() override {
        const std::vector<std::string> items{
            "Alpha", "Beta", "Gamma", "Delta",
            "Epsilon", "Zeta", "Eta", "Theta",
            "Iota", "Kappa", "Lambda", "Mu",
        };
        wire_section(vlist_, "vertical_list",   items, mpapp::collection_layout::vertical_list);
        wire_section(hlist_, "horizontal_list", items, mpapp::collection_layout::horizontal_list);
        wire_section(vgrid_, "vertical_grid",   items, mpapp::collection_layout::vertical_grid);
        wire_section(hgrid_, "horizontal_grid", items, mpapp::collection_layout::horizontal_grid);

        layout_.set_handler(layout_handler_);
        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing           = 6.0;
        layout_.padding           = mpapp::thickness{12.0};
        layout_.add(vlist_.header);  layout_.add(vlist_.cv);
        layout_.add(hlist_.header);  layout_.add(hlist_.cv);
        layout_.add(vgrid_.header);  layout_.add(vgrid_.cv);
        layout_.add(hgrid_.header);  layout_.add(hgrid_.cv);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0028 - CollectionView layout matrix";
        window_.width  = 640;
        window_.height = 880;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    cv_section vlist_{};
    cv_section hlist_{};
    cv_section vgrid_{};
    cv_section hgrid_{};

    mpapp::stack_layout             layout_{};
    mpapp::stack_layout_handler<lp> layout_handler_{};
    mpapp::window                   window_{};
    mpapp::window_handler<lp>       window_handler_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<layout_demo_app>(argc, argv);
}
