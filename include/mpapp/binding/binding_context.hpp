// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// `mpapp::binding_context` — type-erased data context, MAUI's
// `BindingContext` (which is typed `object`). Holds a
// `shared_ptr<void>` plus the stored type so a typed `get<C>()` can
// recover the original pointer safely. Stored by value on every
// `view`; the inheritance walk (a child uses its own context if set,
// else the nearest ancestor's) lives in `relative_source.hpp`, which
// has the full `view` definition.
//
// Self-contained: NO dependency on view.hpp, so view.hpp can embed a
// `binding_context` member without a circular include.

#ifndef MPAPP_BINDING_BINDING_CONTEXT_HPP
#define MPAPP_BINDING_BINDING_CONTEXT_HPP

#include <memory>
#include <typeinfo>
#include <utility>

namespace mpapp {

class binding_context {
public:
    binding_context() = default;

    template <class C>
    explicit binding_context(std::shared_ptr<C> ctx)
        : ptr_{ std::move(ctx) }, type_{ &typeid(C) } {}

    template <class C>
    void set(std::shared_ptr<C> ctx) {
        ptr_  = std::move(ctx);
        type_ = &typeid(C);
    }

    void clear() noexcept {
        ptr_.reset();
        type_ = nullptr;
    }

    [[nodiscard]] bool has_value() const noexcept { return static_cast<bool>(ptr_); }

    // Typed retrieval — nullptr if empty OR the stored type is not
    // exactly `C` (no implicit base/derived conversion, matching the
    // exact-type semantics the XAML compiler will lower against).
    template <class C>
    [[nodiscard]] std::shared_ptr<C> get() const {
        if (!ptr_ || type_ == nullptr || *type_ != typeid(C)) {
            return nullptr;
        }
        return std::static_pointer_cast<C>(ptr_);
    }

    // The stored context's type, or nullptr when empty. Lets callers
    // introspect without naming the type.
    [[nodiscard]] const std::type_info* stored_type() const noexcept { return type_; }

private:
    std::shared_ptr<void> ptr_{};
    const std::type_info*  type_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_BINDING_BINDING_CONTEXT_HPP
