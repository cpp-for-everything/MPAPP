// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0009-behaviors-and-effects.md
//
// `mpapp::behavior` — abstract base for attached behaviors. Counterpart
// to MAUI's `Behavior` / `Behavior<T>`: a reusable object that hooks a
// view's lifecycle + signals without subclassing the view. The
// framework calls `on_attached(view&)` when the behavior is added to a
// view's `behaviors` collection and `on_detached(view&)` when removed.
//
// Self-contained: only forward-declares `view` (its methods take a
// `view&`), so view.hpp can include this + store a behavior collection
// without a circular include. No macros (ADR-0009); not a
// wrapper-component (ADR-0024).

#ifndef MPAPP_BEHAVIORS_BEHAVIOR_HPP
#define MPAPP_BEHAVIORS_BEHAVIOR_HPP

namespace mpapp {

class view;

class behavior {
public:
    virtual ~behavior() = default;

    behavior(const behavior&)            = delete;
    behavior& operator=(const behavior&) = delete;
    behavior(behavior&&)                 = delete;
    behavior& operator=(behavior&&)      = delete;

    // Called by `view::add_behavior` once the behavior is attached, and
    // by `view::remove_behavior` (or the view's destruction) on detach.
    // Default no-ops so a concrete behavior overrides only what it needs.
    virtual void on_attached(view& /*host*/) {}
    virtual void on_detached(view& /*host*/) {}

protected:
    behavior() = default;
};

} // namespace mpapp

#endif // MPAPP_BEHAVIORS_BEHAVIOR_HPP
