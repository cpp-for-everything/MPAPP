// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mock_version_tracking (RFC-0013).

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/version_tracking.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// No return-by-value helper — signal<> is non-copyable/non-movable.
// Each test constructs mock_version_tracking in-place.
// ---------------------------------------------------------------------------

// ===========================================================================
// Construction / pre-track state
// ===========================================================================
TEST_CASE("version_tracking: pre-track state is empty/false",
          "[mock][essentials][version_tracking]") {
    // Arrange
    mock_version_tracking vt{ "2.0", "200" };

    // Assert — nothing tracked yet, all flags false, histories empty
    CHECK_FALSE(vt.is_first_launch_ever());
    CHECK_FALSE(vt.is_first_launch_for_current_version());
    CHECK_FALSE(vt.is_first_launch_for_current_build());
    CHECK(vt.current_version() == "2.0");
    CHECK(vt.current_build()   == "200");
    CHECK_FALSE(vt.previous_version().has_value());
    CHECK(vt.first_installed_version().empty());
    CHECK(vt.version_history().empty());
    CHECK(vt.build_history().empty());
}

// ===========================================================================
// First track() — first-ever launch
// ===========================================================================
TEST_CASE("version_tracking: first track() sets all first-launch flags",
          "[mock][essentials][version_tracking]") {
    // Arrange + Act
    mock_version_tracking vt{ "1.0", "100" };
    vt.track();

    // Assert
    CHECK(vt.is_first_launch_ever());
    CHECK(vt.is_first_launch_for_current_version());
    CHECK(vt.is_first_launch_for_current_build());
    CHECK(vt.current_version() == "1.0");
    CHECK(vt.current_build()   == "100");
    CHECK_FALSE(vt.previous_version().has_value());
    CHECK(vt.first_installed_version() == "1.0");
    CHECK(vt.version_history() == std::vector<std::string>{ "1.0" });
    CHECK(vt.build_history()   == std::vector<std::string>{ "100" });
}

// ===========================================================================
// Second track() with same version+build — repeat launch
// ===========================================================================
TEST_CASE("version_tracking: second track() same version/build clears flags",
          "[mock][essentials][version_tracking]") {
    // Arrange
    mock_version_tracking vt{ "1.0", "100" };
    vt.track();

    // Act — simulate another launch of the same version/build
    vt.track();

    // Assert
    CHECK_FALSE(vt.is_first_launch_ever());
    CHECK_FALSE(vt.is_first_launch_for_current_version());
    CHECK_FALSE(vt.is_first_launch_for_current_build());
    CHECK(vt.previous_version().has_value());
    CHECK(*vt.previous_version() == "1.0");
    CHECK(vt.first_installed_version() == "1.0");
    // Histories must NOT grow (same version/build)
    CHECK(vt.version_history() == std::vector<std::string>{ "1.0" });
    CHECK(vt.build_history()   == std::vector<std::string>{ "100" });
}

// ===========================================================================
// track() after a version upgrade
// ===========================================================================
TEST_CASE("version_tracking: new version sets per-version flag only",
          "[mock][essentials][version_tracking]") {
    // Arrange — first launch with 1.0/100
    mock_version_tracking vt{ "1.0", "100" };
    vt.track(); // first launch
    vt.track(); // second launch same version
    // Act — simulate upgrade to 2.0, same build string
    vt.set_current_version("2.0");
    vt.track();

    // Assert
    CHECK_FALSE(vt.is_first_launch_ever());           // not first ever
    CHECK(vt.is_first_launch_for_current_version());  // new version
    CHECK_FALSE(vt.is_first_launch_for_current_build()); // build unchanged
    CHECK(vt.current_version() == "2.0");
    CHECK(*vt.previous_version() == "1.0");
    CHECK(vt.first_installed_version() == "1.0");
    CHECK(vt.version_history() == (std::vector<std::string>{ "1.0", "2.0" }));
    CHECK(vt.build_history()   == std::vector<std::string>{ "100" });
}

// ===========================================================================
// track() after a build-number change only
// ===========================================================================
TEST_CASE("version_tracking: new build sets per-build flag only",
          "[mock][essentials][version_tracking]") {
    // Arrange
    mock_version_tracking vt{ "1.0", "100" };
    vt.track();
    // Act — same version, different build
    vt.set_current_build("101");
    vt.track();

    // Assert
    CHECK_FALSE(vt.is_first_launch_ever());
    CHECK_FALSE(vt.is_first_launch_for_current_version()); // version unchanged
    CHECK(vt.is_first_launch_for_current_build());         // new build
    CHECK(vt.current_build() == "101");
    CHECK(*vt.previous_version() == "1.0");
    CHECK(vt.version_history() == std::vector<std::string>{ "1.0" });
    CHECK(vt.build_history()   == (std::vector<std::string>{ "100", "101" }));
}

// ===========================================================================
// track() after both version AND build change
// ===========================================================================
TEST_CASE("version_tracking: new version + new build sets both flags",
          "[mock][essentials][version_tracking]") {
    // Arrange
    mock_version_tracking vt{ "1.0", "100" };
    vt.track();
    // Act
    vt.set_current_version("2.0");
    vt.set_current_build("200");
    vt.track();

    // Assert
    CHECK_FALSE(vt.is_first_launch_ever());
    CHECK(vt.is_first_launch_for_current_version());
    CHECK(vt.is_first_launch_for_current_build());
    CHECK(vt.version_history() == (std::vector<std::string>{ "1.0", "2.0" }));
    CHECK(vt.build_history()   == (std::vector<std::string>{ "100", "200" }));
}

// ===========================================================================
// first_installed_version never changes after subsequent tracks
// ===========================================================================
TEST_CASE("version_tracking: first_installed_version stays fixed across updates",
          "[mock][essentials][version_tracking]") {
    // Arrange + Act
    mock_version_tracking vt{ "1.0", "100" };
    vt.track();
    vt.set_current_version("2.0");
    vt.track();
    vt.set_current_version("3.0");
    vt.track();

    // Assert
    CHECK(vt.first_installed_version() == "1.0");
    CHECK(vt.version_history() == (std::vector<std::string>{ "1.0", "2.0", "3.0" }));
}

// ===========================================================================
// previous_version progression across multiple launches
// ===========================================================================
TEST_CASE("version_tracking: previous_version follows version history",
          "[mock][essentials][version_tracking]") {
    // Arrange
    mock_version_tracking vt{ "1.0", "100" };

    // Act / Assert — step by step
    vt.track();
    CHECK_FALSE(vt.previous_version().has_value()); // first ever

    vt.set_current_version("2.0");
    vt.track();
    REQUIRE(vt.previous_version().has_value());
    CHECK(*vt.previous_version() == "1.0");

    vt.set_current_version("3.0");
    vt.track();
    REQUIRE(vt.previous_version().has_value());
    CHECK(*vt.previous_version() == "2.0");
}

// ===========================================================================
// Histories do not record duplicates
// ===========================================================================
TEST_CASE("version_tracking: repeated same version not added again to history",
          "[mock][essentials][version_tracking]") {
    // Arrange
    mock_version_tracking vt{ "1.0", "100" };
    vt.track();

    // Act — 3 more launches with the same version/build
    vt.track();
    vt.track();
    vt.track();

    // Assert — still exactly one entry each
    CHECK(vt.version_history().size() == 1);
    CHECK(vt.build_history().size()   == 1);
}

// ===========================================================================
// set_current_version / set_current_build accessors visible before track
// ===========================================================================
TEST_CASE("version_tracking: set helpers update current_version/build",
          "[mock][essentials][version_tracking]") {
    // Arrange
    mock_version_tracking vt{ "1.0", "100" };

    // Act
    vt.set_current_version("2.0");
    vt.set_current_build("200");

    // Assert
    CHECK(vt.current_version() == "2.0");
    CHECK(vt.current_build()   == "200");
}

// ===========================================================================
// tracked signal fires on every track() call
// ===========================================================================
TEST_CASE("version_tracking: tracked signal fires on each track()",
          "[mock][essentials][version_tracking][signal]") {
    // Arrange
    mock_version_tracking vt{ "1.0", "100" };
    int fire_count = 0;
    signal_slot<> slot;
    auto cb = [&]() { ++fire_count; };
    vt.tracked.subscribe(slot, cb);

    // Act
    vt.track();
    CHECK(fire_count == 1);

    vt.track();
    CHECK(fire_count == 2);

    vt.set_current_version("2.0");
    vt.track();
    CHECK(fire_count == 3);
}

// ===========================================================================
// tracked signal does NOT fire before track() is called
// ===========================================================================
TEST_CASE("version_tracking: tracked signal silent before track()",
          "[mock][essentials][version_tracking][signal]") {
    // Arrange
    mock_version_tracking vt{ "1.0", "100" };
    int fire_count = 0;
    signal_slot<> slot;
    auto cb = [&]() { ++fire_count; };
    vt.tracked.subscribe(slot, cb);

    // Assert — no track(), no signal
    CHECK(fire_count == 0);
}

// ===========================================================================
// Slot disconnect before track() — signal must not call the handler
// ===========================================================================
TEST_CASE("version_tracking: disconnected slot not called",
          "[mock][essentials][version_tracking][signal]") {
    // Arrange
    mock_version_tracking vt{ "1.0", "100" };
    int fire_count = 0;
    {
        signal_slot<> slot;
        auto cb = [&]() { ++fire_count; };
        vt.tracked.subscribe(slot, cb);
        // slot destroyed here -> auto-disconnect
    }

    // Act
    vt.track();

    // Assert
    CHECK(fire_count == 0);
}

// ===========================================================================
// Multiple subscribers each get the signal
// ===========================================================================
TEST_CASE("version_tracking: multiple subscribers all receive tracked signal",
          "[mock][essentials][version_tracking][signal]") {
    // Arrange
    mock_version_tracking vt{ "1.0", "100" };
    int count_a = 0, count_b = 0;
    signal_slot<> slot_a, slot_b;
    auto cb_a = [&]() { ++count_a; };
    auto cb_b = [&]() { ++count_b; };
    vt.tracked.subscribe(slot_a, cb_a);
    vt.tracked.subscribe(slot_b, cb_b);

    // Act
    vt.track();

    // Assert
    CHECK(count_a == 1);
    CHECK(count_b == 1);
}

// ===========================================================================
// Interface pointer usage (polymorphism smoke test)
// ===========================================================================
TEST_CASE("version_tracking: usable through abstract interface pointer",
          "[mock][essentials][version_tracking]") {
    // Arrange
    mock_version_tracking concrete{ "1.0", "100" };
    version_tracking* iface = &concrete;

    // Act
    iface->track();

    // Assert via interface
    CHECK(iface->is_first_launch_ever());
    CHECK(iface->current_version() == "1.0");
    CHECK(iface->current_build()   == "100");
    CHECK(iface->version_history().size() == 1);
    CHECK(iface->build_history().size()   == 1);
    CHECK_FALSE(iface->previous_version().has_value());
    CHECK(iface->first_installed_version() == "1.0");
}

// ===========================================================================
// Accessors consistent with MAUI semantics after long history
// ===========================================================================
TEST_CASE("version_tracking: extended version history",
          "[mock][essentials][version_tracking]") {
    // Arrange + Act
    mock_version_tracking vt{ "1.0", "1" };
    vt.track();                        // 1.0 / build 1

    vt.set_current_version("1.1");
    vt.set_current_build("2");
    vt.track();                        // 1.1 / build 2

    vt.set_current_version("1.2");
    vt.set_current_build("3");
    vt.track();                        // 1.2 / build 3

    vt.track();                        // 1.2 / build 3 again

    // Assert
    CHECK(vt.version_history() == (std::vector<std::string>{ "1.0", "1.1", "1.2" }));
    CHECK(vt.build_history()   == (std::vector<std::string>{ "1", "2", "3" }));
    CHECK(vt.first_installed_version() == "1.0");
    CHECK(*vt.previous_version() == "1.2");   // last recorded before this track
    CHECK_FALSE(vt.is_first_launch_ever());
    CHECK_FALSE(vt.is_first_launch_for_current_version());
    CHECK_FALSE(vt.is_first_launch_for_current_build());
}
