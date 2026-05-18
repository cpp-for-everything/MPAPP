// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Handlers.md
//
// `control<Derived>` is the CRTP base used by every cross-platform widget
// (button, label, …). It inherits the cross-cutting `view` property
// surface (identity, layout, visibility, transforms) and adds CRTP
// `derived()` access for shared lifecycle / property-mapper machinery.
//
// Inheriting from `view` is what makes every widget a layoutable child
// (`stack_layout::add(button)`) and a valid `window.content` /
// `page.content` target — there is one widget hierarchy with `view` at
// the root.

#ifndef MPAPP_CONTROL_HPP
#define MPAPP_CONTROL_HPP

#include "view.hpp"

namespace mpapp {

template <class Derived>
class control : public view {
public:
    Derived&       derived() noexcept       { return static_cast<Derived&>(*this); }
    const Derived& derived() const noexcept { return static_cast<const Derived&>(*this); }

protected:
    control()           = default;
    ~control() override = default;

    control(const control&)            = delete;
    control& operator=(const control&) = delete;
    control(control&&)                 = delete;
    control& operator=(control&&)      = delete;
};

} // namespace mpapp

#endif // MPAPP_CONTROL_HPP
