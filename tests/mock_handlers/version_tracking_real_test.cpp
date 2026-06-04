// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Tests for the real preferences-backed version-tracking
// backend (RFC-0013 Essentials). All tests use in_memory_preferences so
// there is no file I/O; the same store can be passed to successive
// real_version_tracking instances to simulate persistence across launches.

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/preferences.hpp>
#include <mpapp/essentials/real_version_tracking.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// First-ever launch
// ---------------------------------------------------------------------------

TEST_CASE("real_version_tracking first track - first-launch-ever true",
          "[mock][version_tracking]") {
    // Arrange
    in_memory_preferences prefs;

    // Act
    real_version_tracking vt{ prefs, "1.0.0", "100" };
    vt.track();

    // Assert
    CHECK(vt.is_first_launch_ever());
    CHECK(vt.is_first_launch_for_current_version());
    CHECK(vt.is_first_launch_for_current_build());
    CHECK(vt.current_version() == "1.0.0");
    CHECK(vt.current_build()   == "100");
    CHECK_FALSE(vt.previous_version().has_value());
    CHECK(vt.first_installed_version() == "1.0.0");

    const auto vh = vt.version_history();
    REQUIRE(vh.size() == 1);
    CHECK(vh[0] == "1.0.0");

    const auto bh = vt.build_history();
    REQUIRE(bh.size() == 1);
    CHECK(bh[0] == "100");
}

// ---------------------------------------------------------------------------
// Second launch - same version, same build
// ---------------------------------------------------------------------------

TEST_CASE("real_version_tracking second track same version - flags false",
          "[mock][version_tracking]") {
    // Arrange
    in_memory_preferences prefs;
    {
        real_version_tracking vt1{ prefs, "1.0.0", "100" };
        vt1.track();
    }

    // Act - second launch, identical version + build
    real_version_tracking vt2{ prefs, "1.0.0", "100" };
    vt2.track();

    // Assert - no first-launch flags for a repeat launch
    CHECK_FALSE(vt2.is_first_launch_ever());
    CHECK_FALSE(vt2.is_first_launch_for_current_version());
    CHECK_FALSE(vt2.is_first_launch_for_current_build());
    CHECK(vt2.current_version() == "1.0.0");
    CHECK(vt2.current_build()   == "100");
    // previous_version is the last entry before this launch - still "1.0.0"
    REQUIRE(vt2.previous_version().has_value());
    CHECK(*vt2.previous_version() == "1.0.0");
    CHECK(vt2.first_installed_version() == "1.0.0");

    // History should not grow - the version was already recorded
    const auto vh = vt2.version_history();
    REQUIRE(vh.size() == 1);
    CHECK(vh[0] == "1.0.0");

    const auto bh = vt2.build_history();
    REQUIRE(bh.size() == 1);
    CHECK(bh[0] == "100");
}

// ---------------------------------------------------------------------------
// Version bump - is_first_launch_for_current_version becomes true
// ---------------------------------------------------------------------------

TEST_CASE("real_version_tracking version bump - for_current_version true",
          "[mock][version_tracking]") {
    // Arrange - first launch at v1
    in_memory_preferences prefs;
    {
        real_version_tracking vt1{ prefs, "1.0.0", "100" };
        vt1.track();
    }

    // Act - app updated, new launch at v2 (same build slot, different string)
    real_version_tracking vt2{ prefs, "2.0.0", "200" };
    vt2.track();

    // Assert
    CHECK_FALSE(vt2.is_first_launch_ever());
    CHECK(vt2.is_first_launch_for_current_version());
    CHECK(vt2.is_first_launch_for_current_build());
    CHECK(vt2.current_version() == "2.0.0");
    CHECK(vt2.current_build()   == "200");
    REQUIRE(vt2.previous_version().has_value());
    CHECK(*vt2.previous_version() == "1.0.0");
    CHECK(vt2.first_installed_version() == "1.0.0");

    const auto vh = vt2.version_history();
    REQUIRE(vh.size() == 2);
    CHECK(vh[0] == "1.0.0");
    CHECK(vh[1] == "2.0.0");

    const auto bh = vt2.build_history();
    REQUIRE(bh.size() == 2);
    CHECK(bh[0] == "100");
    CHECK(bh[1] == "200");
}

// ---------------------------------------------------------------------------
// Build bump only - version unchanged, build changes
// ---------------------------------------------------------------------------

TEST_CASE("real_version_tracking build bump - for_current_build true, version false",
          "[mock][version_tracking]") {
    // Arrange
    in_memory_preferences prefs;
    {
        real_version_tracking vt1{ prefs, "1.0.0", "100" };
        vt1.track();
    }

    // Act - same version, new build
    real_version_tracking vt2{ prefs, "1.0.0", "101" };
    vt2.track();

    // Assert
    CHECK_FALSE(vt2.is_first_launch_ever());
    CHECK_FALSE(vt2.is_first_launch_for_current_version());
    CHECK(vt2.is_first_launch_for_current_build());

    const auto vh = vt2.version_history();
    REQUIRE(vh.size() == 1); // version did not change
    CHECK(vh[0] == "1.0.0");

    const auto bh = vt2.build_history();
    REQUIRE(bh.size() == 2);
    CHECK(bh[0] == "100");
    CHECK(bh[1] == "101");
}

// ---------------------------------------------------------------------------
// Three launches - full history growth
// ---------------------------------------------------------------------------

TEST_CASE("real_version_tracking three launches - history grows correctly",
          "[mock][version_tracking]") {
    // Arrange
    in_memory_preferences prefs;

    // Launch 1
    {
        real_version_tracking vt{ prefs, "1.0", "1" };
        vt.track();
    }
    // Launch 2 - same
    {
        real_version_tracking vt{ prefs, "1.0", "1" };
        vt.track();
    }
    // Launch 3 - new version
    real_version_tracking vt3{ prefs, "1.1", "2" };
    vt3.track();

    CHECK(vt3.first_installed_version() == "1.0");
    REQUIRE(vt3.previous_version().has_value());
    CHECK(*vt3.previous_version() == "1.0");

    const auto vh = vt3.version_history();
    REQUIRE(vh.size() == 2);
    CHECK(vh[0] == "1.0");
    CHECK(vh[1] == "1.1");
}

// ---------------------------------------------------------------------------
// tracked signal fires on each track()
// ---------------------------------------------------------------------------

TEST_CASE("real_version_tracking tracked signal fires on track",
          "[mock][version_tracking]") {
    // Arrange
    in_memory_preferences prefs;
    real_version_tracking vt{ prefs, "1.0.0", "100" };

    int fire_count = 0;
    signal_slot<> slot;
    auto cb = [&]() { ++fire_count; };
    vt.tracked.subscribe(slot, cb);

    // Act
    vt.track();
    vt.track();

    // Assert
    CHECK(fire_count == 2);
}

// ---------------------------------------------------------------------------
// Accessors before track() - safe defaults
// ---------------------------------------------------------------------------

TEST_CASE("real_version_tracking accessors before track - safe defaults",
          "[mock][version_tracking]") {
    // Arrange
    in_memory_preferences prefs;
    real_version_tracking vt{ prefs, "1.0.0", "100" };

    // Assert - pre-track state: all false, empty histories
    CHECK_FALSE(vt.is_first_launch_ever());
    CHECK_FALSE(vt.is_first_launch_for_current_version());
    CHECK_FALSE(vt.is_first_launch_for_current_build());
    CHECK(vt.current_version() == "1.0.0");
    CHECK(vt.current_build()   == "100");
    CHECK_FALSE(vt.previous_version().has_value());
    CHECK(vt.first_installed_version().empty());
    CHECK(vt.version_history().empty());
    CHECK(vt.build_history().empty());
}

// ---------------------------------------------------------------------------
// Persistence: prefs store carries history to next instance
// ---------------------------------------------------------------------------

TEST_CASE("real_version_tracking persists history via preferences",
          "[mock][version_tracking]") {
    // Arrange - shared prefs simulates the same backing store across launches
    in_memory_preferences prefs;

    {
        real_version_tracking vt{ prefs, "0.9", "90" };
        vt.track();
    }

    // Act - new instance reads from the same prefs
    real_version_tracking vt2{ prefs, "1.0", "100" };
    vt2.track();

    // Assert - history accumulated across instances
    CHECK(vt2.first_installed_version() == "0.9");
    CHECK(*vt2.previous_version()       == "0.9");

    const auto vh = vt2.version_history();
    REQUIRE(vh.size() == 2);
    CHECK(vh[0] == "0.9");
    CHECK(vh[1] == "1.0");
}

// ---------------------------------------------------------------------------
// Storage keys and separator constants are accessible
// ---------------------------------------------------------------------------

TEST_CASE("real_version_tracking storage keys are non-empty strings",
          "[mock][version_tracking]") {
    CHECK(std::string{ real_version_tracking::kVersionHistoryKey }.size() > 0);
    CHECK(std::string{ real_version_tracking::kBuildHistoryKey   }.size() > 0);
    // Separator should not be a printable ASCII character that appears in
    // typical semver/build strings.
    CHECK(real_version_tracking::kSeparator == '\x1F');
}

// ---------------------------------------------------------------------------
// Repeated same-version launches beyond two - history stays deduplicated
// ---------------------------------------------------------------------------

TEST_CASE("real_version_tracking repeated launches do not duplicate history",
          "[mock][version_tracking]") {
    // Arrange
    in_memory_preferences prefs;

    for (int i = 0; i < 5; ++i) {
        real_version_tracking vt{ prefs, "1.0.0", "100" };
        vt.track();
    }

    real_version_tracking vt_final{ prefs, "1.0.0", "100" };
    vt_final.track();

    // Assert - still only one entry in each history
    CHECK(vt_final.version_history().size() == 1);
    CHECK(vt_final.build_history().size()   == 1);
}
