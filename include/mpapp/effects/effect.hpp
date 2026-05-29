// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0009-behaviors-and-effects.md
//
// `mpapp::effect` — minimal attach-point for platform visual effects.
// Counterpart to MAUI's `Effect` / `RoutedEffect`. Each effect carries a
// resolution `id` (MAUI's "MyCompany.MyEffect"); a platform's effect
// registry maps that id to native rendering. The framework calls
// `on_attached`/`on_detached` as the effect enters/leaves a view's
// `effects` collection.
//
// NOTE: MAUI has effectively superseded Effects with Handlers, and
// MPAPP's handler architecture (ADR-0024) already covers the same
// ground. This attach-point exists for surface parity + to give legacy
// MAUI XAML (`<Effect>`) a lowering target; new code should prefer a
// handler. Self-contained (forward-declares view); no macros.

#ifndef MPAPP_EFFECTS_EFFECT_HPP
#define MPAPP_EFFECTS_EFFECT_HPP

#include <string>
#include <utility>

namespace mpapp {

class view;

class effect {
public:
    explicit effect(std::string resolution_id) : id_{ std::move(resolution_id) } {}
    virtual ~effect() = default;

    effect(const effect&)            = delete;
    effect& operator=(const effect&) = delete;
    effect(effect&&)                 = delete;
    effect& operator=(effect&&)      = delete;

    // The platform effect-registry key (MAUI's Effect.ResolveId).
    [[nodiscard]] const std::string& resolution_id() const noexcept { return id_; }

    virtual void on_attached(view& /*host*/) {}
    virtual void on_detached(view& /*host*/) {}

private:
    std::string id_;
};

} // namespace mpapp

#endif // MPAPP_EFFECTS_EFFECT_HPP
