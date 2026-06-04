// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0014-asset-catalog.md
//
// `mpapp::asset_catalog` — the single-project asset model for MAUI-style
// assets.  Models MAUI's <MauiImage>, <MauiFont>, <MauiIcon>, and
// <MauiSplashScreen> project-item types as a flat catalog of `asset_entry`
// records keyed by a logical name.
//
// Design constraints (ADR-0002): no macros in the public API; header-only;
// platform-neutral.

#ifndef MPAPP_ASSETS_ASSET_CATALOG_HPP
#define MPAPP_ASSETS_ASSET_CATALOG_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../signal.hpp"

namespace mpapp {

// ---- Enumeration -----------------------------------------------------------

// Mirrors the four MAUI project-item types that surface in the MauiProgram
// builder.  Stored as uint8_t so it is cheap to copy and compare.
enum class asset_kind : std::uint8_t {
    image    = 0,   // <MauiImage>
    font     = 1,   // <MauiFont>
    app_icon = 2,   // <MauiIcon>
    splash   = 3,   // <MauiSplashScreen>
};

[[nodiscard]] constexpr std::string_view to_string(asset_kind k) noexcept {
    switch (k) {
        case asset_kind::image:    return "image";
        case asset_kind::font:     return "font";
        case asset_kind::app_icon: return "app_icon";
        case asset_kind::splash:   return "splash";
        default:                   return "?";
    }
}

// ---- Data record -----------------------------------------------------------

// A single catalog entry.  `key` is the logical registration name;
// `path` is the file path (relative to the project's Resources/ folder);
// `alias` is an optional display name (blank when unused).
struct asset_entry {
    std::string key{};
    std::string path{};
    asset_kind  kind  = asset_kind::image;
    std::string alias{};
};

// ---- Catalog class ---------------------------------------------------------

class asset_catalog {
public:
    asset_catalog() = default;

    // --- Mutators -----------------------------------------------------------

    // Register an asset entry.  Re-registering under an existing key
    // overwrites the previous entry and fires `asset_registered`.
    void register_asset(asset_entry entry) {
        std::string key = entry.key;                // copy before move
        entries_[key] = std::move(entry);
        asset_registered.emit(entries_[key]);
    }

    // --- Queries ------------------------------------------------------------

    // Find a single entry by its logical key, or nullopt if not found.
    [[nodiscard]] std::optional<asset_entry>
    find(std::string_view key) const {
        auto it = entries_.find(std::string{key});
        if (it == entries_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    // Return all entries of a specific asset_kind.
    [[nodiscard]] std::vector<asset_entry>
    by_kind(asset_kind k) const {
        std::vector<asset_entry> result;
        result.reserve(entries_.size());
        for (const auto& [unused_key, e] : entries_) {
            if (e.kind == k) {
                result.push_back(e);
            }
        }
        return result;
    }

    // Convenience: return the `path` field for a key, or nullopt.
    [[nodiscard]] std::optional<std::string>
    resolve_path(std::string_view key) const {
        auto it = entries_.find(std::string{key});
        if (it == entries_.end()) {
            return std::nullopt;
        }
        return it->second.path;
    }

    // Total number of registered assets.
    [[nodiscard]] std::size_t count() const noexcept {
        return entries_.size();
    }

    // --- Events -------------------------------------------------------------

    // Fires every time an asset is successfully registered (or overwritten).
    // Subscribers receive a const-ref to the stored entry.
    mpapp::signal<const asset_entry&> asset_registered{};

private:
    std::unordered_map<std::string, asset_entry> entries_{};
};

} // namespace mpapp

#endif // MPAPP_ASSETS_ASSET_CATALOG_HPP
