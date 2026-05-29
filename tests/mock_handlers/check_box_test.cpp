// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_check_box`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_check_box.hpp>
#include <mpapp/handlers/mock/check_box_handler.hpp>

namespace {

using check_box_mock = mpapp::internal::check_box_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("check_box mock handler logs initial is_checked",
          "[mock][check_box]") {
    mpapp::internal::basic_check_box c;
    check_box_mock   h;

    h.map_is_checked(c);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_checked=false"});
}

TEST_CASE("check_box mock handler fires once per real toggle",
          "[mock][check_box]") {
    mpapp::internal::basic_check_box c;
    check_box_mock   h;

    h.map_is_checked(c);
    h.clear_calls();

    c.is_checked = true;
    c.is_checked = false;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
                             "is_checked=true",
                             "is_checked=false",
                         });
}

TEST_CASE("check_box mock handler ignores same-value writes",
          "[mock][check_box]") {
    mpapp::internal::basic_check_box c;
    check_box_mock   h;

    c.is_checked = true;
    h.map_is_checked(c);
    h.clear_calls();

    c.is_checked = true;
    c.is_checked = true;

    REQUIRE(h.calls().empty());
}
