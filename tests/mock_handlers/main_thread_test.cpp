// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::main_thread and mock_main_thread.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/main_thread.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// is_main_thread — default and toggled states
// ---------------------------------------------------------------------------

TEST_CASE("mock_main_thread: is_main_thread defaults to true",
          "[mock][essentials][main_thread]") {
    // Arrange
    mock_main_thread mt;

    // Act + Assert
    CHECK(mt.is_main_thread());
}

TEST_CASE("mock_main_thread: set_is_main_thread toggles the return value",
          "[mock][essentials][main_thread]") {
    // Arrange
    mock_main_thread mt;
    CHECK(mt.is_main_thread()); // pre-condition: default true

    // Act — set to false
    mt.set_is_main_thread(false);

    // Assert
    CHECK_FALSE(mt.is_main_thread());

    // Act — restore to true
    mt.set_is_main_thread(true);

    // Assert
    CHECK(mt.is_main_thread());
}

// ---------------------------------------------------------------------------
// begin_invoke_on_main_thread — invoke count
// ---------------------------------------------------------------------------

TEST_CASE("mock_main_thread: invoke_count starts at zero",
          "[mock][essentials][main_thread]") {
    // Arrange + Act
    mock_main_thread mt;

    // Assert
    CHECK(mt.invoke_count() == 0u);
}

TEST_CASE("mock_main_thread: begin_invoke_on_main_thread increments invoke_count",
          "[mock][essentials][main_thread]") {
    // Arrange
    mock_main_thread mt;

    // Act
    mt.begin_invoke_on_main_thread([] {});
    mt.begin_invoke_on_main_thread([] {});
    mt.begin_invoke_on_main_thread([] {});

    // Assert
    CHECK(mt.invoke_count() == 3u);
}

TEST_CASE("mock_main_thread: reset_invoke_count resets the counter to zero",
          "[mock][essentials][main_thread]") {
    // Arrange
    mock_main_thread mt;
    mt.begin_invoke_on_main_thread([] {});
    mt.begin_invoke_on_main_thread([] {});
    REQUIRE(mt.invoke_count() == 2u);

    // Act
    mt.reset_invoke_count();

    // Assert
    CHECK(mt.invoke_count() == 0u);
}

// ---------------------------------------------------------------------------
// begin_invoke_on_main_thread — callback execution
// ---------------------------------------------------------------------------

TEST_CASE("mock_main_thread: callback is executed inline",
          "[mock][essentials][main_thread]") {
    // Arrange
    mock_main_thread mt;
    int side_effect = 0;

    // Act
    mt.begin_invoke_on_main_thread([&] { side_effect = 42; });

    // Assert — synchronous / inline execution
    CHECK(side_effect == 42);
}

TEST_CASE("mock_main_thread: multiple callbacks each execute inline",
          "[mock][essentials][main_thread]") {
    // Arrange
    mock_main_thread mt;
    int counter = 0;

    // Act
    mt.begin_invoke_on_main_thread([&] { counter += 1; });
    mt.begin_invoke_on_main_thread([&] { counter += 10; });
    mt.begin_invoke_on_main_thread([&] { counter += 100; });

    // Assert
    CHECK(counter == 111);
    CHECK(mt.invoke_count() == 3u);
}

TEST_CASE("mock_main_thread: null/empty callback does not crash and still increments counter",
          "[mock][essentials][main_thread]") {
    // Arrange
    mock_main_thread mt;

    // Act — pass a default-constructed (empty) std::function
    std::function<void()> empty_action;
    mt.begin_invoke_on_main_thread(empty_action);

    // Assert — counter incremented, no crash
    CHECK(mt.invoke_count() == 1u);
}

// ---------------------------------------------------------------------------
// Interface polymorphism — use through the abstract base pointer
// ---------------------------------------------------------------------------

TEST_CASE("mock_main_thread: usable via main_thread abstract interface pointer",
          "[mock][essentials][main_thread]") {
    // Arrange
    mock_main_thread concrete;
    main_thread* iface = &concrete;

    // Act
    bool on_main = iface->is_main_thread();
    int visited = 0;
    iface->begin_invoke_on_main_thread([&] { visited = 7; });

    // Assert
    CHECK(on_main == true);
    CHECK(visited == 7);
    CHECK(concrete.invoke_count() == 1u);
}

TEST_CASE("mock_main_thread: is_main_thread false visible through base interface",
          "[mock][essentials][main_thread]") {
    // Arrange
    mock_main_thread concrete;
    concrete.set_is_main_thread(false);
    main_thread* iface = &concrete;

    // Act + Assert
    CHECK_FALSE(iface->is_main_thread());
}
