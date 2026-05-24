// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_switch_`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/switch_handler.hpp>
#include <mpapp/switch_.hpp>

namespace {

using switch_mock = mpapp::switch_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("switch mock handler logs initial is_on", "[mock][switch]") {
    mpapp::internal::basic_switch_ s;
    switch_mock    h;

    h.map_is_on(s);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_on=false"});
}

TEST_CASE("switch mock handler fires once per toggle", "[mock][switch]") {
    mpapp::internal::basic_switch_ s;
    switch_mock    h;

    h.map_is_on(s);
    h.clear_calls();

    s.is_on = true;
    s.is_on = false;
    s.is_on = true;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
                             "is_on=true",
                             "is_on=false",
                             "is_on=true",
                         });
}

TEST_CASE("switch mock handler ignores same-value writes",
          "[mock][switch]") {
    mpapp::internal::basic_switch_ s;
    switch_mock    h;

    s.is_on = true;
    h.map_is_on(s);
    h.clear_calls();

    s.is_on = true;
    s.is_on = true;

    REQUIRE(h.calls().empty());
}
