// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// Visual-tree resolution helpers for data binding:
//
//   * `effective_binding_context(view)` — the inherited BindingContext:
//     the view's own if set, else the nearest ancestor's (walks
//     `view::parent()`). Mirrors MAUI's BindingContext inheritance.
//
//   * `find_ancestor<C>(view)` — the nearest ancestor (or self) that is
//     dynamic-castable to `C`. Backs `{RelativeSource AncestorType=…}`.
//
//   * `resolve_relative_source(view, mode)` — MAUI's RelativeSource
//     (Self / FindAncestor). TemplatedParent is deferred (needs the
//     control-template machinery).
//
// Has the full `view` definition (unlike binding_context.hpp), so it
// can walk the tree.

#ifndef MPAPP_BINDING_RELATIVE_SOURCE_HPP
#define MPAPP_BINDING_RELATIVE_SOURCE_HPP

#include <cstdint>

#include "../view.hpp"
#include "binding_context.hpp"

namespace mpapp {

// Walk view -> parent -> ... and return the first LOCAL binding context
// that has a value. Returns a const-ref to an empty context (a static
// sentinel) when no ancestor carries one. The returned reference stays
// valid as long as the owning view does.
[[nodiscard]] inline const binding_context&
effective_binding_context(const view& v) {
    for (const view* cur = &v; cur != nullptr; cur = cur->parent()) {
        const binding_context& ctx = cur->local_binding_context();
        if (ctx.has_value()) {
            return ctx;
        }
    }
    static const binding_context empty{};
    return empty;
}

// Nearest ancestor-or-self that is `C`. `include_self = false` skips the
// starting view and begins at its parent (MAUI's default AncestorLevel
// semantics start above self for FindAncestor, but Self mode wants
// self-inclusion — hence the flag).
template <class C>
[[nodiscard]] C* find_ancestor(view& start, bool include_self = false) {
    for (view* cur = include_self ? &start : start.parent();
         cur != nullptr; cur = cur->parent()) {
        if (auto* hit = dynamic_cast<C*>(cur)) {
            return hit;
        }
    }
    return nullptr;
}

// MAUI's RelativeSourceMode (the implementable subset).
enum class relative_source_mode : std::uint8_t {
    self           = 0,  // the bound element itself
    find_ancestor  = 1,  // nearest ancestor of a requested type
};

// Resolve Self -> the view itself; FindAncestor -> use find_ancestor<C>
// instead (this overload covers Self; FindAncestor needs the type).
[[nodiscard]] inline view*
resolve_relative_source(view& start, relative_source_mode mode) {
    switch (mode) {
        case relative_source_mode::self:
            return &start;
        case relative_source_mode::find_ancestor:
            return start.parent();  // untyped: nearest ancestor; use
                                    // find_ancestor<C> for a typed match
    }
    return nullptr;
}

} // namespace mpapp

#endif // MPAPP_BINDING_RELATIVE_SOURCE_HPP
