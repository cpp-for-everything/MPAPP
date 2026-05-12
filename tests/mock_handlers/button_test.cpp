// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::button`.
//
// Validates that the mock `button_handler<platform::mock>` records the
// initial value at mapper-attach time and one entry per real change.
// Setting the Observable to the same value must NOT log a new entry —
// that's `Observable::set`'s compare-on-write contract; this test pins it
// in via the handler boundary.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/button.hpp>
#include <mpapp/handlers/mock/button_handler.hpp>

namespace {

using button_mock = mpapp::button_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("button mock handler logs initial text on map", "[mock][button]") {
    mpapp::button b;
    button_mock   h;

    h.map_text(b);

    REQUIRE(h.calls() == std::vector<std::string>{"text="});
}

TEST_CASE("button mock handler fires once per real text change",
          "[mock][button]") {
    mpapp::button b;
    button_mock   h;

    h.map_text(b);
    h.clear_calls();

    b.text = "hello";

    REQUIRE(h.calls() == std::vector<std::string>{"text=hello"});
}

TEST_CASE("button mock handler ignores same-value text writes",
          "[mock][button]") {
    mpapp::button b;
    button_mock   h;

    b.text = "stable";
    h.map_text(b);
    h.clear_calls();

    b.text = "stable";

    REQUIRE(h.calls().empty());
}

TEST_CASE("button mock handler records click forwarding", "[mock][button]") {
    mpapp::button b;
    button_mock   h;

    h.map_clicked(b);
    h.simulate_click(b);
    h.simulate_click(b);

    REQUIRE(h.calls() == std::vector<std::string>{"clicked", "clicked"});
}
