// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0020 — ListView demo (Linux/GTK4).
//
// Visible end-to-end exercise of ADR-0020's virtualized-item-host
// wrap-platform pattern on `list_view`:
//   * `items_source` — vector<string> rendered through GTK4 GtkListBox.
//   * `selected_index` — bidirectional binding with native selection.
//   * `item_tapped` — signal fired when a row is clicked; demo subscribes
//     and reflects the row index + label in a status label.

#include <string>
#include <vector>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/list_view.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/list_view_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

class listview_demo_app : public mpapp::application {
public:
    void on_launch() override {
        items_sets_ = {
            std::vector<std::string>{"Apple", "Banana", "Cherry", "Date",
                                     "Elderberry", "Fig"},
            std::vector<std::string>{"Mercury", "Venus", "Earth", "Mars",
                                     "Jupiter", "Saturn", "Uranus", "Neptune"},
            std::vector<std::string>{"Red", "Green", "Blue"},
        };

        lv_.set_lv_handler(lv_handler_);
        lv_handler_.map_items_source(lv_);
        lv_handler_.map_selected_index(lv_);

        // Initial items + initial selection.
        lv_.items_source = items_sets_[0];
        lv_.selected_index = 0;

        // Subscribe to item_tapped and selected_index.
        lv_.item_tapped.subscribe(tap_slot_, tap_cb_);
        lv_.selected_index.changed.subscribe(sel_slot_, sel_cb_);

        // Rotate button cycles items_source.
        rotate_btn_.text = "Rotate items_source";
        rotate_btn_.clicked.subscribe(rotate_slot_, rotate_cb_);

        status_label_.set_handler(status_label_handler_);
        status_label_handler_.map_text(status_label_);
        refresh_status();

        layout_.set_handler(layout_handler_);
        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing           = 8.0;
        layout_.padding           = mpapp::thickness{16.0};
        layout_.add(status_label_);
        layout_.add(rotate_btn_);
        layout_.add(lv_);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0020 - ListView Demo (GTK4)";
        window_.width  = 460;
        window_.height = 460;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    void refresh_status() {
        const int idx = lv_.selected_index.get();
        const auto& items = lv_.items_source.get();
        std::string sel_label = "(none)";
        if (idx >= 0 && idx < static_cast<int>(items.size())) {
            sel_label = items[static_cast<std::size_t>(idx)];
        }
        status_label_.text =
            "items_count: " + std::to_string(items.size()) +
            "   selected_index: " + std::to_string(idx) +
            "   selected: " + sel_label +
            "   taps: " + std::to_string(tap_count_) +
            "   last_tap_index: " + std::to_string(last_tap_index_);
    }

    void rotate() {
        rotation_index_ = (rotation_index_ + 1) % items_sets_.size();
        lv_.items_source = items_sets_[rotation_index_];
        lv_.selected_index = (items_sets_[rotation_index_].empty() ? -1 : 0);
    }

    struct rotate_cb_t {
        listview_demo_app* self;
        void operator()() const { self->rotate(); }
    };
    struct tap_cb_t {
        listview_demo_app* self;
        void operator()(int idx) const {
            ++self->tap_count_;
            self->last_tap_index_ = idx;
            self->refresh_status();
        }
    };
    struct sel_cb_t {
        listview_demo_app* self;
        void operator()(int /*idx*/) const { self->refresh_status(); }
    };

    std::vector<std::vector<std::string>> items_sets_{};
    std::size_t rotation_index_   = 0;
    int         tap_count_        = 0;
    int         last_tap_index_   = -1;

    mpapp::list_view                    lv_{};
    mpapp::list_view_handler<>        lv_handler_{};

    mpapp::button                       rotate_btn_{};

    mpapp::label                        status_label_{};
    mpapp::label_handler<>            status_label_handler_{};

    mpapp::stack_layout                 layout_{};
    mpapp::stack_layout_handler<>     layout_handler_{};
    mpapp::window                       window_{};
    mpapp::window_handler<>           window_handler_{};

    rotate_cb_t                         rotate_cb_{this};
    tap_cb_t                            tap_cb_{this};
    sel_cb_t                            sel_cb_{this};
    mpapp::signal_slot<>                rotate_slot_{};
    mpapp::signal_slot<int>             tap_slot_{};
    mpapp::signal_slot<const int&>      sel_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<listview_demo_app>(argc, argv);
}
