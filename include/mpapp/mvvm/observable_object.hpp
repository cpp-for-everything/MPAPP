// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0009-public-api-template-wrappers-only.md
//
// observable_object — the CommunityToolkit.Mvvm `ObservableObject` equivalent.
//
// A base class for view-models that exposes `property_changing` and
// `property_changed` signals (PropertyChanging / PropertyChanged in MAUI
// semantics). Derived types mutate backing fields through `set_property`,
// which compares the incoming value with operator==, emits the changing
// signal, assigns, then emits the changed signal — returning whether the
// value actually changed. `raise_property_changed` lets a view-model fire the
// changed notification manually (e.g. for computed properties), and the
// `on_property_changed` virtual hook lets a derived type react centrally.
//
// Header-only, no macros in the public API (ADR-0002), mpapp::signal for
// events.

#ifndef MPAPP_MVVM_OBSERVABLE_OBJECT_HPP
#define MPAPP_MVVM_OBSERVABLE_OBJECT_HPP

#include <string_view>
#include <utility>

#include "../signal.hpp"

namespace mpapp {

class observable_object {
public:
    observable_object() = default;

    observable_object(const observable_object&)            = delete;
    observable_object& operator=(const observable_object&) = delete;
    observable_object(observable_object&&)                 = delete;
    observable_object& operator=(observable_object&&)      = delete;

    virtual ~observable_object() = default;

    // Fires before the backing field is mutated.
    mpapp::signal<std::string_view> property_changing;
    // Fires after the backing field is mutated.
    mpapp::signal<std::string_view> property_changed;

protected:
    // Assigns `value` to `field` only when it differs (compared via
    // operator==). On a real change it emits `property_changing`, performs the
    // assignment, invokes the `on_property_changed` hook, then emits
    // `property_changed`. Returns true iff the field changed.
    template <class T>
    bool set_property(T& field, T value, std::string_view name) {
        if (field == value) {
            return false;
        }
        property_changing.emit(name);
        field = std::move(value);
        on_property_changed(name);
        property_changed.emit(name);
        return true;
    }

    // Manually fire the changed notification for `name` (e.g. a computed
    // property whose dependencies just changed). Also runs the hook.
    void raise_property_changed(std::string_view name) {
        on_property_changed(name);
        property_changed.emit(name);
    }

    // Hook invoked whenever a property changes (via set_property or
    // raise_property_changed). Default implementation does nothing.
    virtual void on_property_changed(std::string_view /*name*/) {}
};

} // namespace mpapp

#endif // MPAPP_MVVM_OBSERVABLE_OBJECT_HPP
