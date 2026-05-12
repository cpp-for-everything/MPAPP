// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::label`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/label_handler.hpp>
#include <mpapp/label.hpp>

namespace {

using label_mock = mpapp::label_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("label mock handler records initial text", "[mock][label]") {
    mpapp::label l;
    label_mock   h;

    l.text = "Initial";
    h.map_text(l);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text=Initial"});
}

TEST_CASE("label mock handler fires once per real text change",
          "[mock][label]") {
    mpapp::label l;
    label_mock   h;

    h.map_text(l);
    h.clear_calls();

    l.text = "Hello";
    l.text = "World";

    REQUIRE(h.calls() ==
            std::vector<std::string>{"text=Hello", "text=World"});
}

TEST_CASE("label mock handler ignores no-op writes", "[mock][label]") {
    mpapp::label l;
    label_mock   h;

    l.text = "stable";
    h.map_text(l);
    h.clear_calls();

    l.text = "stable";
    l.text = "stable";

    REQUIRE(h.calls().empty());
}
