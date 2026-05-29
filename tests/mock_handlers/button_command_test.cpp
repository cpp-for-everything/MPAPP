// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for basic_button's RFC-0014 command property.

#include <memory>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/relay_command.hpp>
#include <mpapp/internal/basic_button.hpp>

using namespace mpapp;

TEST_CASE("button.command executes on click", "[mock][button][command]") {
    internal::basic_button b;
    int runs = 0;
    b.command = std::make_shared<relay_command>([&runs]() { ++runs; });

    b.clicked.emit();
    b.clicked.emit();
    CHECK(runs == 2);
}

TEST_CASE("button.command respects can_execute", "[mock][button][command]") {
    internal::basic_button b;
    int  runs    = 0;
    bool allowed = false;
    b.command = std::make_shared<relay_command>(
        [&runs]() { ++runs; }, [&allowed]() { return allowed; });

    b.clicked.emit();              // gated off
    CHECK(runs == 0);

    allowed = true;
    b.clicked.emit();
    CHECK(runs == 1);
}

TEST_CASE("button with no command is a harmless click", "[mock][button][command]") {
    internal::basic_button b;
    REQUIRE_NOTHROW(b.clicked.emit());   // null command -> no-op
}

TEST_CASE("swapping the bound command redirects clicks",
          "[mock][button][command]") {
    internal::basic_button b;
    int a = 0;
    int c = 0;
    b.command = std::make_shared<relay_command>([&a]() { ++a; });
    b.clicked.emit();
    CHECK(a == 1);

    b.command = std::make_shared<relay_command>([&c]() { ++c; });
    b.clicked.emit();
    CHECK(a == 1);                 // old command no longer invoked
    CHECK(c == 1);
}
