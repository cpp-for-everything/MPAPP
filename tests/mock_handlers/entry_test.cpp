// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::entry`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/entry.hpp>
#include <mpapp/handlers/mock/entry_handler.hpp>

namespace {

using entry_mock = mpapp::entry_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("entry mock handler records all initial values on map",
          "[mock][entry]") {
    mpapp::entry e;
    entry_mock   h;

    e.text = "abc";
    e.placeholder = "Email";
    e.is_password = true;
    e.max_length = 32;

    h.map_text(e);
    h.map_placeholder(e);
    h.map_is_password(e);
    h.map_max_length(e);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
                             "text=abc",
                             "placeholder=Email",
                             "is_password=true",
                             "max_length=32",
                         });
}

TEST_CASE("entry mock handler fires once per real text change",
          "[mock][entry]") {
    mpapp::entry e;
    entry_mock   h;

    h.map_text(e);
    h.clear_calls();

    e.text = "hello";

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text=hello"});
}

TEST_CASE("entry mock handler ignores same-value text writes",
          "[mock][entry]") {
    mpapp::entry e;
    entry_mock   h;

    e.text = "fixed";
    h.map_text(e);
    h.clear_calls();

    e.text = "fixed";

    REQUIRE(h.calls().empty());
}

TEST_CASE("entry mock handler tracks password toggle", "[mock][entry]") {
    mpapp::entry e;
    entry_mock   h;

    h.map_is_password(e);
    h.clear_calls();

    e.is_password = true;
    e.is_password = true;  // no-op
    e.is_password = false;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
                             "is_password=true",
                             "is_password=false",
                         });
}

TEST_CASE("entry mock handler tracks cursor position", "[mock][entry]") {
    mpapp::entry e;
    entry_mock   h;

    h.map_cursor_position(e);
    h.clear_calls();

    e.cursor_position = 5;
    e.cursor_position = 5;  // no-op
    e.cursor_position = 10;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
                             "cursor_position=5",
                             "cursor_position=10",
                         });
}
