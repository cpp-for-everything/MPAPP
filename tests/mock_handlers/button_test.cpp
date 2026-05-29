// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for the `button` surface.
//
// Validates that the mock `internal::button_handler<platform::mock>`
// records the initial value at mapper-attach time and one entry per real
// change. Setting the Observable to the same value must NOT log a new
// entry — that's `Observable::set`'s compare-on-write contract; this
// test pins it in via the handler boundary.
//
// Tests run against `mpapp::internal::basic_button` (the platform-
// agnostic surface), not the user-facing `mpapp::button` wrapper. The
// wrapper embeds the per-platform handler by value and would force this
// translation unit to link the GTK4/WinUI/etc. handler library; the
// surface holds the handler by POINTER, so the mock test stays
// link-isolated from any real platform.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_button.hpp>
#include <mpapp/handlers/mock/button_handler.hpp>

namespace {

using button_mock = mpapp::internal::button_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("button mock handler logs initial text on map", "[mock][button]") {
    mpapp::internal::basic_button b;
    button_mock                   h;

    h.map_text(b);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text="});
}

TEST_CASE("button mock handler fires once per real text change",
          "[mock][button]") {
    mpapp::internal::basic_button b;
    button_mock                   h;

    h.map_text(b);
    h.clear_calls();

    b.text = "hello";

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text=hello"});
}

TEST_CASE("button mock handler ignores same-value text writes",
          "[mock][button]") {
    mpapp::internal::basic_button b;
    button_mock                   h;

    b.text = "stable";
    h.map_text(b);
    h.clear_calls();

    b.text = "stable";

    REQUIRE(h.calls().empty());
}

TEST_CASE("button mock handler records click forwarding", "[mock][button]") {
    mpapp::internal::basic_button b;
    button_mock                   h;

    h.map_clicked(b);
    h.simulate_click(b);
    h.simulate_click(b);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"clicked", "clicked"});
}

TEST_CASE("button mock handler records accessible name", "[mock][button][a11y]") {
    mpapp::internal::basic_button b;
    button_mock                   h;

    b.semantic_description = "Open menu";
    h.map_semantics(b);
    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"semantic_description=Open menu"});

    h.clear_calls();
    b.semantic_description = "Close menu";
    b.semantic_description = "Close menu";   // idempotent
    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"semantic_description=Close menu"});
}
