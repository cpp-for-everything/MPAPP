// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_editor`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_editor.hpp>
#include <mpapp/handlers/mock/editor_handler.hpp>

namespace {

using editor_mock = mpapp::internal::editor_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("editor mock handler logs initial text on map", "[mock][editor]") {
    mpapp::internal::basic_editor e;
    editor_mock   h;

    h.map_text(e);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text="});
}

TEST_CASE("editor mock handler fires once per real text change",
          "[mock][editor]") {
    mpapp::internal::basic_editor e;
    editor_mock   h;

    h.map_text(e);
    h.clear_calls();

    e.text = "line one\nline two";

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"text=line one\nline two"});
}

TEST_CASE("editor mock handler ignores same-value writes", "[mock][editor]") {
    mpapp::internal::basic_editor e;
    editor_mock   h;

    e.placeholder = "Notes...";
    h.map_placeholder(e);
    h.clear_calls();

    e.placeholder = "Notes...";

    REQUIRE(h.calls().empty());
}

TEST_CASE("editor mock handler tracks read-only toggle", "[mock][editor]") {
    mpapp::internal::basic_editor e;
    editor_mock   h;

    h.map_is_read_only(e);
    h.clear_calls();

    e.is_read_only = true;
    e.is_read_only = false;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
                             "is_read_only=true",
                             "is_read_only=false",
                         });
}

TEST_CASE("editor mock handler tracks max length", "[mock][editor]") {
    mpapp::internal::basic_editor e;
    editor_mock   h;

    e.max_length = 500;
    h.map_max_length(e);
    h.clear_calls();

    e.max_length = 1000;
    e.max_length = 1000;  // no-op

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"max_length=1000"});
}
