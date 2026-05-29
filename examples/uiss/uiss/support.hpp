// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. УИСС demo — composition helpers.
//
// Thin wrappers that cut the per-widget boilerplate while staying 100%
// on the public MPAPP surface (no ifdefs, no platform code). The whole
// app composes from these on every target.
//
//   * non_owning  — a shared_ptr<view> that points at a member view
//                   without owning it (for ScrollView.content).
//   * box         — a stack_layout + its handler; add() children, then
//                   done() to bind the native container once.
//   * label_list  — owns a growable set of heap labels (leaf wrappers
//                   auto-bind their own handler) for data-driven rows.
//   * click_button — a button + slot + std::function action.

#ifndef UISS_SUPPORT_HPP
#define UISS_SUPPORT_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <mpapp/button.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/view.hpp>

namespace uiss {

// ---- Spacing metrics (px) ----------------------------------------------
inline constexpr double pad_lg = 20.0;
inline constexpr double pad_md = 14.0;
inline constexpr double gap_md = 10.0;
inline constexpr double gap_sm = 6.0;

// ---- Palette -----------------------------------------------------------
// The portal's deep-navy chrome (#1D3557) for headings + titles.
inline const mpapp::color tu_blue = mpapp::color::from_rgb8(29, 53, 87);

// A non-owning shared_ptr aliasing a long-lived member `view`. Used where
// the surface wants `Observable<shared_ptr<view>>` (e.g. ScrollView) but
// the child is owned elsewhere (a struct member).
inline std::shared_ptr<mpapp::view> non_owning(mpapp::view& v) {
    return std::shared_ptr<mpapp::view>(std::shared_ptr<void>{}, &v);
}

// Single-axis container that owns its native handler. Children are
// recorded with add(); done() binds the GtkBox / StackPanel / LinearLayout
// and appends every child exactly once.
struct box {
    mpapp::internal::basic_stack_layout layout{};
    mpapp::stack_layout_handler<>       handler{};

    box& vertical(double spacing = gap_md, mpapp::thickness pad = {}) {
        layout.stack_orientation = mpapp::orientation::vertical;
        layout.spacing           = spacing;
        layout.padding           = pad;
        return *this;
    }
    box& horizontal(double spacing = gap_md, mpapp::thickness pad = {}) {
        layout.stack_orientation = mpapp::orientation::horizontal;
        layout.spacing           = spacing;
        layout.padding           = pad;
        return *this;
    }
    box& add(mpapp::view& child) { layout.add(child); return *this; }
    void done() { layout.set_handler(handler); handler.bind(layout); }

    mpapp::view& as_view() noexcept { return layout; }
};

// Owns a growable list of heap-allocated labels. Each `mpapp::label` is a
// leaf wrapper that auto-binds its own handler in its constructor, so a
// data-driven loop can mint as many rows as the record holds.
struct label_list {
    std::vector<std::unique_ptr<mpapp::label>> items;

    mpapp::label& add(box& parent, const std::string& text) {
        auto l  = std::make_unique<mpapp::label>();
        l->text = text;
        parent.add(*l);
        items.push_back(std::move(l));
        return *items.back();
    }
};

// A button bound to a std::function. Non-movable (owns a button + slot +
// a self-referential callback) — hold by value as a member or via
// unique_ptr in a vector so the address stays stable.
struct click_button {
    mpapp::button         btn{};
    mpapp::signal_slot<>  slot{};
    std::function<void()> action{};

    struct cb_t {
        click_button* self;
        void operator()() const { if (self->action) self->action(); }
    };
    cb_t cb{this};

    void build(const std::string& text, std::function<void()> a) {
        btn.text = text;
        action   = std::move(a);
        btn.clicked.subscribe(slot, cb);
    }
};

} // namespace uiss

#endif // UISS_SUPPORT_HPP
