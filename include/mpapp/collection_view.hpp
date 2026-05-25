// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/CollectionView.md
//
// `mpapp::collection_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_collection_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::collection_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_collection_view x;
//     mpapp::collection_view_handler<mpapp::platform::mock> h;
//     h.map_items_source(x);

#ifndef MPAPP_COLLECTION_VIEW_HPP
#define MPAPP_COLLECTION_VIEW_HPP

#include "internal/basic_collection_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_collection_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/collection_view_handler.hpp"

namespace mpapp {

class collection_view : public internal::basic_collection_view {
public:
    collection_view() {
        set_cv_handler(embedded_handler_);
        embedded_handler_.map_items_source(*this);
        embedded_handler_.map_typed_items(*this);
        embedded_handler_.map_selected_index(*this);
        embedded_handler_.map_selection_mode(*this);
        embedded_handler_.map_layout(*this);
        embedded_handler_.map_gestures(*this);
    }

    collection_view(const collection_view&)            = delete;
    collection_view& operator=(const collection_view&) = delete;
    collection_view(collection_view&&)                 = delete;
    collection_view& operator=(collection_view&&)      = delete;

private:
    internal::collection_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::collection_view_handler<>` (host-current) and
// `mpapp::collection_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using collection_view_handler = internal::collection_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_COLLECTION_VIEW_HPP
