// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0005-resource-dictionaries-and-styling.md
//
// `mpapp::resource_dictionary` — keyed `std::any` value store with
// merged-dictionary composition + change signals. Counterpart to
// MAUI's `ResourceDictionary` (`IDictionary<string, object>` plus the
// MergedDictionaries collection). The XAML compiler lowers
// `<ResourceDictionary>` literals to put() calls on an instance of
// this type; `{StaticResource Key}` lowers to a `find_in<T>(...)`
// walk (see `static_resource.hpp`).
//
// Resource dictionaries are NOT a wrapper-component (ADR-0024):
// they own no native widget. They are pure configuration that the
// binding / style layers consume. Same shape as RFC-0003 gestures
// and RFC-0004 image sources.

#ifndef MPAPP_RESOURCES_RESOURCE_DICTIONARY_HPP
#define MPAPP_RESOURCES_RESOURCE_DICTIONARY_HPP

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../signal.hpp"

namespace mpapp {

class resource_dictionary {
public:
    resource_dictionary() = default;
    ~resource_dictionary() = default;

    // Move/copy disabled: callers hold dictionaries by
    // `shared_ptr<resource_dictionary>` (matches how `merged_dictionaries`
    // composes them and how `view::resources` stores per-view dictionaries).
    // The signal members embed intrusive slot lists that cannot relocate
    // safely.
    resource_dictionary(const resource_dictionary&)            = delete;
    resource_dictionary& operator=(const resource_dictionary&) = delete;
    resource_dictionary(resource_dictionary&&)                 = delete;
    resource_dictionary& operator=(resource_dictionary&&)      = delete;

    // ---- Type-erased value store --------------------------------------
    //
    // `std::any` matches MAUI's `IDictionary<string, object>` semantics:
    // the dictionary is the universal hand-off point for any value type
    // — colors, doubles, styles, brushes, image sources. Callers extract
    // typed values via `try_get<T>` (one-shot) or via the
    // `static_resource` walker (hierarchical).
    //
    // put() is `insert_or_assign` — sets an existing key to the new
    // value and fires `changed` with the new payload. The emitted
    // `change::key` points into the dictionary's own storage, so the
    // view stays valid for the lifetime of the entry.
    void put(std::string key, std::any value) {
        auto [it, inserted] = entries_.insert_or_assign(
            std::move(key), std::move(value));
        const change c{ std::string_view{it->first}, &it->second };
        changed.emit(c);
    }

    // remove() emits the `change` BEFORE erasing so subscribers can
    // still read the key view safely. `new_value` is null for removals.
    void remove(const std::string& key) {
        auto it = entries_.find(key);
        if (it == entries_.end()) {
            return;
        }
        const change c{ std::string_view{it->first}, nullptr };
        changed.emit(c);
        entries_.erase(it);
    }

    [[nodiscard]] bool has(const std::string& key) const noexcept {
        return entries_.find(key) != entries_.end();
    }

    // Local-only lookup — does NOT traverse merged dictionaries. Used by
    // the templated `try_get<T>` and by callers that want to introspect
    // the raw `std::any` payload (e.g., the XAML compiler when checking
    // for an existing entry before warning on dup-keys).
    [[nodiscard]] const std::any* try_get_local(const std::string& key) const noexcept {
        auto it = entries_.find(key);
        return (it != entries_.end()) ? &it->second : nullptr;
    }

    // Typed lookup with merged-dictionary fallback. Matches MAUI's
    // resolution order: local store first, then `merged_dictionaries`
    // in iteration order. The first match wins.
    template <class T>
    [[nodiscard]] std::optional<T> try_get(const std::string& key) const {
        if (const auto* v = try_get_local(key)) {
            if (const T* casted = std::any_cast<T>(v)) {
                return *casted;
            }
        }
        for (const auto& md : merged_dictionaries) {
            if (!md) {
                continue;
            }
            if (auto v = md->try_get<T>(key)) {
                return v;
            }
        }
        return std::nullopt;
    }

    // ---- Composition ---------------------------------------------------
    //
    // Lookups walk merged dictionaries AFTER the local store (matches
    // MAUI's resolution order). Apps that need to observe theme swaps
    // listen on `composition_changed` and re-resolve every active
    // lookup; mutating this vector via add_merged_dictionary() /
    // clear_merged_dictionaries() raises the signal automatically.
    //
    // Exposed as a public vector for XAML-compiler use (it appends
    // during `<MergedDictionaries>` parsing). Mutating it directly
    // does not raise `composition_changed`; prefer the helpers below.
    std::vector<std::shared_ptr<resource_dictionary>> merged_dictionaries{};

    void add_merged_dictionary(std::shared_ptr<resource_dictionary> d) {
        merged_dictionaries.push_back(std::move(d));
        composition_changed.emit();
    }

    void clear_merged_dictionaries() {
        merged_dictionaries.clear();
        composition_changed.emit();
    }

    // ---- Change notification ------------------------------------------
    //
    // `changed` fires per-key on put/remove. Subscribers route the
    // value to their property (the M-09 binding layer wires
    // `{DynamicResource Key}` to this signal).
    struct change {
        std::string_view key;        // points into entries_ storage
        const std::any*  new_value;  // nullptr for remove
    };

    mpapp::signal<const change&> changed{};

    // Composition-level signal — fires when `merged_dictionaries` is
    // mutated via the helpers (theme swap, dictionary unload, …).
    mpapp::signal<>              composition_changed{};

private:
    std::unordered_map<std::string, std::any> entries_{};
};

} // namespace mpapp

#endif // MPAPP_RESOURCES_RESOURCE_DICTIONARY_HPP
