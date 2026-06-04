// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0009-public-api-template-wrappers-only.md
//
// observable_validator — CommunityToolkit.Mvvm ObservableValidator /
// INotifyDataErrorInfo equivalent. Validation rules are registered per
// property name; each rule reads the owning model (via capture) and returns
// either an error message or std::nullopt when the value is valid.
//
// validate() runs every rule; validate_property() runs only the rules keyed
// to one property. Both store the resulting error messages and emit
// `errors_changed` once per property whose error-set actually changed, so
// idempotent re-validation does not spuriously notify.

#ifndef MPAPP_MVVM_OBSERVABLE_VALIDATOR_HPP
#define MPAPP_MVVM_OBSERVABLE_VALIDATOR_HPP

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../signal.hpp"

namespace mpapp {

class observable_validator {
public:
    // A rule reads the model (via its captures) and returns an error message,
    // or std::nullopt when the inspected value is valid.
    using rule = std::function<std::optional<std::string>()>;

    observable_validator() = default;

    observable_validator(const observable_validator&)            = delete;
    observable_validator& operator=(const observable_validator&) = delete;
    observable_validator(observable_validator&&)                 = delete;
    observable_validator& operator=(observable_validator&&)      = delete;

    // Register a validation rule against a property name. Multiple rules may
    // be attached to the same property; all of them run during validation.
    void add_rule(std::string property, rule r) {
        rules_[std::move(property)].push_back(std::move(r));
    }

    // Run every registered rule. Returns true when no errors remain.
    bool validate() {
        bool ok = true;
        for (const auto& [property, property_rules] : rules_) {
            if (!run_rules(property, property_rules)) {
                ok = false;
            }
        }
        return ok;
    }

    // Run only the rules attached to `property`. Returns true when that
    // property has no errors. A property with no registered rules is valid.
    bool validate_property(std::string_view property) {
        const auto it = rules_.find(std::string(property));
        if (it == rules_.end()) {
            return true;
        }
        return run_rules(it->first, it->second);
    }

    [[nodiscard]] bool has_errors() const { return !errors_.empty(); }

    // Error messages for a single property (empty when valid/unknown).
    [[nodiscard]] std::vector<std::string> get_errors(std::string_view property) const {
        const auto it = errors_.find(std::string(property));
        if (it == errors_.end()) {
            return {};
        }
        return it->second;
    }

    // All error messages across every property, in property-name order.
    [[nodiscard]] std::vector<std::string> get_all_errors() const {
        std::vector<std::string> all;
        for (const auto& [property, messages] : errors_) {
            all.insert(all.end(), messages.begin(), messages.end());
        }
        return all;
    }

    // Fires once per property whose error-set changed during validation.
    mpapp::signal<std::string_view> errors_changed;

private:
    // Execute `property_rules`, collect non-null messages, and update the
    // stored error-set for `property`. Emits errors_changed only on a real
    // change. Returns true when the property ends with no errors.
    bool run_rules(const std::string& property, const std::vector<rule>& property_rules) {
        std::vector<std::string> messages;
        for (const auto& r : property_rules) {
            if (std::optional<std::string> message = r()) {
                messages.push_back(std::move(*message));
            }
        }

        const auto it = errors_.find(property);
        const bool had_errors = it != errors_.end();

        if (messages.empty()) {
            if (had_errors) {
                errors_.erase(it);
                errors_changed.emit(std::string_view{ property });
            }
            return true;
        }

        const bool changed = !had_errors || it->second != messages;
        errors_[property]  = std::move(messages);
        if (changed) {
            errors_changed.emit(std::string_view{ property });
        }
        return false;
    }

    std::map<std::string, std::vector<rule>>   rules_;
    std::map<std::string, std::vector<std::string>> errors_;
};

} // namespace mpapp

#endif // MPAPP_MVVM_OBSERVABLE_VALIDATOR_HPP
