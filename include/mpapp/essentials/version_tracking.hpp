// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::version_tracking` — application version/build launch history.
// Counterpart to MAUI Essentials `VersionTracking`. Abstract interface +
// an in-memory mock implementation whose state is driveable from tests.
// Real per-platform backends persist to storage between launches and
// implement the same interface via DI (RFC-0011). No macros; header-only.

#ifndef MPAPP_ESSENTIALS_VERSION_TRACKING_HPP
#define MPAPP_ESSENTIALS_VERSION_TRACKING_HPP

#include <optional>
#include <string>
#include <vector>

#include "../signal.hpp"

namespace mpapp {

// Abstract interface — mirrors MAUI's IVersionTracking.
class version_tracking {
public:
    virtual ~version_tracking() = default;

    // --- State queries -------------------------------------------------

    // True only on the very first launch of the app ever.
    [[nodiscard]] virtual bool is_first_launch_ever() const = 0;

    // True on the first launch after an app version change.
    [[nodiscard]] virtual bool is_first_launch_for_current_version() const = 0;

    // True on the first launch after a build number change.
    [[nodiscard]] virtual bool is_first_launch_for_current_build() const = 0;

    // The version string supplied at construction / by the platform.
    [[nodiscard]] virtual std::string current_version() const = 0;

    // The build string supplied at construction / by the platform.
    [[nodiscard]] virtual std::string current_build() const = 0;

    // Version string from the previous launch (nullopt on first-ever launch).
    [[nodiscard]] virtual std::optional<std::string> previous_version() const = 0;

    // The very first version string ever recorded.
    [[nodiscard]] virtual std::string first_installed_version() const = 0;

    // Ordered list of all distinct versions seen, oldest first.
    [[nodiscard]] virtual std::vector<std::string> version_history() const = 0;

    // Ordered list of all distinct builds seen, oldest first.
    [[nodiscard]] virtual std::vector<std::string> build_history() const = 0;

    // --- Lifecycle -----------------------------------------------------

    // Record a new application launch against the current version/build.
    // Implementations update first-launch flags and persist to storage.
    virtual void track() = 0;

    // Fired after each successful track() call.
    mpapp::signal<> tracked{};
};

// In-memory mock — fully testable without any I/O.
//
// Construction: supply the version + build that the "current launch" will
// report. The histories start empty; call track() to simulate a launch.
//
// track() semantics:
//   - First call  : records this version+build as the first-ever install;
//                   all three first_launch flags are true.
//   - Second call : flags transition to false (simulates a subsequent
//                   launch of the same version/build).
//   - Changing version/build between calls re-raises the per-version and
//     per-build flags on the next track().
class mock_version_tracking final : public version_tracking {
public:
    explicit mock_version_tracking(std::string version, std::string build)
        : current_version_{ std::move(version) }
        , current_build_{ std::move(build) }
    {}

    // --- State queries -------------------------------------------------

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

    // --- Lifecycle -----------------------------------------------------

    void track() override {
        const bool ever_launched = !version_history_.empty();

        // Determine whether this version/build have been seen before.
        const bool version_is_new = !contains(version_history_, current_version_);
        const bool build_is_new   = !contains(build_history_,   current_build_);

        if (!ever_launched) {
            // Very first launch ever.
            first_launch_ever_        = true;
            first_launch_for_version_ = true;
            first_launch_for_build_   = true;
            first_installed_version_  = current_version_;
            previous_version_         = std::nullopt;
        } else {
            first_launch_ever_ = false;

            // Update previous_version to the last tracked version (before
            // recording this one).
            previous_version_ = version_history_.back();

            first_launch_for_version_ = version_is_new;
            first_launch_for_build_   = build_is_new;
        }

        if (version_is_new) {
            version_history_.push_back(current_version_);
        }
        if (build_is_new) {
            build_history_.push_back(current_build_);
        }

        tracked.emit();
    }

    // --- Test helpers --------------------------------------------------

    // Replace the current version/build (simulates an app update between
    // launches). Call track() afterwards to record the new launch.
    void set_current_version(std::string version) {
        current_version_ = std::move(version);
    }

    void set_current_build(std::string build) {
        current_build_ = std::move(build);
    }

private:
    [[nodiscard]] static bool contains(const std::vector<std::string>& v,
                                       const std::string& s) {
        for (const auto& item : v) {
            if (item == s) { return true; }
        }
        return false;
    }

    std::string current_version_;
    std::string current_build_;

    bool                     first_launch_ever_        = false;
    bool                     first_launch_for_version_ = false;
    bool                     first_launch_for_build_   = false;
    std::string              first_installed_version_{};
    std::optional<std::string> previous_version_       = std::nullopt;
    std::vector<std::string> version_history_{};
    std::vector<std::string> build_history_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_VERSION_TRACKING_HPP
