// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Layout.md
//
// `mpapp::layout` — abstract multi-child container. Concrete subclasses
// (vertical_stack_layout, grid, flex_layout, …) plug in a layout-manager
// strategy via `create_layout_manager()`.
//
// Mock surface (P2). The children collection is a flat std::vector of
// non-owning pointers; the real `ObservableList<shared_ptr<view>>` lands
// with the M-03 collection-binding plumbing (it needs the items-source
// machinery from BindableLayout).

#ifndef MPAPP_LAYOUT_HPP
#define MPAPP_LAYOUT_HPP

#include <cstddef>
#include <vector>
#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_LAYOUT_HAS_STD_FORMAT 1
#endif

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

// Four-sided inset. Same shape as MAUI's `Thickness`.
struct thickness {
    double left   = 0.0;
    double top    = 0.0;
    double right  = 0.0;
    double bottom = 0.0;

    constexpr thickness() = default;
    explicit constexpr thickness(double uniform) noexcept
        : left(uniform), top(uniform), right(uniform), bottom(uniform) {}
    constexpr thickness(double l, double t, double r, double b) noexcept
        : left(l), top(t), right(r), bottom(b) {}

    bool operator==(const thickness&) const = default;
};

template <class Platform = platform::current>
class layout_handler;

class layout : public view {
public:
    layout() = default;

    Observable<thickness>     padding{};
    Observable<bool>          is_clipped_to_bounds{false};
    Observable<bool>          cascade_input_transparent{false};

    // Children — observed via direct `add`/`remove`/`clear` calls. Each
    // mutation drives the corresponding handler command mapper. A real
    // `ObservableList<T>` with INotifyCollectionChanged-equivalent
    // signals replaces this in M-03; the mock surface keeps the API
    // explicit so tests can assert the command sequence.
    void add(view& child);
    void insert(std::size_t index, view& child);
    void remove(view& child);
    void clear();
    void update_z_index(view& child, int new_z);

    std::size_t child_count() const noexcept { return children_.size(); }
    view*       child_at(std::size_t i) const noexcept {
        return i < children_.size() ? children_[i] : nullptr;
    }

    layout_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const layout_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                     has_handler() const noexcept { return handler_ != nullptr; }
    void                                     set_handler(layout_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    std::vector<view*>                    children_;
    layout_handler<platform::current>*    handler_ = nullptr;
};

// Inline child-list mutators. Kept in-header for the mock surface so the
// 7-component group has no .cpp files of its own — each `*_handler.cpp`
// implementation lands with its real platform handler.
//
// Each mutator also maintains the child's `parent_` pointer so the
// `find_in<T>` walker from RFC-0005 (resource lookup) — and any future
// visual-tree consumer (focus traversal, accessibility, …) — sees a
// consistent up-link. Clearing or removing a child detaches the link
// rather than leaving a dangling pointer to a destroyed layout.
inline void layout::add(view& child) {
    children_.push_back(&child);
    child.set_parent(this);
}

inline void layout::insert(std::size_t index, view& child) {
    if (index >= children_.size()) {
        children_.push_back(&child);
    } else {
        children_.insert(children_.begin() + static_cast<std::ptrdiff_t>(index), &child);
    }
    child.set_parent(this);
}

inline void layout::remove(view& child) {
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if (*it == &child) {
            (*it)->set_parent(nullptr);
            children_.erase(it);
            return;
        }
    }
}

inline void layout::clear() {
    for (view* c : children_) {
        if (c) {
            c->set_parent(nullptr);
        }
    }
    children_.clear();
}

inline void layout::update_z_index(view& child, int new_z) {
    child.z_index = new_z;
}

} // namespace mpapp

#ifdef MPAPP_LAYOUT_HAS_STD_FORMAT
template <>
struct std::formatter<mpapp::thickness> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::thickness& t, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "thickness({},{},{},{})",
                              t.left, t.top, t.right, t.bottom);
    }
};
#endif

#endif // MPAPP_LAYOUT_HPP
