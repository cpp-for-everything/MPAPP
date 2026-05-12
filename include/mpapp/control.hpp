// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Handlers.md
//
// `control<Derived>` is the CRTP base used by every cross-platform widget
// (button, label, …). For the T-0003 spike this is intentionally a thin
// placeholder: it just provides static-dispatch access to the derived
// type. Once more controls land this will grow shared lifecycle and
// property-mapper machinery — see the milestone M-03 roadmap.

#ifndef MPAPP_CONTROL_HPP
#define MPAPP_CONTROL_HPP

namespace mpapp {

template <class Derived>
class control {
public:
    Derived&       derived() noexcept       { return static_cast<Derived&>(*this); }
    const Derived& derived() const noexcept { return static_cast<const Derived&>(*this); }

protected:
    control()  = default;
    ~control() = default;

    control(const control&)            = delete;
    control& operator=(const control&) = delete;
    control(control&&)                 = delete;
    control& operator=(control&&)      = delete;
};

} // namespace mpapp

#endif // MPAPP_CONTROL_HPP
