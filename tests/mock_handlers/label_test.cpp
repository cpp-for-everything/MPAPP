// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_label`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/label_handler.hpp>
#include <mpapp/internal/basic_label.hpp>

namespace {

using label_mock = mpapp::internal::label_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("label mock handler records initial text", "[mock][label]") {
    mpapp::internal::basic_label l;
    label_mock   h;

    l.text = "Initial";
    h.map_text(l);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text=Initial"});
}

TEST_CASE("label mock handler fires once per real text change",
          "[mock][label]") {
    mpapp::internal::basic_label l;
    label_mock   h;

    h.map_text(l);
    h.clear_calls();

    l.text = "Hello";
    l.text = "World";

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"text=Hello", "text=World"});
}

TEST_CASE("label mock handler ignores no-op writes", "[mock][label]") {
    mpapp::internal::basic_label l;
    label_mock   h;

    l.text = "stable";
    h.map_text(l);
    h.clear_calls();

    l.text = "stable";
    l.text = "stable";

    REQUIRE(h.calls().empty());
}

TEST_CASE("label mock handler records initial font styling",
          "[mock][label]") {
    mpapp::internal::basic_label l;
    label_mock   h;

    l.font_size   = 18.0;
    l.font_bold   = true;
    l.font_family = "Sans";
    h.map_font_size(l);
    h.map_font_bold(l);
    h.map_font_family(l);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "font_size=18", "font_bold=true", "font_family=Sans"});
}

TEST_CASE("label mock handler tracks font changes", "[mock][label]") {
    mpapp::internal::basic_label l;
    label_mock   h;

    h.map_font_size(l);
    h.map_font_bold(l);
    h.clear_calls();

    l.font_size = 24.0;
    l.font_bold = true;
    l.font_bold = true;        // no-op, must not re-fire

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"font_size=24", "font_bold=true"});
}
