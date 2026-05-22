// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Modern MAUI virtualized item host (supersedes
// [[ListView]]).
//
// Two parallel item surfaces:
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
// When typed_items is non-empty, the typed path wins. The handler
// falls back to items_source rendering otherwise. Both surfaces
// hold values independently and can coexist.

#ifndef MPAPP_COLLECTION_VIEW_HPP
#define MPAPP_COLLECTION_VIEW_HPP

#include <algorithm>
#include <cstdint>
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
    collection_view() = default;
    ~collection_view() override = default;

    collection_view(const collection_view&)            = delete;
    collection_view& operator=(const collection_view&) = delete;
    collection_view(collection_view&&)                 = delete;
    collection_view& operator=(collection_view&&)      = delete;

    // ----- Surface ------------------------------------------------------

    Observable<std::vector<std::string>>   items_source{};
    // Parallel typed surface — see header comment for the trade-off.
    Observable<std::vector<view*>>         typed_items{};
    Observable<collection_selection_mode>  selection_mode{collection_selection_mode::single};
    Observable<int>                        selected_index{-1};                  // for single-select
    Observable<std::vector<int>>           selected_indices{};                  // for multi-select
    Observable<collection_layout>          layout{collection_layout::vertical_list};
    Observable<int>                        span{1};                              // grid span

    // ----- Events -------------------------------------------------------

    signal<int>                            item_tapped{};

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
    collection_view_handler<platform::current>* cv_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_COLLECTION_VIEW_HPP
