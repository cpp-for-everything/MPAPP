// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Templates.md
//
// `mpapp::control_template` — a lightweight, header-only value type that
// carries a factory callable for a view subtree.  Mirrors MAUI's
// `ControlTemplate`: calling `instantiate()` creates a fresh root view
// from the stored factory, returning nullptr when no factory is set.
//
// Usage:
//   mpapp::control_template ct;
//   ct.factory = []() { return std::make_unique<mpapp::some_view>(); };
//   auto root = ct.instantiate();   // unique_ptr<view> or nullptr

#ifndef MPAPP_TEMPLATES_CONTROL_TEMPLATE_HPP
#define MPAPP_TEMPLATES_CONTROL_TEMPLATE_HPP

#include <functional>
#include <memory>

#include "../view.hpp"  // mpapp::view

namespace mpapp {

struct control_template {
    // Callable that produces a freshly-constructed view subtree.
    // May be empty (null) — `instantiate()` returns nullptr in that case.
    std::function<std::unique_ptr<view>()> factory;

    // Invoke the factory and return the new root view, or nullptr when no
    // factory has been set.
    [[nodiscard]] std::unique_ptr<view> instantiate() const {
        return factory ? factory() : nullptr;
    }

    // True when a factory callable is stored.
    [[nodiscard]] bool has_factory() const noexcept {
        return static_cast<bool>(factory);
    }
};

} // namespace mpapp

#endif // MPAPP_TEMPLATES_CONTROL_TEMPLATE_HPP
