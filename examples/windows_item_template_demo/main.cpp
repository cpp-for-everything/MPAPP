// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0019 — CollectionView item_template demo (Windows/WinUI 3).
//
// Sibling of gtk4_item_template_demo. Same collection_view +
// item_template factory + rotate button + status label, different
// per-platform handler set.

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

#include <mpapp/handlers/collection_view_handler.hpp>
#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

using wp = mpapp::platform::current;

class row_label : public mpapp::label {
public:
    row_label() {
        set_handler(handler_);
        handler_.map_text(*this);
    }
private:
    mpapp::label_handler<wp> handler_{};
};

class item_template_demo_app : public mpapp::application {
public:
    void on_launch() override {
        items_sets_ = {
            std::vector<std::string>{"Apples", "Bananas", "Cherries", "Dates"},
            std::vector<std::string>{"Sapphire", "Topaz", "Garnet"},
            std::vector<std::string>{"Mercury", "Venus", "Earth",
                                     "Mars", "Jupiter"},
        };

        cv_.layout         = mpapp::collection_layout::vertical_list;
        cv_.selection_mode = mpapp::collection_selection_mode::single;

        cv_.item_template = [this](int i) -> std::unique_ptr<mpapp::view> {
            ++this->factory_invocations_;
            this->last_factory_index_ = i;
            auto cell = std::make_unique<row_label>();
            const auto& src = this->cv_.items_source.get();
            cell->text = "[" + std::to_string(i) + "] " +
                         (i < static_cast<int>(src.size()) ? src[i] : std::string{"???"});
            return cell;
        };

        cv_.set_cv_handler(cv_handler_);
        cv_handler_.map_items_source(cv_);
        cv_handler_.map_typed_items(cv_);
        cv_handler_.map_selected_index(cv_);
        cv_handler_.map_selection_mode(cv_);
        cv_handler_.map_layout(cv_);

        cv_.materialized_changed.subscribe(materialized_slot_, materialized_cb_);

        cv_.items_source = items_sets_[0];

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
        layout_.add(cv_);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0019 - CollectionView item_template Demo (WinUI 3)";
        window_.width  = 540;
        window_.height = 460;
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

    std::vector<std::vector<std::string>> items_sets_{};
    std::size_t rotation_index_       = 0;
    int         last_factory_index_   = -1;
    int         factory_invocations_  = 0;

    mpapp::collection_view              cv_{};
    mpapp::collection_view_handler<wp>  cv_handler_{};

    mpapp::button                       rotate_btn_{};

    mpapp::label                        status_label_{};
    mpapp::label_handler<wp>            status_label_handler_{};

    mpapp::stack_layout                 layout_{};
    mpapp::stack_layout_handler<wp>     layout_handler_{};
    mpapp::window                       window_{};
    mpapp::window_handler<wp>           window_handler_{};

    rotate_cb_t                         rotate_cb_{this};
    materialized_cb_t                   materialized_cb_{this};
    mpapp::signal_slot<>                rotate_slot_{};
    mpapp::signal_slot<>                materialized_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<item_template_demo_app>(argc, argv);
}
