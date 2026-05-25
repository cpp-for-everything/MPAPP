// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0005-resource-dictionaries-and-styling.md
//
// `mpapp::style` — a TargetType-tagged bundle of property setters.
// Counterpart to MAUI's `Style`. Apps reference styles by key from a
// resource_dictionary and apply them explicitly via
// `style.apply_to(view)`; the M-09 XAML pipeline lowers
// `Style="{StaticResource …}"` attributes to that call.
//
// A setter is `std::function<void(view&)>` — type-erased so a style
// can carry setters for any observable property. The setter is
// responsible for any downcast needed (e.g., to `basic_button&`); the
// style holds the target-type *name* purely for implicit-style
// matching (deferred to v2 per the RFC's open-questions section).
//
// Style inheritance via `based_on` runs the parent's setters first,
// then this style's, so derived setters override.

#ifndef MPAPP_RESOURCES_STYLE_HPP
#define MPAPP_RESOURCES_STYLE_HPP

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace mpapp {

// Forward-declared — style only references `view&` via the type-erased
// setter signature, so the full definition is not needed here. Users
// who write setters that touch view properties include `<mpapp/view.hpp>`
// (or the relevant control header) in their own translation unit.
class view;

class style {
public:
    explicit style(std::string target_type_name)
        : target_type{ std::move(target_type_name) } {}

    style(const style&)            = delete;
    style& operator=(const style&) = delete;
    style(style&&)                 = delete;
    style& operator=(style&&)      = delete;

    ~style() = default;

    // The TargetType name used for implicit-style matching (MAUI's
    // `TargetType="Button"`). Concrete controls publish their name via a
    // static `class_name()` method so XAML codegen can stamp the right
    // string at lowering time. For v1, app code drives the matching
    // explicitly — implicit-style scanning is deferred (see RFC §Open).
    std::string target_type;

    // Per-property setters, keyed by property name so derived styles can
    // override individual entries from `based_on`. The setter signature
    // is `void(view&)`; setters that touch typed properties downcast
    // internally (the style records what type its setters expect via
    // `target_type`, but does not enforce the cast — that's the app /
    // XAML codegen's responsibility, mirroring MAUI's behaviour).
    std::unordered_map<std::string, std::function<void(view&)>> setters{};

    // Style inheritance. When non-null, `based_on->apply_to(v)` runs
    // first; this style's setters then run on top, so they win on
    // duplicate keys (matches MAUI's Style.BasedOn semantics).
    std::shared_ptr<style> based_on{};

    // Apply every setter (after based_on) to `v`. Setters that throw
    // are caught so a single bad setter does not take down the entire
    // style chain (mirrors MAUI's swallow-and-log behaviour for setter
    // exceptions). The logged-exception channel is the framework's
    // concern; at the mock surface the catch is silent.
    void apply_to(view& v) const {
        if (based_on) {
            based_on->apply_to(v);
        }
        for (const auto& [name, setter] : setters) {
            if (!setter) {
                continue;
            }
            try {
                setter(v);
            } catch (const std::exception&) {
                // Intentionally swallowed — per RFC-0005, real handlers
                // route the exception to the framework's logger; the
                // mock surface keeps the catch silent so tests don't
                // race on log output.
            } catch (...) {
                // Same as above for non-std exceptions.
            }
        }
    }
};

} // namespace mpapp

#endif // MPAPP_RESOURCES_STYLE_HPP
