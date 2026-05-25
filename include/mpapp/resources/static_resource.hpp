// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0005-resource-dictionaries-and-styling.md
//
// `mpapp::find_in<T>(view&, key)` — hierarchical resource lookup.
//
// Walks `v.resources → v.parent()->resources → ... → root.resources`
// and returns the first typed match. Counterpart to MAUI's
// `{StaticResource Key}` markup extension; the M-09 XAML pipeline
// lowers a `{StaticResource ButtonBg}` attribute to a `find_in<T>` call
// at the binding site, where `T` is the deduced setter target type.
//
// Merged dictionaries are resolved inside each visited dictionary
// (via `resource_dictionary::try_get<T>` walking its
// `merged_dictionaries` vector); this header only handles the
// visual-tree walk between dictionaries.

#ifndef MPAPP_RESOURCES_STATIC_RESOURCE_HPP
#define MPAPP_RESOURCES_STATIC_RESOURCE_HPP

#include <optional>
#include <string>

#include "../view.hpp"
#include "resource_dictionary.hpp"

namespace mpapp {

// Walk `v.resources → v.parent()->resources → ...` and return the
// first match for `key` as a `T`. Returns `std::nullopt` when no
// dictionary in the chain has a `T`-typed entry for `key`.
//
// Type-mismatch is a soft miss: if a dictionary entry exists for
// `key` but holds a value of a different type, the walker continues
// up the chain rather than returning the wrong-typed value. This
// matches MAUI's `{StaticResource}` behaviour — a typed binding
// against a wrongly-typed resource resolves at the next ancestor
// with the right type, falling through to nullopt only when the
// whole chain misses.
template <class T>
[[nodiscard]] std::optional<T> find_in(const view& v, const std::string& key) {
    for (const view* cur = &v; cur != nullptr; cur = cur->parent()) {
        if (cur->resources) {
            if (auto value = cur->resources->try_get<T>(key)) {
                return value;
            }
        }
    }
    return std::nullopt;
}

} // namespace mpapp

#endif // MPAPP_RESOURCES_STATIC_RESOURCE_HPP
