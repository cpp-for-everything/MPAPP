// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::real_version_tracking` — a REAL, cross-platform backend for the
// `mpapp::version_tracking` interface, written once and compiled into every
// target (no ifdefs): launch history is persisted via an injected
// `mpapp::preferences` store (e.g. file_preferences for durable storage or
// in_memory_preferences for tests). Separator-joined lists are stored under
// two well-known keys. Counterpart to MAUI Essentials' VersionTracking, but
// the preferences-backed approach keeps it a pure cross-platform
// implementation with no platform macros.

#ifndef MPAPP_ESSENTIALS_REAL_VERSION_TRACKING_HPP
#define MPAPP_ESSENTIALS_REAL_VERSION_TRACKING_HPP

#include <optional>
#include <string>
#include <vector>

#include "preferences.hpp"
#include "version_tracking.hpp"

namespace mpapp {

// Real, preferences-backed version tracking.
//
// Construction: supply a preferences& (lifetime must exceed this object),
// plus the version and build strings for the current launch.
//
// track() reads the persisted history from the preferences store, computes
// all first-launch flags, then writes the updated history back. The object
// holds the post-track() state and is queryable until the next track() call.
//
// Storage keys:
//   "mpapp.version_tracking.version_history"  — separator-joined version list
//   "mpapp.version_tracking.build_history"    — separator-joined build list
//
// Separator: '\x1F' (ASCII Unit Separator, unlikely to appear in semver/build
// strings). The separator is not part of the public API.
//
// Rule of Zero: all members are value types or references; no explicit
// destructor, copy, or move needed (copy/move are deleted because we hold a
// reference, which prevents accidental misuse).
class real_version_tracking final : public version_tracking {
public:
    static constexpr char kSeparator = '\x1F';

    static constexpr const char* kVersionHistoryKey =
        "mpapp.version_tracking.version_history";
    static constexpr const char* kBuildHistoryKey =
        "mpapp.version_tracking.build_history";

    // Construct with an external preferences store (injected dependency).
    // The caller retains ownership of `prefs`; it must outlive this object.
    explicit real_version_tracking(preferences& prefs,
                                   std::string  version,
                                   std::string  build)
        : prefs_{ prefs }
        , current_version_{ std::move(version) }
        , current_build_{ std::move(build) }
    {}

    real_version_tracking(const real_version_tracking&)            = delete;
    real_version_tracking& operator=(const real_version_tracking&) = delete;
    real_version_tracking(real_version_tracking&&)                 = delete;
    real_version_tracking& operator=(real_version_tracking&&)      = delete;

    // --- State queries (valid after the first track() call) ----------------

    [[nodiscard]] bool is_first_launch_ever() const override {
        return first_launch_ever_;
    }

    [[nodiscard]] bool is_first_launch_for_current_version() const override {
        return first_launch_for_version_;
    }

    [[nodiscard]] bool is_first_launch_for_current_build() const override {
        return first_launch_for_build_;
    }

    [[nodiscard]] std::string current_version() const override {
        return current_version_;
    }

    [[nodiscard]] std::string current_build() const override {
        return current_build_;
    }

    [[nodiscard]] std::optional<std::string> previous_version() const override {
        return previous_version_;
    }

    [[nodiscard]] std::string first_installed_version() const override {
        return first_installed_version_;
    }

    [[nodiscard]] std::vector<std::string> version_history() const override {
        return version_history_;
    }

    [[nodiscard]] std::vector<std::string> build_history() const override {
        return build_history_;
    }

    // --- Lifecycle ---------------------------------------------------------

    // Record a new application launch. Reads persisted history from the
    // preferences store, updates all first-launch flags, then writes the
    // updated history back. Fires the `tracked` signal on completion.
    void track() override {
        // --- Load persisted history -----------------------------------------
        version_history_ = load_history_(kVersionHistoryKey);
        build_history_   = load_history_(kBuildHistoryKey);

        const bool ever_launched  = !version_history_.empty();
        const bool version_is_new = !contains_(version_history_, current_version_);
        const bool build_is_new   = !contains_(build_history_,   current_build_);

        // --- Compute flags ---------------------------------------------------
        if (!ever_launched) {
            first_launch_ever_        = true;
            first_launch_for_version_ = true;
            first_launch_for_build_   = true;
            first_installed_version_  = current_version_;
            previous_version_         = std::nullopt;
        } else {
            first_launch_ever_        = false;
            previous_version_         = version_history_.back();
            first_launch_for_version_ = version_is_new;
            first_launch_for_build_   = build_is_new;
            // first_installed_version_ is always the first entry.
            first_installed_version_  = version_history_.front();
        }

        // --- Update histories ------------------------------------------------
        if (version_is_new) {
            version_history_.push_back(current_version_);
        }
        if (build_is_new) {
            build_history_.push_back(current_build_);
        }

        // --- Persist ---------------------------------------------------------
        prefs_.set_string(kVersionHistoryKey, join_(version_history_));
        prefs_.set_string(kBuildHistoryKey,   join_(build_history_));

        tracked.emit();
    }

private:
    // --- Helpers ------------------------------------------------------------

    [[nodiscard]] static bool contains_(const std::vector<std::string>& v,
                                        const std::string& s) {
        for (const auto& item : v) {
            if (item == s) { return true; }
        }
        return false;
    }

    // Split a separator-joined string into tokens. An empty stored string
    // (or absent key) yields an empty vector.
    [[nodiscard]] std::vector<std::string>
    load_history_(const char* key) const {
        auto raw = prefs_.get_string(key);
        if (!raw || raw->empty()) {
            return {};
        }
        return split_(*raw);
    }

    [[nodiscard]] static std::vector<std::string>
    split_(const std::string& s) {
        std::vector<std::string> out;
        std::string token;
        for (char c : s) {
            if (c == kSeparator) {
                out.push_back(token);
                token.clear();
            } else {
                token += c;
            }
        }
        if (!token.empty()) {
            out.push_back(token);
        }
        return out;
    }

    [[nodiscard]] static std::string
    join_(const std::vector<std::string>& v) {
        std::string out;
        for (std::size_t i = 0; i < v.size(); ++i) {
            if (i != 0) { out += kSeparator; }
            out += v[i];
        }
        return out;
    }

    // --- Members ------------------------------------------------------------

    preferences& prefs_;
    std::string  current_version_;
    std::string  current_build_;

    bool                       first_launch_ever_        = false;
    bool                       first_launch_for_version_ = false;
    bool                       first_launch_for_build_   = false;
    std::string                first_installed_version_{};
    std::optional<std::string> previous_version_         = std::nullopt;
    std::vector<std::string>   version_history_{};
    std::vector<std::string>   build_history_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_REAL_VERSION_TRACKING_HPP
