// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0019 — CollectionView item_template demo (Linux/GTK4).
//
// Visible end-to-end exercise of `collection_view::item_template` —
// the `std::function<unique_ptr<view>(int)>` factory that materializes
// one typed cell per `items_source` row. When set:
//   * Each `items_source.set(v)` triggers automatic re-materialization.
//   * The `materialized_changed` signal fires after each rebuild.
//   * The handler picks up `materialized_views()` via the typed-items
//     render path (`rebuild_typed`).
//
// UI:
//   * collection_view rendered through GTK4 GtkListBox.
//   * "Rotate items" button cycles items_source between three lists.
//   * Status label showing `materialized_count` + cumulative factory
//     invocation count + last invoked index.

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/collection_view.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/button_handler.hpp>
#include <mpapp/handlers/collection_view_handler.hpp>
#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

using lp = mpapp::platform::current;

// A label that owns its own handler so item_template's factory can
// emit fully renderable cells via a single `unique_ptr<view>`. The
// collection_view owns the unique_ptr; the unique_ptr owns the
// label + its handler together.
class row_label : public mpapp::label {
public:
    row_label() {
        set_handler(handler_);
        handler_.map_text(*this);
    }
private:
    mpapp::label_handler<lp> handler_{};
};

class item_template_demo_app : public mpapp::application {
public:
    void on_launch() override {
        // ---- collection_view + item_template ----------------------
        // Three rotating items_source variants. Each call to "Rotate"
        // bumps `rotation_index_` and re-applies the corresponding
        // items_source, which triggers re-materialization.
        items_sets_ = {
            std::vector<std::string>{"Apples", "Bananas", "Cherries", "Dates"},
            std::vector<std::string>{"Sapphire", "Topaz", "Garnet"},
            std::vector<std::string>{"Mercury", "Venus", "Earth",
                                     "Mars", "Jupiter"},
        };

        cv_.layout         = mpapp::collection_layout::vertical_list;
        cv_.selection_mode = mpapp::collection_selection_mode::single;

        // Factory: produces a renderable cell (row_label) per items_source
        // index. The label text is composed from items_source[i].
        // Captured state: `&factory_invocations_` and `&last_factory_index_`
        // for status reporting.
        cv_.item_template = [this](int i) -> std::unique_ptr<mpapp::view> {
            ++this->factory_invocations_;
            this->last_factory_index_ = i;
            auto cell = std::make_unique<row_label>();
            const auto& src = this->cv_.items_source.get();
            cell->text = "[" + std::to_string(i) + "] " +
                         (i < static_cast<int>(src.size()) ? src[i] : std::string{"???"});
            return cell;
        };

        // Wire the handler. map_typed_items also subscribes to
        // materialized_changed, so the handler re-renders whenever
        // the factory re-materializes.
        cv_.set_cv_handler(cv_handler_);
        cv_handler_.map_items_source(cv_);
        cv_handler_.map_typed_items(cv_);
        cv_handler_.map_selected_index(cv_);
        cv_handler_.map_selection_mode(cv_);
        cv_handler_.map_layout(cv_);

        // Status update: subscribe to materialized_changed so we can
        // recompute the count + cumulative factory-invocation number.
        cv_.materialized_changed.subscribe(materialized_slot_, materialized_cb_);

        // Initial items_source — triggers first materialization.
        cv_.items_source = items_sets_[0];

        // ---- "Rotate items" button -------------------------------
        rotate_btn_.set_handler(rotate_btn_handler_);
        rotate_btn_handler_.map_text(rotate_btn_);
        rotate_btn_handler_.map_clicked(rotate_btn_);
        rotate_btn_.text = "Rotate items_source";
        rotate_btn_.clicked.subscribe(rotate_slot_, rotate_cb_);

        // ---- Status label ----------------------------------------
        status_label_.set_handler(status_label_handler_);
        status_label_handler_.map_text(status_label_);
        refresh_status();

        // ---- Layout ----------------------------------------------
        layout_.set_handler(layout_handler_);
        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing           = 8.0;
        layout_.padding           = mpapp::thickness{16.0};
        layout_.add(status_label_);
        layout_.add(rotate_btn_);
        layout_.add(cv_);
        layout_handler_.bind(layout_);

        // ---- Window ----------------------------------------------
        window_.title  = "MPAPP T-0019 - CollectionView item_template Demo (GTK4)";
        window_.width  = 480;
        window_.height = 440;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    void refresh_status() {
        status_label_.text =
            "materialized_count: " + std::to_string(cv_.materialized_count()) +
            "   factory_invocations: " + std::to_string(factory_invocations_) +
            "   last_index: " + std::to_string(last_factory_index_);
    }

    void rotate() {
        rotation_index_ = (rotation_index_ + 1) % items_sets_.size();
        cv_.items_source = items_sets_[rotation_index_];
    }

    struct rotate_cb_t {
        item_template_demo_app* self;
        void operator()() const { self->rotate(); }
    };
    struct materialized_cb_t {
        item_template_demo_app* self;
        void operator()() const { self->refresh_status(); }
    };

    // ---- Members ----------------------------------------------------
    std::vector<std::vector<std::string>> items_sets_{};
    std::size_t rotation_index_       = 0;
    int         last_factory_index_   = -1;
    int         factory_invocations_  = 0;

    mpapp::collection_view              cv_{};
    mpapp::collection_view_handler<lp>  cv_handler_{};

    mpapp::button                       rotate_btn_{};
    mpapp::button_handler<lp>           rotate_btn_handler_{};

    mpapp::label                        status_label_{};
    mpapp::label_handler<lp>            status_label_handler_{};

    mpapp::stack_layout                 layout_{};
    mpapp::stack_layout_handler<lp>     layout_handler_{};
    mpapp::window                       window_{};
    mpapp::window_handler<lp>           window_handler_{};

    rotate_cb_t                         rotate_cb_{this};
    materialized_cb_t                   materialized_cb_{this};
    mpapp::signal_slot<>                rotate_slot_{};
    mpapp::signal_slot<>                materialized_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<item_template_demo_app>(argc, argv);
}
