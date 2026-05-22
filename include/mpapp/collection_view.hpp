// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Modern MAUI virtualized item host (supersedes
// [[ListView]]).
//
// Three parallel item surfaces:
//
//   1. `items_source`  — title + vector<string> rows. Flat-string
//                        rendering through the platform recycler
//                        (ListView/GtkListBox/GtkFlowBox/GridView).
//                        Full virtualization. Good for thousands of
//                        homogeneous rows.
//
//   2. `typed_items`   — vector<view*>. Each entry is a fully-typed
//                        widget (cell tree, label, image, custom
//                        view). The handler renders each via its
//                        native widget through ADR-0013 dispatch.
//                        Non-virtualizing in v1 (one native widget
//                        per item) — good for small lists with rich
//                        item content. Cells / views are non-owning;
//                        the app owns each.
//
//   3. `item_template` — a factory `std::function<unique_ptr<view>(int)>`.
//                        When set, the collection_view materializes
//                        one cell per index in items_source by calling
//                        the factory; the materialized cells are owned
//                        by the collection_view itself and surface
//                        through the same typed render path as
//                        `typed_items`. Non-virtualizing — the entire
//                        items_source materializes at once; suitable
//                        for small-to-medium lists where you want
//                        per-row typed cells without managing cell
//                        ownership manually.
//
// Precedence on the render side:
//   * typed_items non-empty               → render typed_items
//   * item_template set + items_source    → render materialized_
//   * items_source non-empty              → flat-string render
// `typed_items` and `item_template` are mutually exclusive in their
// effect; both surfaces hold values independently and the surface
// doesn't enforce exclusivity, but the handler picks one.

#ifndef MPAPP_COLLECTION_VIEW_HPP
#define MPAPP_COLLECTION_VIEW_HPP

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp {

enum class collection_selection_mode : std::uint8_t {
    none     = 0,
    single   = 1,
    multiple = 2,
};

enum class collection_layout : std::uint8_t {
    vertical_list = 0,
    horizontal_list = 1,
    vertical_grid = 2,
    horizontal_grid = 3,
};

template <class Platform>
class collection_view_handler;

class collection_view : public view {
public:
    using item_factory_t = std::function<std::unique_ptr<view>(int /*index*/)>;

    collection_view() {
        // When items_source changes and item_template is set,
        // re-materialize the cells. Same for item_template changing
        // while items_source is non-empty. Slots and callbacks are
        // members so they outlive the connection.
        items_source.changed.subscribe(items_materialize_slot_, items_materialize_cb_);
        item_template.changed.subscribe(template_materialize_slot_, template_materialize_cb_);
    }
    ~collection_view() override = default;

    collection_view(const collection_view&)            = delete;
    collection_view& operator=(const collection_view&) = delete;
    collection_view(collection_view&&)                 = delete;
    collection_view& operator=(collection_view&&)      = delete;

    // ----- Surface ------------------------------------------------------

    Observable<std::vector<std::string>>   items_source{};
    // Parallel typed surface — see header comment for the trade-off.
    Observable<std::vector<view*>>         typed_items{};
    // Factory that materializes one cell per items_source index.
    // When set, the collection_view auto-materializes on each change
    // and stores the cells in `materialized_` (owned). Read the
    // materialized cells via `materialized_views()` if a handler
    // needs to walk them; otherwise the standard typed-items render
    // path picks them up.
    Observable<item_factory_t>             item_template{};
    Observable<collection_selection_mode>  selection_mode{collection_selection_mode::single};
    Observable<int>                        selected_index{-1};                  // for single-select
    Observable<std::vector<int>>           selected_indices{};                  // for multi-select
    Observable<collection_layout>          layout{collection_layout::vertical_list};
    Observable<int>                        span{1};                              // grid span

    // Non-owning view of the materialized cells produced by
    // item_template, in items_source order. Empty when no template
    // is set or items_source is empty.
    [[nodiscard]] std::vector<view*> materialized_views() const {
        std::vector<view*> out;
        out.reserve(materialized_.size());
        for (const auto& p : materialized_) out.push_back(p.get());
        return out;
    }

    [[nodiscard]] std::size_t materialized_count() const noexcept {
        return materialized_.size();
    }

    // ----- Events -------------------------------------------------------

    signal<int>                            item_tapped{};
    // Fired after item_template re-materializes (whether items_source
    // changed, item_template changed, or both). Handlers subscribe to
    // this to know when to consume `materialized_views()`. Distinct
    // from items_source.changed because materialize is decoupled from
    // signal-fire ordering — by the time materialized_changed emits,
    // materialized_ is fully up to date.
    signal<>                               materialized_changed{};

    // ----- Helpers -------------------------------------------------------

    // Select an item. Honors selection_mode: in `none` mode this is a
    // no-op; in `single` mode it sets selected_index and resets
    // selected_indices to a single-element vector; in `multiple` mode it
    // appends idx to selected_indices if not already present.
    void select(int idx) {
        if (selection_mode.get() == collection_selection_mode::none) return;
        if (idx < 0 || idx >= static_cast<int>(items_source.get().size())) return;
        if (selection_mode.get() == collection_selection_mode::single) {
            selected_index.set(idx);
            selected_indices.set(std::vector<int>{idx});
        } else {
            auto v = selected_indices.get();
            if (std::find(v.begin(), v.end(), idx) == v.end()) {
                v.push_back(idx);
                selected_indices.set(std::move(v));
            }
            selected_index.set(idx);
        }
    }

    void deselect(int idx) {
        if (selection_mode.get() != collection_selection_mode::multiple) return;
        auto v = selected_indices.get();
        auto it = std::find(v.begin(), v.end(), idx);
        if (it == v.end()) return;
        v.erase(it);
        selected_indices.set(std::move(v));
        if (selected_index.get() == idx) {
            selected_index.set(v.empty() ? -1 : v.back());
        }
    }

    void clear_selection() {
        selected_index.set(-1);
        selected_indices.set(std::vector<int>{});
    }

    // ----- Handler ------------------------------------------------------

    collection_view_handler<platform::current>&       cv_handler() noexcept       { return *cv_handler_; }
    const collection_view_handler<platform::current>& cv_handler() const noexcept { return *cv_handler_; }
    bool                                              has_cv_handler() const noexcept { return cv_handler_ != nullptr; }
    void                                              set_cv_handler(collection_view_handler<platform::current>& h) noexcept { cv_handler_ = &h; }

private:
    void rematerialize_if_template() {
        const auto& factory = item_template.get();
        if (!factory) {
            materialized_.clear();
            materialized_changed.emit();
            return;
        }
        const auto& src = items_source.get();
        std::vector<std::unique_ptr<view>> next;
        next.reserve(src.size());
        for (std::size_t i = 0; i < src.size(); ++i) {
            next.push_back(factory(static_cast<int>(i)));
        }
        materialized_ = std::move(next);
        materialized_changed.emit();
    }

    // Functor structs (rather than direct lambdas) so the callable's
    // address is stable and the signal_slot holds a valid reference
    // for the lifetime of the collection_view.
    struct items_materialize_cb {
        collection_view* self = nullptr;
        void operator()(const std::vector<std::string>&) const {
            self->rematerialize_if_template();
        }
    };
    struct template_materialize_cb {
        collection_view* self = nullptr;
        void operator()(const item_factory_t&) const {
            self->rematerialize_if_template();
        }
    };

    items_materialize_cb                          items_materialize_cb_{this};
    template_materialize_cb                       template_materialize_cb_{this};
    signal_slot<const std::vector<std::string>&>  items_materialize_slot_{};
    signal_slot<const item_factory_t&>            template_materialize_slot_{};

    std::vector<std::unique_ptr<view>>            materialized_{};
    collection_view_handler<platform::current>*   cv_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_COLLECTION_VIEW_HPP
